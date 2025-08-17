#include "TerrainRenderer.h"
#include "CETerrainVertex.hpp"
#include "vertex.h"

#include <map>

#include "C2MapFile.h"
#include "C2MapRscFile.h"
#include "CEWorldModel.h"
#include "CESimpleGeometry.h"
#include "CEGeometry.h"
#include "CETexture.h"
#include "shader_program.h"
#include "camera.h"
#include "transform.h"
#include "C2Sky.h"
#include "CEShadowManager.h"

#include "CEWaterEntity.h"

#include <cstdint>

#include <nlohmann/json.hpp>
#include <filesystem>

namespace fs = std::filesystem;
using json = nlohmann::json;

TerrainRenderer::TerrainRenderer(std::shared_ptr<C2MapFile> c_map_weak, std::shared_ptr<C2MapRscFile> c_rsc_weak)
: m_cmap_data_weak(c_map_weak), m_crsc_data_weak(c_rsc_weak)
{
  this->loadIntoHardwareMemory();
  // this->exportAsRaw();
  this->loadShader();
  this->preloadObjectMap();
}

TerrainRenderer::~TerrainRenderer()
{
  glDeleteTextures(1, &this->underwaterStateTexture);
  glDeleteTextures(1, &this->heightmapTexture);
  glDeleteBuffers(1, &this->m_vertex_array_buffer);
  glDeleteBuffers(1, &this->m_indices_array_buffer);
  glDeleteVertexArrays(1, &this->m_vertex_array_object);

  for (int w = 0; w < this->m_waters.size(); w++) {
    glDeleteBuffers(1, &this->m_waters[w].m_vab);
    glDeleteBuffers(1, &this->m_waters[w].m_iab);
    glDeleteVertexArrays(1, &this->m_waters[w].m_vao);
  }
  
  for (auto& fog_volume : this->m_fog_volumes) {
    if (fog_volume.m_vao != 0) {
      glDeleteBuffers(1, &fog_volume.m_vab);
      glDeleteBuffers(1, &fog_volume.m_iab);
      glDeleteVertexArrays(1, &fog_volume.m_vao);
    }
  }
}

void TerrainRenderer::exportAsRaw()
{
    ofstream out("output.obj", ios::out | ios::binary);
    for (int vi = 0; vi < m_vertices.size(); vi++)
    {
        CETerrainVertex v = m_vertices.at(vi);

        out << "v " << v.m_position.x << " " <<
            v.m_position.y << " " <<
            v.m_position.z << "\n";
    }
    for (size_t i = 0; i < m_indices.size(); i += 3) {
        uint32 ia = m_indices.at(i) + 1; // obj is base 1 idx
        uint32 ib = m_indices.at(i + (size_t)1) + 1; // obj is base 1 idx
        uint32 ic = m_indices.at(i + (size_t)2) + 1; // obj is base 1 idx

        // triangle groups
        out << "f" << " " <<
            ia << " " <<
            ib << " " <<
            ic << "\n";
    }

    out.close();
}

/*
 * precalculate transforms for all map objects
 */
void TerrainRenderer::preloadObjectMap()
{
  int map_square_size = this->m_cmap_data_weak->getHeight();
  float map_tile_length = this->m_cmap_data_weak->getTileLength();
  
  std::cout << "Precalculating world object transforms" << std::endl;
  
  for (int y = 0; y < map_square_size; y++) {
    for (int x = 0; x < map_square_size; x++) {
      int xy = (y*map_square_size)+x;
      int obj_id = this->m_cmap_data_weak->getObjectAt(xy);
      
      if (obj_id == 255 || obj_id == 254) continue;

      float object_height;
      CEWorldModel* w_obj = this->m_crsc_data_weak->getWorldModel(obj_id);
      if (w_obj == nullptr) {
          printf("Invalid object referenced: %d not found in RSC\n", obj_id);
          continue;
      }
      
      if (w_obj->getObjectInfo()->flags & objectPLACEGROUND) {
        // Use original algorithm: GetObjectH(x, y, GrRad) - finds lowest height within radius
        // Original: HMapO[y][x] = GetObjectH(x,y, MObjects[ob].info.GrRad);
        // Original rendering: v[0].y = (float)(HMapO[y][x]) * ctHScale - CameraY;
        float objectH = m_cmap_data_weak->getObjectHeightForRadius(x, y, w_obj->getObjectInfo()->GrRad);
        object_height = objectH; // Already scaled correctly in getObjectHeightForRadius
      } else {
        object_height = this->m_cmap_data_weak->getObjectHeightAt(xy);
      }
      
      int rotation_idx = (this->m_cmap_data_weak->getFlagsAt(xy) >> 2) & 3;
      glm::vec3 rotation;
      switch (rotation_idx) {
        case 0:
          rotation = glm::vec3(0, glm::radians(0.f), 0);
          break;
        case 1:
          rotation = glm::vec3(0, glm::radians(90.f), 0);
          break;
        case 2:
          rotation = glm::vec3(0, glm::radians(180.f), 0);
          break;
        case 3:
          rotation = glm::vec3(0, glm::radians(270.f), 0);
          break;
      }
      
      glm::vec3 world_position = glm::vec3(((float)(x)*map_tile_length) + map_tile_length, object_height, ((float)(y)*map_tile_length) + map_tile_length);
      
      // Scale objects down 16x to match new world scale 
      Transform transform_initial(
                                  world_position,
                                  rotation,
                                  glm::vec3(0.0625f, 0.0625f, 0.0625f) // 1/16 scale
                                  );

      w_obj->addFar(transform_initial);
      w_obj->addNear(transform_initial);
    }
  }

  auto color = this->m_crsc_data_weak->getFadeColor();
  auto dist = (m_cmap_data_weak->getTileLength() * (m_cmap_data_weak->getWidth() / 4.f));
  auto dColor = glm::vec4(color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a);

  for (int m = 0; m < this->m_crsc_data_weak->getWorldModelCount(); m++) {
    auto model = this->m_crsc_data_weak->getWorldModel(m);
    // Update instances
    model->updateNearInstances();
    // Configure shader
    model->getGeometry()->ConfigureShaderUniforms(m_cmap_data_weak.get(), m_crsc_data_weak.get());
  }
}

void TerrainRenderer::loadShader()
{
  std::ifstream f("config.json");
  json data = json::parse(f);
  
  fs::path basePath = fs::path(data["basePath"].get<std::string>());
  fs::path shaderPath = basePath / "shaders";
  
  auto color = this->m_crsc_data_weak->getFadeColor();
  float r = color.r / 255.0f;
  float g = color.g / 255.0f;
  float b = color.b / 255.0f;
  float a = color.a;

  // Define a baseline brightness factor
  float brightnessFactor = 1.0f;

  // Increase the brightness
  r = std::min(r * brightnessFactor, 1.0f);
  g = std::min(g * brightnessFactor, 1.0f);
  b = std::min(b * brightnessFactor, 1.0f);
  
  float atlas_square_size = (float)this->m_crsc_data_weak->getTextureAtlasWidth();
  
  this->m_shader = std::unique_ptr<ShaderProgram>(new ShaderProgram((shaderPath / "terrain.vs").string(), (shaderPath / "terrain.fs").string()));
  this->m_water_shader = std::unique_ptr<ShaderProgram>(new ShaderProgram((shaderPath / "water_surface.vs").string(), (shaderPath / "water_surface.fs").string()));
  this->m_fog_shader = std::unique_ptr<ShaderProgram>(new ShaderProgram((shaderPath / "fog_volume.vs").string(), (shaderPath / "fog_volume.fs").string()));

  this->m_shader->use();
  this->m_shader->setFloat("view_distance", (m_cmap_data_weak->getTileLength() * (m_cmap_data_weak->getWidth() / 8.f)));
  this->m_shader->setVec4("distanceColor", glm::vec4(r, g, b, a));
  this->m_shader->setFloat("terrainWidth", this->m_cmap_data_weak->getWidth());
  this->m_shader->setFloat("terrainHeight", this->m_cmap_data_weak->getHeight());
  this->m_shader->setFloat("tileWidth", this->m_cmap_data_weak->getTileLength());
  this->m_shader->setVec2("atlasSize", glm::vec2(atlas_square_size));

  this->m_water_shader->use();
  this->m_water_shader->setFloat("terrainWidth", this->m_cmap_data_weak->getWidth());
  this->m_water_shader->setFloat("terrainHeight", this->m_cmap_data_weak->getHeight());
  this->m_water_shader->setFloat("tileWidth", this->m_cmap_data_weak->getTileLength());
  this->m_water_shader->setVec4("skyColor", glm::vec4(r, g, b, a));
  this->m_water_shader->setFloat("view_distance", (m_cmap_data_weak->getTileLength() * (m_cmap_data_weak->getWidth() / 8.f)));
  this->m_water_shader->setVec2("atlasSize", glm::vec2(atlas_square_size));
}

