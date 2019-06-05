#include "TerrainRenderer.h"
#include "CETerrainVertex.hpp"
#include "vertex.h"

#include "C2MapFile.h"
#include "C2MapRscFile.h"
#include "CEWorldModel.h"
#include "CESimpleGeometry.h"
#include "CEGeometry.h"
#include "shader_program.h"
#include "camera.h"
#include "transform.h"

#include "CEWaterEntity.h"

#include <cstdint>

TerrainRenderer::TerrainRenderer(C2MapFile* c_map_weak, C2MapRscFile* c_rsc_weak)
: m_cmap_data_weak(c_map_weak), m_crsc_data_weak(c_rsc_weak)
{
  this->loadIntoHardwareMemory();
  this->loadShader();
  this->preloadObjectMap();
}

TerrainRenderer::~TerrainRenderer()
{
  glDeleteBuffers(1, &this->m_vertex_array_buffer);
  glDeleteBuffers(1, &this->m_indices_array_buffer);
  glDeleteVertexArrays(1, &this->m_vertex_array_object);

  for (int w = 0; w < this->m_waters.size(); w++) {
    glDeleteBuffers(1, &this->m_waters[w].m_vab);
    glDeleteBuffers(1, &this->m_waters[w].m_iab);
    glDeleteVertexArrays(1, &this->m_waters[w].m_vao);
  }
}

/*
 * precalculate transforms for all map objects
 */