void TerrainRenderer::Update(Transform& transform, Camera& camera)
{
  glm::mat4 MVP = transform.GetMVP(camera);
  glm::mat4 model = transform.GetStaticModel();
  double t = glfwGetTime();

  this->m_shader->use();
  this->m_shader->setMat4("MVP", MVP);
  this->m_shader->setMat4("model", model);
  this->m_shader->setMat4("view", camera.GetVM());
  this->m_shader->setMat4("projection", camera.GetProjection());
  this->m_shader->setFloat("time", (float)t);

  // TODO: use UBO
  this->m_water_shader->use();
  this->m_water_shader->setMat4("MVP", MVP);
  this->m_water_shader->setMat4("model", model);
  this->m_water_shader->setMat4("view", camera.GetVM());
  this->m_water_shader->setMat4("projection", camera.GetProjection());
  this->m_water_shader->setFloat("time", (float)t);
  this->m_water_shader->setVec3("cameraPos", camera.GetPosition());
  
  // Set up fog shader transforms
  this->m_fog_shader->use();
  this->m_fog_shader->setMat4("MVP", MVP);
  this->m_fog_shader->setMat4("model", model);
  this->m_fog_shader->setMat4("view", camera.GetVM());
  this->m_fog_shader->setMat4("projection", camera.GetProjection());
  this->m_fog_shader->setFloat("time", (float)t);
  this->m_fog_shader->setVec3("cameraPos", camera.GetPosition());
  
  // Water level is now set per water plane during rendering
  
  for (int m = 0; m < this->m_crsc_data_weak->getWorldModelCount(); m++) {
    this->m_crsc_data_weak->getWorldModel(m)->getGeometry()->Update(transform, camera);
  }

  m_last_update_time = t;
}

void TerrainRenderer::RenderObjects(Camera& camera)
{
  for (int m = 0; m < this->m_crsc_data_weak->getWorldModelCount(); m++) {
    this->m_crsc_data_weak->getWorldModel(m)->getGeometry()->DrawInstances();
  }
}

void TerrainRenderer::RenderObjectsWithShadows(Camera& camera, CEShadowManager* shadowManager)
{
  if (!shadowManager) {
    // Fallback to regular rendering if no shadow system
    RenderObjects(camera);
    return;
  }

  int totalModels = 0;
  int shadowModels = 0;
  int regularModels = 0;
  int totalShadowInstances = 0;
  int totalRegularInstances = 0;

  // Enable shadows in the existing simple_geo shaders
  for (int m = 0; m < this->m_crsc_data_weak->getWorldModelCount(); m++) {
    CEWorldModel* model = this->m_crsc_data_weak->getWorldModel(m);
    if (!model) continue;
    
    totalModels++;
    CEGeometry* geometry = model->getGeometry();
    if (!geometry) continue;
    
    // Check if this object should cast shadows based on metadata
    if (!CEShadowManager::shouldCastShadow(model)) {
      // Render without shadows for objects that shouldn't cast them
      geometry->DrawInstances();
      regularModels++;
      totalRegularInstances += model->getTransforms().size();
      continue;
    }
    
    // Get the shader from the geometry (the simple_geo shader)
    ShaderProgram* shader = geometry->getShader();
    if (!shader) {
      geometry->DrawInstances();
      regularModels++;
      totalRegularInstances += model->getTransforms().size();
      continue;
    }
    
    // Use the geometry's own shader but enable shadows
    shader->use();
    
    // Set camera matrices (required for instanced rendering)
    glm::mat4 projection_view = camera.getProjectionMatrix() * camera.getViewMatrix();
    shader->setMat4("projection_view", projection_view);
    shader->setMat4("view", camera.getViewMatrix());
    shader->setMat4("projection", camera.getProjectionMatrix());
    shader->setFloat("time", (float)glfwGetTime());
    
    // Enable shadows
    shader->setBool("enableShadows", true);
    
    // Set shadow/light uniforms
    shader->setMat4("lightSpaceMatrix", shadowManager->getLightSpaceMatrix());
    shader->setVec3("lightDirection", shadowManager->getLightDirection());
    
    // Bind shadow map texture to texture unit 1
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, shadowManager->getShadowMapTexture());
    shader->setInt("shadowMap", 1);
    
    // Reset to texture unit 0 for the geometry's texture
    glActiveTexture(GL_TEXTURE0);
    
    // Now draw the instances with shadows enabled (keep our shader active)
    geometry->DrawInstancesWithShader(shader);
    shadowModels++;
    totalShadowInstances += model->getTransforms().size();
  }
}

void TerrainRenderer::RenderWithShadows(Camera& camera, CEShadowManager* shadowManager)
{
  if (!shadowManager) {
    // Fallback to regular rendering if no shadow system
    Render();
    return;
  }

  // Enable shadows in the terrain shader
  this->m_shader->use();
  
  // Set shadow uniforms
  this->m_shader->setBool("enableShadows", true);
  this->m_shader->setMat4("lightSpaceMatrix", shadowManager->getLightSpaceMatrix());
  this->m_shader->setVec3("lightDirection", shadowManager->getLightDirection());
  
  // Bind shadow map texture to texture unit 3 (units 1 and 2 are used by other textures)
  glActiveTexture(GL_TEXTURE3);
  unsigned int shadowTexId = shadowManager->getShadowMapTexture();
  glBindTexture(GL_TEXTURE_2D, shadowTexId);
  this->m_shader->setInt("shadowMap", 3);
  
  // Reset to texture unit 0 for terrain texture
  glActiveTexture(GL_TEXTURE0);
  
  // Call the normal render method
  Render();
}

glm::vec3 TerrainRenderer::calcWorldVertex(int tile_x, int tile_y, bool water, float water_height_scaled)
{
  int width = this->m_cmap_data_weak->getWidth(), height = this->m_cmap_data_weak->getHeight();
  float tile_size = this->m_cmap_data_weak->getTileLength();
  
  // TODO: Define exceptions
  if (tile_x > width || tile_y > height) {
    throw 1;
  }

  float tile_height;

  if (water)
  {
    tile_height = water_height_scaled;
  } else {
    tile_height = this->m_cmap_data_weak->getHeightAt((tile_y * width) + tile_x);
  }

  float vx = (tile_x*tile_size) + (tile_size/2.f);
  float vy = (tile_y*tile_size) + (tile_size/2.f);
  
  return glm::vec3(vx, tile_height, vy);
}

/*
 00: no rotation (right-side-up)
 01: rotated 90 degrees clockwise
 10: rotated 180 degrees (upside-down)
 11: rotated 90 degrees counter-clockwise
 */
std::array<glm::vec2, 4> TerrainRenderer::calcUVMapForQuad(int x, int y, bool quad_reversed, int rotation_code)
{
  std::array<glm::vec2, 4> vertex_uv_mapping;
  x = 0; y = 0;

  // This is called "half pixel correction" - a naive implementation. See https://gamedev.stackexchange.com/questions/46963/how-to-avoid-texture-bleeding-in-a-texture-atlas for something better. Note this doesn't solve mipmaps issue
  // For solution, see answer here: https://gamedev.stackexchange.com/questions/46963/how-to-avoid-texture-bleeding-in-a-texture-atlas
  float max_tc = 1.f;
  float tu, tv;
  float i = 1.f;
  tu = 0.0f; tv = 0.0f;
  
  // See https://github.com/carnivores-cpe/carnivores_original/blob/022cc1761ddd294df8aa0b8035edceecb84ecafb/Carnivores%202/Render3DFX.cpp#L1050
  
  if (!quad_reversed) {
    switch (rotation_code) {
      case 0:
        vertex_uv_mapping = {
          glm::vec2(tu, tv),
          glm::vec2(fminf(max_tc, tu + i), tv),
          glm::vec2(tu, fminf(max_tc, tv + i)),
          glm::vec2(fminf(max_tc, tu + i), fminf(max_tc, tv + i))
        };
        break;
      case 1:
        vertex_uv_mapping = {
          glm::vec2(tu, fminf(max_tc, tv + i)),
          glm::vec2(tu, tv),
          glm::vec2(fminf(max_tc, tu + i), fminf(max_tc, tv + i)),
          glm::vec2(fminf(max_tc, tu + i), tv)
        };
        break;
      case 2:
        vertex_uv_mapping = {
          glm::vec2(fminf(max_tc, tu + i), fminf(max_tc, tv + i)),
          glm::vec2(tu, tv + i),
          glm::vec2(fminf(max_tc, tu + i), tv),
          glm::vec2(tu, tv)
        };
        break;
      case 3:
        vertex_uv_mapping = {
          glm::vec2(fminf(max_tc, tu + i), tv),
          glm::vec2(fminf(max_tc, tu + i), fminf(max_tc, tv + i)),
          glm::vec2(tu, tv),
          glm::vec2(tu, fminf(max_tc, tv + i))
        };
        break;
        
      default:
          vertex_uv_mapping = {};
        break;
    }
  } else {  // UL, UR, LL, LR
    switch (rotation_code) {
      case 0:
        vertex_uv_mapping = {
          glm::vec2(tu, tv),
          glm::vec2(fminf(max_tc, tu + i), tv),
          glm::vec2(tu, fminf(max_tc, tv + i)),
          glm::vec2(fminf(max_tc, tu + i), fminf(max_tc, tv + i))
        };
        break;
      case 1:
        vertex_uv_mapping = {
          glm::vec2(tu, fminf(max_tc, tv + i)),
          glm::vec2(tu, tv),
          glm::vec2(fminf(max_tc, tu + i), fminf(max_tc, tv + i)),
          glm::vec2(fminf(max_tc, tu + i), tv)
        };
        break;
      case 2:
        vertex_uv_mapping = {
          glm::vec2(fminf(max_tc, tu + i), fminf(max_tc, tv + i)),
          glm::vec2(tu, fminf(max_tc, tv + i)),
          glm::vec2(fminf(max_tc, tu + i), tv),
          glm::vec2(tu, tv)
        };
        break;
      case 3:
        vertex_uv_mapping = {
          glm::vec2(fminf(max_tc, tu + i), tv),
          glm::vec2(fminf(max_tc, tu + i), fminf(max_tc, tv + i)),
          glm::vec2(tu, tv),
          glm::vec2(tu, fminf(max_tc, tv + i))
        };
        break;
        
      default:
          vertex_uv_mapping = {};
        break;
    }
  }
  
  return vertex_uv_mapping;
}

glm::vec2 TerrainRenderer::scaleAtlasUV(glm::vec2 atlas_uv, int texture_id)
{
  float atlas_square_size = (float)this->m_crsc_data_weak->getTextureAtlasWidth();
  int texture_row = int(floor(float(texture_id) / atlas_square_size));
  int padding = this->m_crsc_data_weak->getAtlasTilePadding();
  int size = this->m_crsc_data_weak->getAtlasTileWidth();

  float tile_scale = (1.f / atlas_square_size);

  float u_scaled = atlas_uv.x;
  float y_scaled = atlas_uv.y;

  float padding_fraction = (float(padding) / float(size + (padding*2.f)));
  float min_uv = padding_fraction * tile_scale;
  float max_uv = (1.f - padding_fraction) * tile_scale;

  if (u_scaled == 0) {
    u_scaled = (texture_id * tile_scale) + min_uv;
  } else {
    u_scaled = (texture_id * tile_scale) + max_uv;
  }

  if (y_scaled == 0) {
    y_scaled = (texture_row * tile_scale) + min_uv;
  } else {
    y_scaled = (texture_row * tile_scale) + max_uv;
  }
  
  return glm::vec2(
                   u_scaled,
                   y_scaled
                   );
}

glm::vec4 TerrainRenderer::getScaledAtlasUVQuad(glm::vec2 atlas_uv, int texture_id_1, int texture_id_2)
{
  return glm::vec4(this->scaleAtlasUV(atlas_uv, texture_id_2), this->scaleAtlasUV(atlas_uv, texture_id_1));
}

/*
 * add to water VBO at given location
 */
void TerrainRenderer::loadWaterAt(int x, int y)
{
  if (m_crsc_data_weak->getWaterCount() < 1) {
      std::cout << "No water entities defined in map" << std::endl;
      return;
  }

  int width = this->m_cmap_data_weak->getWidth();
  int height = this->m_cmap_data_weak->getHeight();
  int xy = (y*width) + x;
  
  _Water* water_object;
  CEWaterEntity water_data;

  if (m_cmap_data_weak->m_type == CEMapType::C2) {
    // This is expected to exist in C2
    int water_index = this->m_cmap_data_weak->getWaterAt(xy);

    if (water_index > this->m_waters.size()) {
      printf("Attempted to access water_index `%i` at x: %i, y: %i, which is out of bounds\n", water_index, x, y);
      water_index = 0;
    }
    water_object = &this->m_waters[water_index];
    water_data = this->m_crsc_data_weak->getWater(water_index);
  } else {
    // C1 does not use persisted water entities. Instead, we need to construct ourselves
    // based on what is considered a "distinct" body of water.
    // Here we're distinct if the texID and height match.
    int texID = m_cmap_data_weak->getWaterTextureIDAt(xy, 0);
    if (texID == 255) {
      // This is "default" water in C1, which is always the first texture
      // Dont blame me for this
      texID = 0;
    }
    int unscaledWHeight = (int)m_cmap_data_weak->getLowestHeight(x, y, true);
    CEWaterEntity mWater;
    mWater.fogRGB = 0;
    mWater.texture_id = texID;
    mWater.transparency = 1.0f;
    mWater.water_level = unscaledWHeight + 48;
    int water_index = m_crsc_data_weak->findMatchingWater(mWater);
    if (water_index < 0) {
      // Register the water and create corresponding m_waters entry
      m_crsc_data_weak->registerDynamicWater(mWater);
      water_index = m_crsc_data_weak->findMatchingWater(mWater);
      
      if (water_index < 0) {
        throw std::runtime_error("Error building dynamic water... expected water not found!");
      }
      
      // Expand m_waters vector if needed
      while (water_index >= this->m_waters.size()) {
        _Water wd;
        const CEWaterEntity& we = this->m_crsc_data_weak->getWater(this->m_waters.size());
        
        wd.m_height_unscaled = we.water_level;
        wd.m_height = we.water_level * this->m_cmap_data_weak->getHeightmapScale();
        wd.m_texture_id = we.texture_id;
        wd.m_transparency = we.transparency;
        
        glGenVertexArrays(1, &wd.m_vao);
        glBindVertexArray(wd.m_vao);
        glGenBuffers(1, &wd.m_vab);
        glBindVertexArray(0);
        
        this->m_waters.push_back(wd);
        std::cout << "Created m_waters[" << (this->m_waters.size()-1) << "] for dynamic water" << std::endl;
      }
    }

    if (water_index < 0 || water_index >= this->m_waters.size()) {
      std::cout << "Water index " << water_index << " out of bounds (size=" << this->m_waters.size() << "), using 0" << std::endl;
      water_index = 0;
    }
    
    water_object = &this->m_waters[water_index];
    water_data = this->m_crsc_data_weak->getWater(water_index);
  }

  // Check if this water tile is in a danger zone (lava)
  bool isDangerWater = this->m_cmap_data_weak->hasDangerTileAt(this->m_crsc_data_weak, glm::vec2(x, y));

  float wheight = water_object->m_height;
  glm::vec3 vpositionLL = this->calcWorldVertex(x, y, true, wheight);
  glm::vec3 vpositionLR = this->calcWorldVertex(fmin(x + 1, height - 1), y, true, wheight);
  glm::vec3 vpositionUL = this->calcWorldVertex(x, fmin(y + 1, width - 1), true, wheight);
  glm::vec3 vpositionUR = this->calcWorldVertex(fmin(x + 1, height - 1), fmin(y + 1, width - 1), true, wheight);

  // bool quad_reverse = this->m_cmap_data_weak->isQuadRotatedAt(xy);
  bool quad_reverse = false;
  int texture_id = this->m_cmap_data_weak->getWaterTextureIDAt(xy, water_data.texture_id);

  std::array<glm::vec2, 4> vertex_uv_mapping = this->calcUVMapForQuad(x, y, false, 0);

  // Use flags parameter to pass danger water information (1 = danger water, 0 = normal water)
  uint32_t dangerFlag = isDangerWater ? 1 : 0;

  Vertex v1(vpositionLL, scaleAtlasUV(vertex_uv_mapping[0], texture_id), glm::vec3(0.0, 1.0, 0.0), false, water_object->m_transparency, texture_id, dangerFlag);
  Vertex v2(vpositionLR, scaleAtlasUV(vertex_uv_mapping[1], texture_id), glm::vec3(0.0, 1.0, 0.0), false, water_object->m_transparency, texture_id, dangerFlag);
  Vertex v3(vpositionUL, scaleAtlasUV(vertex_uv_mapping[2], texture_id), glm::vec3(0.0, 1.0, 0.0), false, water_object->m_transparency, texture_id, dangerFlag);
  Vertex v4(vpositionUR, scaleAtlasUV(vertex_uv_mapping[3], texture_id), glm::vec3(0.0, 1.0, 0.0), false, water_object->m_transparency, texture_id, dangerFlag);

  water_object->m_vertices.push_back(v1);
  water_object->m_vertices.push_back(v2);
  water_object->m_vertices.push_back(v3);
  water_object->m_vertices.push_back(v4);

    // TODO: clean this up
  unsigned int lower_left = (unsigned int)water_object->m_vertices.size() - 4;
  unsigned int lower_right = (unsigned int)water_object->m_vertices.size() - 3;
  unsigned int upper_left = (unsigned int)water_object->m_vertices.size() - 2;
  unsigned int upper_right = (unsigned int)water_object->m_vertices.size() - 1;

  if (quad_reverse) {
    water_object->m_indices.push_back(lower_left);
    water_object->m_indices.push_back(upper_left);
    water_object->m_indices.push_back(lower_right);

    water_object->m_indices.push_back(lower_right);
    water_object->m_indices.push_back(upper_left);
    water_object->m_indices.push_back(upper_right);
  } else {
    water_object->m_indices.push_back(lower_left);
    water_object->m_indices.push_back(upper_right);
    water_object->m_indices.push_back(lower_right);

    water_object->m_indices.push_back(lower_left);
    water_object->m_indices.push_back(upper_left);
    water_object->m_indices.push_back(upper_right);
  }
  
  static int addCount = 0;
  addCount++;
  if (addCount < 10) {
    std::cout << "Added vertices to water " << water_object << ": now has " << water_object->m_vertices.size() << " vertices, " << water_object->m_indices.size() << " indices" << std::endl;
  }
}