void TerrainRenderer::preloadObjectMap()
{
  int map_square_size = this->m_cmap_data_weak->getHeight();
  float map_tile_length = this->m_cmap_data_weak->getTileLength();
  
  for (int y = 0; y < map_square_size; y++) {
    for (int x = 0; x < map_square_size; x++) {
      int xy = (y*map_square_size)+x;
      int obj_id = this->m_cmap_data_weak->getObjectAt(xy);
      
      if (obj_id == 255 || obj_id == 254) continue;

      float object_height;
      CEWorldModel* w_obj = this->m_crsc_data_weak->getWorldModel(obj_id);
      
      if (w_obj->getObjectInfo()->flags & objectPLACEGROUND) {
        // TODO: original implementation gets the lowest height of a quad and uses that.
        float map_height = this->m_cmap_data_weak->getHeightAt(xy);
        object_height = map_height - (w_obj->getObjectInfo()->YLo / 2.f); // Copies funny offsetting in original source - key is to avoid z-fighting without exposing gaps
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
      
      Transform transform_initial(
                                  world_position,
                                  rotation,
                                  glm::vec3(1.f, 1.f, 1.f)
                                  );

      w_obj->addFar(transform_initial);
      w_obj->addNear(transform_initial);
    }
  }

  for (int m = 0; m < this->m_crsc_data_weak->getWorldModelCount(); m++) {
    this->m_crsc_data_weak->getWorldModel(m)->updateNearInstances();
  }
}

void TerrainRenderer::loadShader()
{
  this->m_shader = std::unique_ptr<ShaderProgram>(new ShaderProgram("resources/shaders/terrain.vs", "resources/shaders/terrain.fs"));
  this->m_water_shader = std::unique_ptr<ShaderProgram>(new ShaderProgram("resources/shaders/water_surface.vs", "resources/shaders/water_surface.fs"));

  this->m_shader->use();
  this->m_shader->setFloat("view_distance", (128.f*256.f));
}

//      /*
//       * WTF Guide:
//       * even y: (xev)0-1 (xod)1-1 0-1 1-1
//       * odd y   (xev)0-0 (xod)1-0 0-0 1-0
//       *
//       * We basically need to map the uv coords properly based on the position of the
//       * tile.
//       *
//       */

void TerrainRenderer::Update(Transform& transform, Camera& camera)
{
  glm::mat4 MVP = transform.GetMVP(camera);
  double t = glfwGetTime();

  this->m_shader->use();
  this->m_shader->setMat4("MVP", MVP);
  this->m_water_shader->use();
  this->m_water_shader->setMat4("MVP", MVP);
  this->m_water_shader->setFloat("RealTime", (float)t);

  m_last_update_time = t;
}

void TerrainRenderer::RenderObjects(Camera& camera)
{
  for (int m = 0; m < this->m_crsc_data_weak->getWorldModelCount(); m++) {
    this->m_crsc_data_weak->getWorldModel(m)->getGeometry()->Update(camera);
    this->m_crsc_data_weak->getWorldModel(m)->getGeometry()->DrawInstances();
  }
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

  // This is called "half pixel correction" - a niave implementation. See https://gamedev.stackexchange.com/questions/46963/how-to-avoid-texture-bleeding-in-a-texture-atlas for something better. Note this doesn't solve mipmaps issue
  // For solution, see answer here: https://gamedev.stackexchange.com/questions/46963/how-to-avoid-texture-bleeding-in-a-texture-atlas
  float max_tc = 1.f;
  float tu, tv;
  float i = 1.f;
  tu = 0.f; tv = 0.f;
  
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

  // TODO: alpha should be dependent on the angle of viewer, and thus handled in a shader. This calculation is based on a precalculated 90 degree view
float TerrainRenderer::calcWaterAlpha(int tile_x, int tile_y, float water_height_scaled)
{
  int width = this->m_cmap_data_weak->getWidth();
  int xy = (tile_y*width) + tile_x;
  float h_delta = water_height_scaled - this->m_cmap_data_weak->getHeightAt(xy);
  float max_delta = this->m_cmap_data_weak->getTileLength() * 0.25f;

  h_delta = fminf(h_delta, max_delta);
  float trans = 0.99f * (h_delta / max_delta); // 0 = close, max .9

  return trans;
}

/*
 * add to water VBO at given location
 */
void TerrainRenderer::loadWaterAt(int x, int y)
{
  int width = this->m_cmap_data_weak->getWidth();
  int height = this->m_cmap_data_weak->getHeight();
  int xy = (y*width) + x;

  int water_index = this->m_cmap_data_weak->getWaterAt(xy);

  CEWaterEntity water_data = this->m_crsc_data_weak->getWater(water_index);
  if (water_index > this->m_waters.size()) {
    printf("Attempted to access water_index `%i` at x: %i, y: %i, which is out of bounds\n", water_index, x, y);
    water_index = 0;
  }
  _Water* water_object = &this->m_waters[water_index];

  float wheight;
  if (water_object->m_height <= 0) {
    // Magic value used for C1 calculation - C1 will always use this method
    wheight = this->m_cmap_data_weak->getWaterHeightAt(x, y);
  } else {
    // C2/Ice age will always use pre-set heights
    wheight = water_object->m_height;
  }

  glm::vec3 vpositionLL = this->calcWorldVertex(x, y, true, wheight);
  glm::vec3 vpositionLR = this->calcWorldVertex(fmin(x + 1, height - 1), y, true, wheight);
  glm::vec3 vpositionUL = this->calcWorldVertex(x, fmin(y + 1, width - 1), true, wheight);
  glm::vec3 vpositionUR = this->calcWorldVertex(fmin(x + 1, height - 1), fmin(y + 1, width - 1), true, wheight);

  bool quad_reverse = this->m_cmap_data_weak->isQuadRotatedAt(xy);
  int texture_id = this->m_cmap_data_weak->getWaterTextureIDAt(xy, water_data.texture_id);

  std::array<glm::vec2, 4> vertex_uv_mapping = this->calcUVMapForQuad(x, y, quad_reverse, 0);

  Vertex v1(vpositionLL, this->scaleAtlasUV(vertex_uv_mapping[0], texture_id), glm::vec3(0,0,0), false, calcWaterAlpha(x, y, wheight), texture_id, 0);
  Vertex v2(vpositionLR, this->scaleAtlasUV(vertex_uv_mapping[1], texture_id), glm::vec3(0,0,0), false, calcWaterAlpha(fmin(x+1, height-1), y, wheight), texture_id, 0);
  Vertex v3(vpositionUL, this->scaleAtlasUV(vertex_uv_mapping[2], texture_id), glm::vec3(0,0,0), false, calcWaterAlpha(x, fmin(y+1, width-1), wheight), texture_id, 0);
  Vertex v4(vpositionUR, this->scaleAtlasUV(vertex_uv_mapping[3], texture_id), glm::vec3(0,0,0), false, calcWaterAlpha(fmin(x+1, height-1), fmin(y+1, width-1), wheight), texture_id, 0);

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
}

void TerrainRenderer::loadWaterIntoMemory()
{
  for (int w = 0; w < this->m_crsc_data_weak->getWaterCount(); w++)
  {
    _Water* water_object = &this->m_waters[w];
    water_object->m_vertex_count = (int)water_object->m_vertices.size();
    water_object->m_num_indices = (int)water_object->m_indices.size();

    glBindVertexArray(water_object->m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, water_object->m_vab);
    glBufferData(GL_ARRAY_BUFFER, (int)water_object->m_vertices.size()*sizeof(Vertex), water_object->m_vertices.data(), GL_STATIC_DRAW);

      // describe vertex details
    glEnableVertexAttribArray(0); // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glEnableVertexAttribArray(1); // uv
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(glm::vec3));
    glEnableVertexAttribArray(2); // normal
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec3)+sizeof(glm::vec2)));
    glEnableVertexAttribArray(3); // alpha
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec3)+sizeof(glm::vec2)+(sizeof(glm::vec3))));

    glGenBuffers(1, &water_object->m_iab);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, water_object->m_iab);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (int)water_object->m_indices.size()*sizeof(unsigned int), water_object->m_indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
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

    glGenVertexArrays(1, &wd.m_vao);
    glBindVertexArray(wd.m_vao);
    
    glGenBuffers(1, &wd.m_vab);

    glBindVertexArray(0);

    this->m_waters.push_back(wd);
  }

  for (int y=0; y < width; y++) {
    for (int x=0; x < height; x++) {
      unsigned int base_index = (y * width) + x;
      
      int texID = this->m_cmap_data_weak->getTextureIDAt(base_index);
      int texID2 = this->m_cmap_data_weak->getSecondaryTextureIDAt(base_index);
      uint16_t flags = this->m_cmap_data_weak->getFlagsAt(x, y);

      if (this->m_cmap_data_weak->hasWaterAt(base_index)) loadWaterAt(x, y);

      glm::vec3 vpositionLL = this->calcWorldVertex(x, y, false, 0.f);
      glm::vec3 vpositionLR = this->calcWorldVertex(fmin(x + 1, height - 1), y, false, 0.f);
      glm::vec3 vpositionUL = this->calcWorldVertex(x, fmin(y + 1, width - 1), false, 0.f);
      glm::vec3 vpositionUR = this->calcWorldVertex(fmin(x + 1, height - 1), fmin(y + 1, width - 1), false, 0.f);
      
      bool quad_reverse = this->m_cmap_data_weak->isQuadRotatedAt(base_index);
      int texture_direction = (flags & 3);
      
      std::array<glm::vec2, 4> vertex_uv_mapping = this->calcUVMapForQuad(x, y, quad_reverse, texture_direction);

      CETerrainVertex v1(vpositionLL, this->getScaledAtlasUVQuad(vertex_uv_mapping[0], texID, texID2), glm::vec3(0), this->m_cmap_data_weak->getBrightnessAt(x, y));
      CETerrainVertex v2(vpositionLR, this->getScaledAtlasUVQuad(vertex_uv_mapping[1], texID, texID2), glm::vec3(0), this->m_cmap_data_weak->getBrightnessAt(fmin(x + 1, height - 1), y));
      CETerrainVertex v3(vpositionUL, this->getScaledAtlasUVQuad(vertex_uv_mapping[2], texID, texID2), glm::vec3(0), this->m_cmap_data_weak->getBrightnessAt(x, fmin(y + 1, width - 1)));
      CETerrainVertex v4(vpositionUR, this->getScaledAtlasUVQuad(vertex_uv_mapping[3], texID, texID2), glm::vec3(0), this->m_cmap_data_weak->getBrightnessAt(fmin(x + 1, height - 1), fmin(y + 1, width - 1)));
      
      m_vertices.push_back(v1);
      m_vertices.push_back(v2);
      m_vertices.push_back(v3);
      m_vertices.push_back(v4);

      unsigned int lower_left = (y * width * 4) + (x*4);
      unsigned int lower_right = lower_left + 1;
      unsigned int upper_left = lower_right + 1;
      unsigned int upper_right = upper_left + 1;

      // If quad reverse, anchor upper right (bottom left, upper right, lower right). Otherwise, anchor upper left (bottom left, upper left, lower right)
      if (quad_reverse) {
        m_indices.push_back(lower_left);
        m_indices.push_back(upper_left);
        m_indices.push_back(lower_right);

        m_indices.push_back(lower_right);
        m_indices.push_back(upper_left);
        m_indices.push_back(upper_right);
      } else {
        m_indices.push_back(lower_left);
        m_indices.push_back(upper_right);
        m_indices.push_back(lower_right);

        m_indices.push_back(lower_left);
        m_indices.push_back(upper_left);
        m_indices.push_back(upper_right);
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

  glEnableVertexAttribArray(2); // txture coords
  glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(CETerrainVertex), (void*)(sizeof(glm::vec3)+sizeof(glm::vec3)));

  glEnableVertexAttribArray(3); // brightness
  glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(CETerrainVertex), (void*)(sizeof(glm::vec3)+sizeof(glm::vec3)+(sizeof(glm::vec4))));
  
  // indices
  glGenBuffers(1, &this->m_indices_array_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_indices_array_buffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, (int)m_indices.size()*sizeof(unsigned int), m_indices.data(), GL_STATIC_DRAW);
  
  glBindVertexArray(0);

  this->loadWaterIntoMemory();
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
  
  glBindVertexArray(this->m_vertex_array_object);
  
  glDrawElementsBaseVertex(GL_TRIANGLES, m_num_indices, GL_UNSIGNED_INT, 0, 0);
  
  glBindVertexArray(0);
}

void TerrainRenderer::RenderWater()
{
  this->m_water_shader->use();

  for (int w = 0; w < this->m_waters.size(); w++) {
    glBindVertexArray(this->m_waters[w].m_vao);

    glDrawElementsBaseVertex(GL_TRIANGLES, this->m_waters[w].m_num_indices, GL_UNSIGNED_INT, 0, 0);
  }

  glBindVertexArray(0);
}