void TerrainRenderer::loadWaterIntoMemory()
{
    std::cout << "=== Starting loadWaterIntoMemory ===" << std::endl;
    std::vector<float> underwaterStateData(m_cmap_data_weak->getWidth() * m_cmap_data_weak->getHeight(), 0);
    auto width = m_cmap_data_weak->getWidth();
    auto height = m_cmap_data_weak->getHeight();
    std::cout << "Map dimensions: " << width << "x" << height << " tiles" << std::endl;
    auto tileL = m_cmap_data_weak->getTileLength();

    for (int w = 0; w < this->m_waters.size(); w++)
    {
        _Water* water_object = &this->m_waters[w];
        water_object->m_vertex_count = (int)water_object->m_vertices.size();
        water_object->m_num_indices = (int)water_object->m_indices.size();
        
        // Set underwater state in the texture data
        for (auto& vertex : water_object->m_vertices) {
            int x = static_cast<int>(vertex.getPos().x / tileL); // Convert to tile coordinate
            int y = static_cast<int>(vertex.getPos().z / tileL); // Convert to tile coordinate
            if (x >= 0 && x < width && y >= 0 && y < height) {
                underwaterStateData[(y * width) + x] = vertex.getPos().y;
            }
        }

        glBindVertexArray(water_object->m_vao);

        glBindBuffer(GL_ARRAY_BUFFER, water_object->m_vab);
        glBufferData(GL_ARRAY_BUFFER, (int)water_object->m_vertices.size() * sizeof(Vertex), water_object->m_vertices.data(), GL_STATIC_DRAW);

        // Describe vertex details
        glEnableVertexAttribArray(0); // position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
        glEnableVertexAttribArray(1); // uv
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(glm::vec3));
        glEnableVertexAttribArray(2); // normal
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec3) + sizeof(glm::vec2)));
        glEnableVertexAttribArray(3); // alpha
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(glm::vec3)));
        glEnableVertexAttribArray(4); // flags
        glVertexAttribPointer(4, 1, GL_UNSIGNED_INT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(glm::vec3) + sizeof(float)));

        glGenBuffers(1, &water_object->m_iab);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, water_object->m_iab);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (int)water_object->m_indices.size() * sizeof(unsigned int), water_object->m_indices.data(), GL_STATIC_DRAW);

        glBindVertexArray(0);
    }

    // Generate and bind the underwater state texture
    glGenTextures(1, &underwaterStateTexture);
    std::cout << "Generated underwaterStateTexture ID: " << underwaterStateTexture << std::endl;
    glBindTexture(GL_TEXTURE_2D, underwaterStateTexture);

    // Initialize the texture with the data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, underwaterStateData.data());
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);

    // Create the heightmap texture for depth-based water rendering
    createHeightmapTexture();
  
  GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR) {
      std::cerr << "OpenGL error at " << "loadWaterIntoMem" << ": " << err << std::endl;
  }
}

void TerrainRenderer::updateUnderwaterStateTexture(const std::vector<float>& data)
{
    glBindTexture(GL_TEXTURE_2D, underwaterStateTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_cmap_data_weak->getWidth(), m_cmap_data_weak->getHeight(), GL_RED, GL_FLOAT, data.data());
    glBindTexture(GL_TEXTURE_2D, 0);
  
  GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR) {
      std::cerr << "OpenGL error at " << "updateUnderwaterStateTexture" << ": " << err << std::endl;
  }
}

// From http://www.learnopengles.com/android-lesson-eight-an-introduction-to-index-buffer-objects-ibos/
// using http://stackoverflow.com/questions/10114577/a-method-for-indexing-triangles-from-a-loaded-heightmap
void TerrainRenderer::loadIntoHardwareMemory()
{
  int width = this->m_cmap_data_weak->getWidth(), height = this->m_cmap_data_weak->getHeight();

  // build water base objects
  this->m_waters.clear();
  for (int w = 0; w < this->m_crsc_data_weak->getWaterCount(); w++)
  {
    _Water wd;
    const CEWaterEntity& we = this->m_crsc_data_weak->getWater(w);

    wd.m_height_unscaled = we.water_level;
    wd.m_height = we.water_level * this->m_cmap_data_weak->getHeightmapScale();
    wd.m_texture_id = we.texture_id;
    wd.m_transparency = we.transparency;
    
    std::cout << "Water " << w << ": unscaled=" << we.water_level << ", scaled=" << wd.m_height 
              << ", scale=" << this->m_cmap_data_weak->getHeightmapScale() << std::endl;

    glGenVertexArrays(1, &wd.m_vao);
    glBindVertexArray(wd.m_vao);
    
    glGenBuffers(1, &wd.m_vab);

    glBindVertexArray(0);

    this->m_waters.push_back(wd);
  }
  
  // Generate water geometry AFTER water objects are created
  std::cout << "Generating water geometry for " << width << "x" << height << " tiles" << std::endl;
  int waterTileCount = 0;
  for (int y = 0; y < width; y++) {
    for (int x = 0; x < height; x++) {
      unsigned int base_index = (y * width) + x;
      bool hasWater = this->m_cmap_data_weak->hasWaterAt(base_index);
      
      // For C1 maps, also check if either quad texture matches any water entity texture
      // BUT only if adjacent to explicitly marked water (to avoid false positives on muddy ground)
      if (!hasWater && this->m_cmap_data_weak->m_type == CEMapType::C1) {
        // Check texture A
        int texID_A = this->m_cmap_data_weak->getTextureIDAt(base_index);
        if (texID_A == 255) texID_A = 0; // Default water texture
        
        // Check texture B  
        int texID_B = this->m_cmap_data_weak->getSecondaryTextureIDAt(base_index);
        if (texID_B == 255) texID_B = 0; // Default water texture
        
        // Check if either texture ID is used by any existing water entity
        bool hasWaterTexture = false;
        for (int i = 0; i < this->m_crsc_data_weak->getWaterCount(); i++) {
          const CEWaterEntity& waterEntity = this->m_crsc_data_weak->getWater(i);
          if (waterEntity.texture_id == texID_A || waterEntity.texture_id == texID_B) {
            hasWaterTexture = true;
            break;
          }
        }
        
        // If has water texture, check if adjacent to explicitly marked water
        if (hasWaterTexture) {
          // Check 8 adjacent tiles (including diagonals)
          int adjacentWaterX = -1, adjacentWaterY = -1;
          for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
              if (dx == 0 && dy == 0) continue; // Skip center tile
              
              int adj_x = x + dx;
              int adj_y = y + dy;
              
              // Check bounds
              if (adj_x >= 0 && adj_x < height && adj_y >= 0 && adj_y < width) {
                int adj_index = (adj_y * width) + adj_x;
                if (this->m_cmap_data_weak->hasWaterAt(adj_index)) {
                  hasWater = true;
                  adjacentWaterX = adj_x;
                  adjacentWaterY = adj_y;
                  break;
                }
              }
            }
            if (hasWater) break;
          }
          
          // If we found adjacent water, get the adjacent tile's water entity info
          if (hasWater && adjacentWaterX >= 0 && adjacentWaterY >= 0) {
            // Get the water entity used by the adjacent water tile
            int adj_index = (adjacentWaterY * width) + adjacentWaterX;
            int adj_texID = m_cmap_data_weak->getWaterTextureIDAt(adj_index, 0);
            if (adj_texID == 255) adj_texID = 0;
            int adj_unscaledWHeight = (int)m_cmap_data_weak->getLowestHeight(adjacentWaterX, adjacentWaterY, true);
            
            // Create a temporary water entity matching the adjacent water
            CEWaterEntity adjWater;
            adjWater.fogRGB = 0;
            adjWater.texture_id = adj_texID;
            adjWater.transparency = 1.0f;
            adjWater.water_level = adj_unscaledWHeight + 48;
            
            // Find the matching water entity for this adjacent water
            int adj_water_index = m_crsc_data_weak->findMatchingWater(adjWater);
            if (adj_water_index >= 0) {
              // Store the water index to force this tile to use the same water entity
              // We'll create a simple way to pass this to loadWaterAt
              
              // For now, let's directly call a modified loadWaterAt that uses this specific water index
              waterTileCount++;
              if (waterTileCount < 10) {
                std::cout << "Found edge case water at tile (" << x << "," << y << ") using water index " << adj_water_index << std::endl;
              }
              loadWaterAtWithIndex(x, y, adj_water_index);
              hasWater = false; // Prevent the regular loadWaterAt call below
            }
          }
        }
      }
      
      if (hasWater) {
        waterTileCount++;
        if (waterTileCount < 10) { // Show first 10
          std::cout << "Found water at tile (" << x << "," << y << ") - calling loadWaterAt" << std::endl;
        }
        loadWaterAt(x, y);
      }
    }
  }
  std::cout << "Water generation complete - found " << waterTileCount << " water tiles" << std::endl;
  std::cout << "m_waters vector size: " << this->m_waters.size() << std::endl;
  for (int i = 0; i < std::min(5, (int)this->m_waters.size()); i++) {
    std::cout << "m_waters[" << i << "] addr=" << &this->m_waters[i] << " vertices=" << this->m_waters[i].m_vertices.size() << std::endl;
  }
  
  // Structures to store accumulated normals and counts for averaging
  std::vector<glm::vec3> vertexNormals(width * height, glm::vec3(0.0f));
  std::vector<int> normalCounts(width * height, 0);
  
  std::cout << "Generating terrain normal map" << std::endl;
  
  for (int y = 0; y < width; y++) {
      for (int x = 0; x < height; x++) {
          unsigned int base_index = (y * width) + x;

          glm::vec3 vpositionLL = this->calcWorldVertex(x, y, false, 0.f);
          glm::vec3 vpositionLR = this->calcWorldVertex(fmin(x + 1, height - 1), y, false, 0.f);
          glm::vec3 vpositionUL = this->calcWorldVertex(x, fmin(y + 1, width - 1), false, 0.f);
          glm::vec3 vpositionUR = this->calcWorldVertex(fmin(x + 1, height - 1), fmin(y + 1, width - 1), false, 0.f);

          // Calculate face normals
          glm::vec3 normal1, normal2;
          if (this->m_cmap_data_weak->isQuadRotatedAt(base_index)) {
              normal1 = calculateFaceNormal(vpositionLL, vpositionUL, vpositionLR);
              normal2 = calculateFaceNormal(vpositionLR, vpositionUL, vpositionUR);
          } else {
              normal1 = calculateFaceNormal(vpositionLL, vpositionUR, vpositionLR);
              normal2 = calculateFaceNormal(vpositionLL, vpositionUL, vpositionUR);
          }

        // Accumulate face normals for each vertex with boundary checks
        if (y < height && x < width) {
            vertexNormals[y * width + x] += normal1;
            normalCounts[y * width + x]++;
        }
        if (y < height && x + 1 < width) {
            vertexNormals[y * width + (x + 1)] += normal1;
            normalCounts[y * width + (x + 1)]++;
        }
        if (y + 1 < height && x < width) {
            vertexNormals[(y + 1) * width + x] += normal1;
            normalCounts[(y + 1) * width + x]++;
        }
        if (y + 1 < height && x + 1 < width) {
            vertexNormals[(y + 1) * width + (x + 1)] += normal2;
            normalCounts[(y + 1) * width + (x + 1)]++;
        }
      }
  }

  // Normalize accumulated normals
  for (int i = 0; i < vertexNormals.size(); ++i) {
      vertexNormals[i] = glm::normalize(vertexNormals[i]);
  }

  std::cout << "Building terrain mesh" << std::endl;
  for (int y=0; y < width; y++) {
    for (int x=0; x < height; x++) {
      unsigned int base_index = (y * width) + x;
      
      int texID = this->m_cmap_data_weak->getTextureIDAt(base_index);
      int texID2 = this->m_cmap_data_weak->getSecondaryTextureIDAt(base_index);
      uint16_t flags = this->m_cmap_data_weak->getFlagsAt(x, y);

      // Water generation moved to after water objects are created

      glm::vec3 vpositionLL = this->calcWorldVertex(x, y, false, 0.f);
      glm::vec3 vpositionLR = this->calcWorldVertex(fmin(x + 1, height - 1), y, false, 0.f);
      glm::vec3 vpositionUL = this->calcWorldVertex(x, fmin(y + 1, width - 1), false, 0.f);
      glm::vec3 vpositionUR = this->calcWorldVertex(fmin(x + 1, height - 1), fmin(y + 1, width - 1), false, 0.f);
      
      bool quad_reverse = this->m_cmap_data_weak->isQuadRotatedAt(base_index);
      int texture_direction = (flags & 3);
      
      std::array<glm::vec2, 4> vertex_uv_mapping = this->calcUVMapForQuad(x, y, quad_reverse, texture_direction);
      
      glm::vec3 normalLL = vertexNormals[y * width + x];
      glm::vec3 normalLR = (x + 1 < width) ? vertexNormals[y * width + (x + 1)] : normalLL;
      glm::vec3 normalUL = (y + 1 < height) ? vertexNormals[(y + 1) * width + x] : normalLL;
      glm::vec3 normalUR = (y + 1 < height && x + 1 < width) ? vertexNormals[(y + 1) * width + (x + 1)] : normalLL;

      CETerrainVertex v1(vpositionLL, this->getScaledAtlasUVQuad(vertex_uv_mapping[0], texID, texID2), normalLL);
      CETerrainVertex v2(vpositionLR, this->getScaledAtlasUVQuad(vertex_uv_mapping[1], texID, texID2), normalLR);
      CETerrainVertex v3(vpositionUL, this->getScaledAtlasUVQuad(vertex_uv_mapping[2], texID, texID2), normalUL);
      CETerrainVertex v4(vpositionUR, this->getScaledAtlasUVQuad(vertex_uv_mapping[3], texID, texID2), normalUR);
      
      m_vertices.push_back(v1);
      m_vertices.push_back(v2);
      m_vertices.push_back(v3);
      m_vertices.push_back(v4);

      unsigned int lower_left = (y * width * 4) + (x*4);
      unsigned int lower_right = lower_left + 1;
      unsigned int upper_left = lower_right + 1;
      unsigned int upper_right = upper_left + 1;

      // If quad reverse, anchor upper right (bottom left, upper right, lower right). Otherwise, anchor upper left (bottom left, upper left, lower right)
      // Clockwise order.
      
      // Memoize the height
      float centerHeight;
      glm::vec3 avgNormal;
      
      if (quad_reverse) {
        centerHeight = (vpositionLL.y + vpositionUL.y + vpositionUR.y + vpositionLR.y) / 4.0f;

        m_indices.push_back(lower_left); // Face 1
        m_indices.push_back(upper_left);
        m_indices.push_back(lower_right);

        m_indices.push_back(lower_right); // Face 2
        m_indices.push_back(upper_left);
        m_indices.push_back(upper_right);
        
        avgNormal = glm::normalize(normalLL + normalUL + normalUR + normalLR);
      } else {
        centerHeight = (vpositionLL.y + vpositionLR.y + vpositionUL.y + vpositionUR.y) / 4.0f;

        m_indices.push_back(lower_left); // Face 1
        m_indices.push_back(upper_right);
        m_indices.push_back(lower_right);

        m_indices.push_back(lower_left); // Face 2
        m_indices.push_back(upper_left);
        m_indices.push_back(upper_right);
        
        avgNormal = glm::normalize(normalLL + normalLR + normalUL + normalUR);
      }
      
      float slopeAngle = glm::degrees(atan2(glm::length(glm::vec2(avgNormal.x, avgNormal.z)), avgNormal.y));

      m_cmap_data_weak->setGroundLevelAt(x, y, centerHeight, slopeAngle);
    }
  }
  
  std::cout << "Generating AI walkability map from terrain data" << std::endl;
  for (int y=0; y < width; y++) {
    for (int x=0; x < height; x++) {
      bool walkable = true;
      int xy = (y * width) + x;
      if (m_cmap_data_weak->hasWaterAt(x, y))
      {
        walkable = false;
      } else if (m_cmap_data_weak->hasDangerTileAt(m_crsc_data_weak, glm::vec2(x, y))) {
        walkable = false;
      } else if (m_cmap_data_weak->getGroundAngleAt(x, y) > 35.f) {
        walkable = false;
      }
//      } else {
//        auto objectIdx = m_cmap_data_weak->getObjectAt(xy);
//        if (objectIdx < 254) {
//          auto obj = m_crsc_data_weak->getWorldModel(objectIdx);
//          if (obj->getObjectInfo()->Radius > 32.f) {
//            walkable = false;
//          }
//        }
//      }
      
      if (!walkable) {
        m_cmap_data_weak->setWalkableFlagsAt(glm::vec2(x, y), 0x1);
      } else {
        m_cmap_data_weak->setWalkableFlagsAt(glm::vec2(x, y), 0x0);
      }
    }
  }
  
  m_num_indices = (int)m_indices.size();
  
  // generate buffers and upload
  glGenVertexArrays(1, &this->m_vertex_array_object);
  glBindVertexArray(this->m_vertex_array_object);
  
  glGenBuffers(1, &this->m_vertex_array_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, this->m_vertex_array_buffer);
  glBufferData(GL_ARRAY_BUFFER, (int)m_vertices.size()*sizeof(CETerrainVertex), m_vertices.data(), GL_STATIC_DRAW);
  
  // describe vertex details
  glEnableVertexAttribArray(0); // position
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(CETerrainVertex), 0);
  
  glEnableVertexAttribArray(1); // normal
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(CETerrainVertex), (void*)sizeof(glm::vec3));

  glEnableVertexAttribArray(2); // texture coords
  glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(CETerrainVertex), (void*)(sizeof(glm::vec3)+sizeof(glm::vec3)));

  glEnableVertexAttribArray(3); // brightness
  glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(CETerrainVertex), (void*)(sizeof(glm::vec3)+sizeof(glm::vec3)+(sizeof(glm::vec4))));
  
  // indices
  glGenBuffers(1, &this->m_indices_array_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_indices_array_buffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, (int)m_indices.size()*sizeof(unsigned int), m_indices.data(), GL_STATIC_DRAW);
  
  glBindVertexArray(0);

  this->loadWaterIntoMemory();
  this->loadFogVolumesIntoMemory();
}

// Calculate the real UV coords using the atlas
glm::vec2 TerrainRenderer::calcAtlasUV(int texID, glm::vec2 uv)
{
  double f3;
  float atlas_width_squares = this->m_crsc_data_weak->getTextureAtlasWidth();
  int tex_y = int(std::floor(float(texID) / atlas_width_squares));
  
  double f2 = std::modf((float)uv.x/(float)atlas_width_squares, &f3);
  double f4 = std::modf((float)uv.y/(float)atlas_width_squares, &f3);
  
  return glm::vec2((1.f/atlas_width_squares) * (texID + f2), (1.f/atlas_width_squares) * (tex_y + f4));
}

void TerrainRenderer::Render()
{
  this->m_shader->use();
  
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, underwaterStateTexture);
  this->m_shader->setInt("underwaterStateTexture", 1);

  this->m_shader->bindTexture("skyTexture", m_crsc_data_weak->getDaySky()->getTextureID(), 2);
  
  glBindVertexArray(this->m_vertex_array_object);
  
  glDrawElementsBaseVertex(GL_TRIANGLES, m_num_indices, GL_UNSIGNED_INT, 0, 0);
  
  glBindVertexArray(0);
}

void TerrainRenderer::RenderWater()
{
  this->m_water_shader->use();
  this->m_water_shader->setFloat("time", (float)glfwGetTime()); // Fix: Update time for water animation
  this->m_water_shader->bindTexture("skyTexture", m_crsc_data_weak->getDaySky()->getTextureID(), 2);
  this->m_water_shader->bindTexture("heightmapTexture", heightmapTexture, 3);

  for (int w = 0; w < this->m_waters.size(); w++) {
    // Skip empty water planes (original logic)
    if (this->m_waters[w].m_vertices.size() < 30) continue;
    
    // Set the correct water level for this specific water plane
    float waterHeight = this->m_waters[w].m_height;

    // If stored height is 0, try to get it from vertex data
    if (waterHeight == 0.0f && !this->m_waters[w].m_vertices.empty()) {
        // Sample a few vertices to get a good water height (some might be at edges)
        float totalHeight = 0.0f;
        int maxSamples = std::min(10, (int)this->m_waters[w].m_vertices.size());
        
        for (int i = 0; i < maxSamples; i++) {
            float vertexY = this->m_waters[w].m_vertices[i].getPos().y;
            totalHeight += vertexY;
        }
    }
    
    this->m_water_shader->setFloat("waterLevel", waterHeight);
    this->m_water_shader->setFloat("heightmapScale", this->m_cmap_data_weak->getHeightmapScale());

    glBindVertexArray(this->m_waters[w].m_vao);

    glDrawElementsBaseVertex(GL_TRIANGLES, this->m_waters[w].m_num_indices, GL_UNSIGNED_INT, 0, 0);
  }

  glBindVertexArray(0);
}

void TerrainRenderer::RenderFogVolumes()
{
  if (m_fog_volumes.empty()) {
    static bool logged = false;
    if (!logged) {
      std::cout << "No fog volumes to render" << std::endl;
      logged = true;
    }
    return;
  }
  
  static bool logged = false;
  if (!logged) {
    std::cout << "Rendering " << m_fog_volumes.size() << " fog volumes" << std::endl;
    logged = true;
  }
  
  // Enable blending for transparent fog
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDepthMask(GL_FALSE); // Don't write to depth buffer for transparency
  glDisable(GL_CULL_FACE); // Disable culling so fog is visible from all angles
  
  this->m_fog_shader->use();
  this->m_fog_shader->bindTexture("skyTexture", m_crsc_data_weak->getDaySky()->getTextureID(), 2);
  
  // Set common uniforms for fog shader (similar to water shader)
  this->m_fog_shader->setFloat("terrainWidth", this->m_cmap_data_weak->getWidth());
  this->m_fog_shader->setFloat("terrainHeight", this->m_cmap_data_weak->getHeight());
  this->m_fog_shader->setFloat("tileWidth", this->m_cmap_data_weak->getTileLength());
  this->m_fog_shader->setFloat("view_distance", (m_cmap_data_weak->getTileLength() * (m_cmap_data_weak->getWidth() / 8.f)));
  
  // Get sky color for fog blending
  glm::vec4 skyColor = m_crsc_data_weak->getFadeColor();
  skyColor.r /= 255.0f;
  skyColor.g /= 255.0f; 
  skyColor.b /= 255.0f;
  skyColor.a /= 255.0f;
  this->m_fog_shader->setVec4("skyColor", skyColor);
  
  for (size_t fv_idx = 0; fv_idx < m_fog_volumes.size(); fv_idx++) {
    auto& fog_volume = m_fog_volumes[fv_idx];
    
    // Skip empty fog volumes
    if (fog_volume.m_vertices.empty()) {
      continue;
    }
    
    // Set fog-specific uniforms
    float fogTransparency = fog_volume.m_fog_data.transparency;
    
    // Normalize transparency from map data (likely 0-255 or 0-1000 scale)
    if (fogTransparency > 1.0f) {
      if (fogTransparency > 255.0f) {
        fogTransparency = fogTransparency / 1000.0f; // 0-1000 scale
      } else {
        fogTransparency = fogTransparency / 255.0f;  // 0-255 scale
      }
    }
    
    // Ensure reasonable visibility for fog
    fogTransparency = std::max(0.1f, std::min(0.8f, fogTransparency)); // Min 10%, max 80% transparency
    
    // Extract fog color from RGB value
    int rgb = fog_volume.m_fog_data.rgb;
    
    // Extract RGB like original Carnivores code, but swap R and B channels
    float b = ((rgb >> 16) & 0xFF) / 255.0f; // Blue from high bits
    float g = ((rgb >> 8) & 0xFF) / 255.0f;  // Green from middle bits
    float r = (rgb & 0xFF) / 255.0f;         // Red from low bits
    
    this->m_fog_shader->setFloat("fogTransparency", fogTransparency);
    this->m_fog_shader->setVec3("fogColor", glm::vec3(r, g, b));

    // Create and bind VAO/VBO for this fog volume if not already done
    if (fog_volume.m_vao == 0) {
      // Generate OpenGL objects
      glGenVertexArrays(1, &fog_volume.m_vao);
      glGenBuffers(1, &fog_volume.m_vab);
      glGenBuffers(1, &fog_volume.m_iab);
      
      // Bind and upload vertex data
      glBindVertexArray(fog_volume.m_vao);
      
      glBindBuffer(GL_ARRAY_BUFFER, fog_volume.m_vab);
      glBufferData(GL_ARRAY_BUFFER, fog_volume.m_vertices.size() * sizeof(Vertex), fog_volume.m_vertices.data(), GL_STATIC_DRAW);
      
      // Set vertex attributes (same layout as water/terrain)
      glEnableVertexAttribArray(0); // position
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
      
      glEnableVertexAttribArray(1); // texCoord
      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
      
      glEnableVertexAttribArray(2); // normal
      glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(5 * sizeof(float)));
      
      glEnableVertexAttribArray(3); // alpha
      glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(8 * sizeof(float)));
      
      glEnableVertexAttribArray(4); // flags
      glVertexAttribIPointer(4, 1, GL_UNSIGNED_INT, sizeof(Vertex), (void*)(9 * sizeof(float)));
      
      // Upload index data
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fog_volume.m_iab);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, fog_volume.m_indices.size() * sizeof(unsigned int), fog_volume.m_indices.data(), GL_STATIC_DRAW);
    }
    
    // Bind texture (use first texture in atlas for noise)
    auto texture = this->m_crsc_data_weak->getTexture(fog_volume.m_texture_id);
    if (texture) {
      this->m_fog_shader->bindTexture("basic_texture", texture->getTextureID(), 1);
    }
    
    glBindVertexArray(fog_volume.m_vao);
    glDrawElements(GL_TRIANGLES, fog_volume.m_num_indices, GL_UNSIGNED_INT, 0);
  }
  
  // Restore OpenGL state
  glBindVertexArray(0);
  glDepthMask(GL_TRUE);
  glDisable(GL_BLEND);
  glEnable(GL_CULL_FACE); // Re-enable culling
}

void TerrainRenderer::createHeightmapTexture()
{
    int width = m_cmap_data_weak->getWidth();
    int height = m_cmap_data_weak->getHeight();
    
    // Create heightmap data - get the actual terrain height, not water height
    std::vector<float> heightmapData(width * height);
    float minHeight = 999999.0f, maxHeight = -999999.0f;
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int index = y * width + x;
            // Use the new method that correctly handles C1 vs C2 terrain height
            float terrainHeight = m_cmap_data_weak->getTerrainHeightAt(index);
            heightmapData[index] = terrainHeight;
            
            minHeight = std::min(minHeight, terrainHeight);
            maxHeight = std::max(maxHeight, terrainHeight);
        }
    }
    
    std::cout << "Terrain height range: " << minHeight << " to " << maxHeight << std::endl;
    
    // Generate and bind the heightmap texture
    glGenTextures(1, &heightmapTexture);
    std::cout << "Generated heightmapTexture ID: " << heightmapTexture << std::endl;
    glBindTexture(GL_TEXTURE_2D, heightmapTexture);
    
    // Initialize the texture with the height data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, heightmapData.data());
    
    // Set texture parameters for smooth interpolation
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    std::cout << "Created heightmap texture with " << width << "x" << height << " resolution" << std::endl;
}

/*
 * add to water VBO at given location, forcing specific water index for edge cases
 */
void TerrainRenderer::loadWaterAtWithIndex(int x, int y, int forceWaterIndex)
{
  if (forceWaterIndex < 0 || forceWaterIndex >= this->m_waters.size()) {
    std::cout << "Invalid forced water index " << forceWaterIndex << ", falling back to regular detection" << std::endl;
    loadWaterAt(x, y);
    return;
  }

  int width = this->m_cmap_data_weak->getWidth();
  int height = this->m_cmap_data_weak->getHeight();
  
  // Use the forced water index directly
  _Water* water_object = &this->m_waters[forceWaterIndex];
  CEWaterEntity water_data = this->m_crsc_data_weak->getWater(forceWaterIndex);

  // Check if this water tile is in a danger zone (lava)
  bool isDangerWater = this->m_cmap_data_weak->hasDangerTileAt(this->m_crsc_data_weak, glm::vec2(x, y));

  float wheight = water_object->m_height;
  glm::vec3 vpositionLL = this->calcWorldVertex(x, y, true, wheight);
  glm::vec3 vpositionLR = this->calcWorldVertex(fmin(x + 1, height - 1), y, true, wheight);
  glm::vec3 vpositionUL = this->calcWorldVertex(x, fmin(y + 1, width - 1), true, wheight);
  glm::vec3 vpositionUR = this->calcWorldVertex(fmin(x + 1, height - 1), fmin(y + 1, width - 1), true, wheight);

  int base_vertex_index = water_object->m_vertices.size();

  // Add vertices (using correct constructor with all 7 parameters)
  int texture_id = water_data.texture_id;
  
  std::array<glm::vec2, 4> vertex_uv_mapping = this->calcUVMapForQuad(x, y, false, 0);
  
  // Use flags parameter to pass danger water information (1 = danger water, 0 = normal water)
  uint32_t dangerFlag = isDangerWater ? 1 : 0;
  
  // Water uses atlas textures - scale UV coordinates with texture ID like original loadWaterAt
  Vertex v1(vpositionLL, scaleAtlasUV(vertex_uv_mapping[0], texture_id), glm::vec3(0.0, 1.0, 0.0), false, water_object->m_transparency, texture_id, dangerFlag);
  Vertex v2(vpositionLR, scaleAtlasUV(vertex_uv_mapping[1], texture_id), glm::vec3(0.0, 1.0, 0.0), false, water_object->m_transparency, texture_id, dangerFlag);
  Vertex v3(vpositionUL, scaleAtlasUV(vertex_uv_mapping[2], texture_id), glm::vec3(0.0, 1.0, 0.0), false, water_object->m_transparency, texture_id, dangerFlag);
  Vertex v4(vpositionUR, scaleAtlasUV(vertex_uv_mapping[3], texture_id), glm::vec3(0.0, 1.0, 0.0), false, water_object->m_transparency, texture_id, dangerFlag);

  water_object->m_vertices.push_back(v1);
  water_object->m_vertices.push_back(v2);
  water_object->m_vertices.push_back(v3);
  water_object->m_vertices.push_back(v4);

  // Add indices for two triangles
  water_object->m_indices.push_back(base_vertex_index + 0); // LL
  water_object->m_indices.push_back(base_vertex_index + 1); // LR
  water_object->m_indices.push_back(base_vertex_index + 2); // UL

  water_object->m_indices.push_back(base_vertex_index + 1); // LR
  water_object->m_indices.push_back(base_vertex_index + 3); // UR
  water_object->m_indices.push_back(base_vertex_index + 2); // UL

  water_object->m_num_indices = water_object->m_indices.size();
}

/*
 * Load fog volumes into memory and create geometry
 */
void TerrainRenderer::loadFogVolumesIntoMemory()
{
  std::cout << "=== Starting loadFogVolumesIntoMemory ===" << std::endl;
  
  auto width = m_cmap_data_weak->getWidth();
  auto height = m_cmap_data_weak->getHeight();
  std::cout << "Scanning map for fog zones: " << width << "x" << height << " tiles" << std::endl;
  
  // Find all fog zones by scanning the fog map at proper resolution
  // Fog map is at half resolution, so scan every 2nd tile to avoid duplicates
  std::map<int, std::vector<glm::vec2>> fogZones; // fog_id -> list of fog map cells (not individual tiles)
  
  // Scan at fog map resolution (every 2x2 tile block)
  for (int y = 0; y < height; y += 2) {
    for (int x = 0; x < width; x += 2) {
      int fogIndex = m_cmap_data_weak->getFogIndexAt(x, y);
      if (fogIndex >= 0) {
        // Store fog map coordinates (which represent 2x2 tile blocks)
        fogZones[fogIndex].push_back(glm::vec2(x >> 1, y >> 1));
      }
    }
  }
  
  std::cout << "Found " << fogZones.size() << " unique fog zones" << std::endl;
  
  // Generate fog volumes for each zone
  for (const auto& zone : fogZones) {
    int fogIndex = zone.first;
    const auto& tiles = zone.second;
    
    if (tiles.empty()) continue;
    
    // Get fog data
    FogData fogData = m_crsc_data_weak->getFog(fogIndex);
    
    // Calculate bounding box of fog zone (in fog map coordinates)
    float minX = tiles[0].x, maxX = tiles[0].x;
    float minY = tiles[0].y, maxY = tiles[0].y;
    
    for (const auto& tile : tiles) {
      minX = std::min(minX, tile.x);
      maxX = std::max(maxX, tile.x);
      minY = std::min(minY, tile.y);
      maxY = std::max(maxY, tile.y);
    }
    
    // Convert fog map coordinates to world tile coordinates (multiply by 2)
    // Each fog map cell covers a 2x2 tile area
    glm::vec2 centerFogCoords = glm::vec2((minX + maxX) * 0.5f, (minY + maxY) * 0.5f);
    glm::vec2 sizeFogCoords = glm::vec2(maxX - minX + 1, maxY - minY + 1);
    
    // Convert to world tile coordinates
    glm::vec2 center = centerFogCoords * 2.0f;  // Scale up to world coordinates
    glm::vec2 size = sizeFogCoords * 2.0f;      // Scale up size as well
    
    std::cout << "Generating fog volume " << fogIndex << " at center (" << center.x << "," << center.y 
              << ") size (" << size.x << "," << size.y << ")" << std::endl;
    
    generateFogVolume(fogIndex, fogData, center, size);
  }
  
  std::cout << "Generated " << m_fog_volumes.size() << " fog volumes" << std::endl;
  
  // Debug: Print details about each fog volume
  for (size_t i = 0; i < m_fog_volumes.size(); i++) {
    const auto& fv = m_fog_volumes[i];
    std::cout << "Fog volume " << i << ": center(" << fv.m_center.x << "," << fv.m_center.y 
              << ") size(" << fv.m_size.x << "," << fv.m_size.y 
              << ") vertices=" << fv.m_vertices.size() 
              << " danger=" << fv.m_fog_data.danger 
              << " transparency=" << fv.m_fog_data.transparency
              << " altitude=" << fv.m_fog_data.altitude
              << " rgb=" << fv.m_fog_data.rgb << std::endl;
              
    // Print first vertex position for debugging
    if (!fv.m_vertices.empty()) {
      glm::vec3 pos = fv.m_vertices[0].getPos();
      std::cout << "  First vertex position: (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
    }
  }
}

/*
 * Generate a single fog volume
 */
void TerrainRenderer::generateFogVolume(int fog_id, const FogData& fog_data, glm::vec2 center, glm::vec2 size)
{
  _FogVolume fog_volume;
  fog_volume.m_fog_id = fog_id;
  fog_volume.m_fog_data = fog_data;
  fog_volume.m_center = center;
  fog_volume.m_size = size;
  fog_volume.m_texture_id = 0; // Use first texture for noise
  
  // Create layered geometry for volumetric effect
  createFogVolumeGeometry(fog_volume, 6); // More layers for better volume effect and ground coverage
  
  m_fog_volumes.push_back(fog_volume);
}

/*
 * Create layered geometry for fog volume
 */
void TerrainRenderer::createFogVolumeGeometry(_FogVolume& fog_volume, int num_layers)
{
  float tileSize = m_cmap_data_weak->getTileLength();
  
  // Calculate world coordinates
  glm::vec2 worldCenter = fog_volume.m_center * tileSize;
  glm::vec2 worldSize = fog_volume.m_size * tileSize;
  
  std::cout << "Creating fog geometry: tileSize=" << tileSize 
            << " center(" << fog_volume.m_center.x << "," << fog_volume.m_center.y << ")"
            << " worldCenter(" << worldCenter.x << "," << worldCenter.y << ")"
            << " size(" << fog_volume.m_size.x << "," << fog_volume.m_size.y << ")"
            << " worldSize(" << worldSize.x << "," << worldSize.y << ")" << std::endl;
  
  // Get terrain height at center for fog base altitude
  // getTerrainHeightAt() already returns scaled height
  float baseHeight = m_cmap_data_weak->getTerrainHeightAt((int)fog_volume.m_center.y * m_cmap_data_weak->getWidth() + (int)fog_volume.m_center.x);
  std::cout << "Scaled terrain height at fog center: " << baseHeight << std::endl;
  
  // The altitude from fog data needs to be scaled like original Carnivores
  // In original code: YBegin * ctHScale where ctHScale = 64
  float fogAltitudeOffset = fog_volume.m_fog_data.altitude;
  
  // Debug the raw altitude value
  std::cout << "Raw fog altitude from data: " << fogAltitudeOffset << std::endl;
  
  // Apply Carnivores scaling factor (ctHScale: C2=64, C1=32)
  fogAltitudeOffset *= m_cmap_data_weak->getHeightmapScale();
  
  std::cout << "Fog altitude after ctHScale (" << m_cmap_data_weak->getHeightmapScale() << "): " << fogAltitudeOffset << std::endl;
  
  // Ensure reasonable altitude offset (scaled for new world scale)
  if (fogAltitudeOffset > 31.25f) { // Scaled down 16x (was 500.0f)
    fogAltitudeOffset = 12.5f; // Scaled down 16x (was 200.0f)
  }
  if (fogAltitudeOffset < -6.25f) { // Scaled down 16x (was -100.0f)
    fogAltitudeOffset = 3.125f; // Scaled down 16x (was 50.0f)
  }
  
  // Create fog like original Carnivores: infinite fog above YBegin level
  // In original: fog starts at YBegin*ctHScale and extends infinitely upward
  float fogStartHeight = baseHeight + fogAltitudeOffset; // Original YBegin*ctHScale equivalent
  
  // Apply height corrections based on original engine differences
  if (m_cmap_data_weak->m_type == CEMapType::C1) {
    // C1 maps need the -48 offset that appears throughout the original C1 code
    fogStartHeight += (-48.0f * m_cmap_data_weak->getHeightmapScale()); // -48 * ctHScale
  }
  // C2 uses no offset - fog starts exactly at YBegin*ctHScale as per original C2 source
  float fogThickness = (m_cmap_data_weak->m_type == CEMapType::C1) ? 25.0f : 12.5f; // C1 needs thicker fog due to height scale differences
  float fogTopHeight = fogStartHeight + fogThickness;
  float totalFogHeight = fogTopHeight - fogStartHeight;
  float layerThickness = totalFogHeight / num_layers; // Distribute layers from fog start upward
  
  std::cout << "Fog volume altitude: base=" << baseHeight << " rawOffset=" << fog_volume.m_fog_data.altitude 
            << " normalizedOffset=" << fogAltitudeOffset << " start=" << fogStartHeight 
            << " top=" << fogTopHeight << " totalHeight=" << totalFogHeight;
  if (m_cmap_data_weak->m_type == CEMapType::C1) {
    std::cout << " (C1 with -48*" << m_cmap_data_weak->getHeightmapScale() << " offset)";
  }
  std::cout << std::endl;
  
  // Determine fog type flag
  uint32_t fogTypeFlag = fog_volume.m_fog_data.danger ? 1 : 0;
  
  for (int layer = 0; layer < num_layers; layer++) {
    float layerHeight = fogStartHeight + (layer * layerThickness);
    float layerAlpha = 0.8f - (float(layer) / float(num_layers)) * 0.5f; // Start at 80% alpha, fade to 30%
    
    // Create quad for this layer
    glm::vec3 corners[4] = {
      glm::vec3(worldCenter.x - worldSize.x * 0.5f, layerHeight, worldCenter.y - worldSize.y * 0.5f), // LL
      glm::vec3(worldCenter.x + worldSize.x * 0.5f, layerHeight, worldCenter.y - worldSize.y * 0.5f), // LR
      glm::vec3(worldCenter.x - worldSize.x * 0.5f, layerHeight, worldCenter.y + worldSize.y * 0.5f), // UL
      glm::vec3(worldCenter.x + worldSize.x * 0.5f, layerHeight, worldCenter.y + worldSize.y * 0.5f)  // UR
    };
    
    // UV coordinates for texture sampling
    glm::vec2 uvs[4] = {
      glm::vec2(0.0f, 0.0f),
      glm::vec2(1.0f, 0.0f),
      glm::vec2(0.0f, 1.0f),
      glm::vec2(1.0f, 1.0f)
    };
    
    int base_vertex_index = fog_volume.m_vertices.size();
    
    // Add vertices
    for (int i = 0; i < 4; i++) {
      Vertex vertex(corners[i], uvs[i], glm::vec3(0.0, 1.0, 0.0), false, layerAlpha, fog_volume.m_texture_id, fogTypeFlag);
      fog_volume.m_vertices.push_back(vertex);
    }
    
    // Add indices for two triangles
    fog_volume.m_indices.push_back(base_vertex_index + 0); // LL
    fog_volume.m_indices.push_back(base_vertex_index + 1); // LR
    fog_volume.m_indices.push_back(base_vertex_index + 2); // UL
    
    fog_volume.m_indices.push_back(base_vertex_index + 1); // LR
    fog_volume.m_indices.push_back(base_vertex_index + 3); // UR
    fog_volume.m_indices.push_back(base_vertex_index + 2); // UL
  }
  
  // Add vertical planes around the edges for better side visibility
  float edgeAlpha = 0.4f; // Lower alpha for edge planes
  
  // Front vertical plane (facing negative Z)
  {
    int base_vertex_index = fog_volume.m_vertices.size();
    
    glm::vec3 corners[4] = {
      glm::vec3(worldCenter.x - worldSize.x * 0.5f, fogStartHeight, worldCenter.y - worldSize.y * 0.5f), // Bottom Left
      glm::vec3(worldCenter.x + worldSize.x * 0.5f, fogStartHeight, worldCenter.y - worldSize.y * 0.5f), // Bottom Right  
      glm::vec3(worldCenter.x - worldSize.x * 0.5f, fogTopHeight, worldCenter.y - worldSize.y * 0.5f),   // Top Left
      glm::vec3(worldCenter.x + worldSize.x * 0.5f, fogTopHeight, worldCenter.y - worldSize.y * 0.5f)    // Top Right
    };
    
    glm::vec2 uvs[4] = {
      glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f),
      glm::vec2(0.0f, 1.0f), glm::vec2(1.0f, 1.0f)
    };
    
    for (int i = 0; i < 4; i++) {
      Vertex vertex(corners[i], uvs[i], glm::vec3(0.0, 0.0, -1.0), false, edgeAlpha, fog_volume.m_texture_id, fogTypeFlag);
      fog_volume.m_vertices.push_back(vertex);
    }
    
    // Two triangles for the plane
    fog_volume.m_indices.push_back(base_vertex_index + 0);
    fog_volume.m_indices.push_back(base_vertex_index + 1);
    fog_volume.m_indices.push_back(base_vertex_index + 2);
    
    fog_volume.m_indices.push_back(base_vertex_index + 1);
    fog_volume.m_indices.push_back(base_vertex_index + 3);
    fog_volume.m_indices.push_back(base_vertex_index + 2);
  }
  
  // Back vertical plane (facing positive Z)
  {
    int base_vertex_index = fog_volume.m_vertices.size();
    
    glm::vec3 corners[4] = {
      glm::vec3(worldCenter.x + worldSize.x * 0.5f, fogStartHeight, worldCenter.y + worldSize.y * 0.5f), // Bottom Right
      glm::vec3(worldCenter.x - worldSize.x * 0.5f, fogStartHeight, worldCenter.y + worldSize.y * 0.5f), // Bottom Left  
      glm::vec3(worldCenter.x + worldSize.x * 0.5f, fogTopHeight, worldCenter.y + worldSize.y * 0.5f),   // Top Right
      glm::vec3(worldCenter.x - worldSize.x * 0.5f, fogTopHeight, worldCenter.y + worldSize.y * 0.5f)    // Top Left
    };
    
    glm::vec2 uvs[4] = {
      glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f),
      glm::vec2(0.0f, 1.0f), glm::vec2(1.0f, 1.0f)
    };
    
    for (int i = 0; i < 4; i++) {
      Vertex vertex(corners[i], uvs[i], glm::vec3(0.0, 0.0, 1.0), false, edgeAlpha, fog_volume.m_texture_id, fogTypeFlag);
      fog_volume.m_vertices.push_back(vertex);
    }
    
    fog_volume.m_indices.push_back(base_vertex_index + 0);
    fog_volume.m_indices.push_back(base_vertex_index + 1);
    fog_volume.m_indices.push_back(base_vertex_index + 2);
    
    fog_volume.m_indices.push_back(base_vertex_index + 1);
    fog_volume.m_indices.push_back(base_vertex_index + 3);
    fog_volume.m_indices.push_back(base_vertex_index + 2);
  }
  
  fog_volume.m_vertex_count = fog_volume.m_vertices.size();
  fog_volume.m_num_indices = fog_volume.m_indices.size();
}
