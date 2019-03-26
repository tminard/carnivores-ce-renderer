//
//  TerrainRenderer.cpp
//  CE Character Lab
//
//  Created by Tyler Minard on 8/22/15.
//  Copyright (c) 2015 Tyler Minard. All rights reserved.
//

#include "TerrainRenderer.h"
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
        object_height = map_height + (w_obj->getObjectInfo()->YLo / 2.f); // Copies funny offsetting in original source - key is to avoid z-fighting without exposing gaps
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
  
  this->m_shader->use();
  this->m_shader->setMat4("MVP", MVP);
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
  float max_tc = 0.990f;
  float tu, tv;
  float i = 1.f;
  tu = 0.003f; tv = 0.003f;
  
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
  int texture_y = int(floor(float(texture_id) / atlas_square_size));
  float tile_scale = (1.f / atlas_square_size); // maps 0-1 UV coords to 1/square portion of atlas
  
  return glm::vec2(
                   (texture_id * tile_scale) + (atlas_uv.x * tile_scale),
                   (texture_y * tile_scale) + (atlas_uv.y * tile_scale)
                   );
}

/*
 * add to water VBO at given location
 */
void TerrainRenderer::loadWaterAt(int x, int y)
{
  int width = this->m_cmap_data_weak->getWidth();
  int xy = (y*width) + x;

  int water_index = this->m_cmap_data_weak->getWaterAt(xy);
  uint16_t flags = this->m_cmap_data_weak->getFlagsAt(xy);

  CEWaterEntity water_data = this->m_crsc_data_weak->getWater(water_index);
  _Water* water_object = &this->m_waters[water_index];

  float wheight;
  if (water_object->m_height <= 0) {
    wheight = this->m_cmap_data_weak->getWaterHeightAt(xy);
  } else {
    wheight = water_object->m_height;
  }


  glm::vec3 vpositionUL = this->calcWorldVertex(x, y, true, wheight);
  glm::vec3 vpositionUR = this->calcWorldVertex(x + 1, y, true, wheight);
  glm::vec3 vpositionLL = this->calcWorldVertex(x, y + 1, true, wheight);
  glm::vec3 vpositionLR = this->calcWorldVertex(x + 1, y + 1, true, wheight);

  bool quad_reverse = (flags & 0x0010);
  int texture_direction = (flags & 3);

  std::array<glm::vec2, 4> vertex_uv_mapping = this->calcUVMapForQuad(x, y, quad_reverse, texture_direction);

  Vertex v1(vpositionUL, this->scaleAtlasUV(vertex_uv_mapping[0], water_data.texture_id), glm::vec3(0,0,0), false, 1.4f - water_data.transparency, water_data.texture_id, 0);
  Vertex v2(vpositionUR, this->scaleAtlasUV(vertex_uv_mapping[1], water_data.texture_id), glm::vec3(0,0,0), false, 1.4f - water_data.transparency, water_data.texture_id, 0);
  Vertex v3(vpositionLL, this->scaleAtlasUV(vertex_uv_mapping[2], water_data.texture_id), glm::vec3(0,0,0), false, 1.4f - water_data.transparency, water_data.texture_id, 0);
  Vertex v4(vpositionLR, this->scaleAtlasUV(vertex_uv_mapping[3], water_data.texture_id), glm::vec3(0,0,0), false, 1.4f - water_data.transparency, water_data.texture_id, 0);

  if (quad_reverse) {
    water_object->m_vertices.push_back(v1);
    water_object->m_vertices.push_back(v2);
    water_object->m_vertices.push_back(v3);

    water_object->m_vertices.push_back(v3);
    water_object->m_vertices.push_back(v2);
    water_object->m_vertices.push_back(v4);
  } else {
    water_object->m_vertices.push_back(v3);
    water_object->m_vertices.push_back(v2);
    water_object->m_vertices.push_back(v4);

    water_object->m_vertices.push_back(v1);
    water_object->m_vertices.push_back(v2);
    water_object->m_vertices.push_back(v3);
  }
}

void TerrainRenderer::loadWaterIntoMemory()
{
  for (int w = 0; w < this->m_crsc_data_weak->getWaterCount(); w++)
  {
    _Water* water_object = &this->m_waters[w];
    water_object->m_vertex_count = (int)water_object->m_vertices.size();

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
  
  // vertices and indices
  for (int y=0; y < width; y++) {
    for (int x=0; x < height; x++) {
      unsigned int base_index = (y * width) + x;
      
      int texID = this->m_cmap_data_weak->getTextureIDAt(base_index);
      uint16_t flags = this->m_cmap_data_weak->getFlagsAt(x, y);

      if (this->m_cmap_data_weak->hasWaterAt(base_index)) loadWaterAt(x, y);
      
      bool final_y_row = (y + 1 > width);
      bool final_x_column = (x + 1 > height);
      
      if (final_y_row || final_x_column) {
        continue;
      }
      
      glm::vec3 vpositionUL = this->calcWorldVertex(x, y, false, 0.f);
      glm::vec3 vpositionUR = this->calcWorldVertex(x + 1, y, false, 0.f);
      glm::vec3 vpositionLL = this->calcWorldVertex(x, y + 1, false, 0.f);
      glm::vec3 vpositionLR = this->calcWorldVertex(x + 1, y + 1, false, 0.f);
      
      bool quad_reverse = (flags & 0x0010);
      int texture_direction = (flags & 3);
      
      std::array<glm::vec2, 4> vertex_uv_mapping = this->calcUVMapForQuad(x, y, quad_reverse, texture_direction);
      
      Vertex v1(vpositionUL, this->scaleAtlasUV(vertex_uv_mapping[0], texID), glm::vec3(0,0,0), false, 1.f, texID, 0);
      Vertex v2(vpositionUR, this->scaleAtlasUV(vertex_uv_mapping[1], texID), glm::vec3(0,0,0), false, 1.f, texID, 0);
      Vertex v3(vpositionLL, this->scaleAtlasUV(vertex_uv_mapping[2], texID), glm::vec3(0,0,0), false, 1.f, texID, 0);
      Vertex v4(vpositionLR, this->scaleAtlasUV(vertex_uv_mapping[3], texID), glm::vec3(0,0,0), false, 1.f, texID, 0);
      
      m_vertices.push_back(v1);
      m_vertices.push_back(v2);
      m_vertices.push_back(v3);
      m_vertices.push_back(v4);

      unsigned int upper_left = (y * width * 4) + (x*4);
      unsigned int upper_right = upper_left + 1;
      unsigned int lower_left = upper_right + 1;
      unsigned int lower_right = lower_left + 1;
      
      if (quad_reverse) {
        m_indices.push_back(upper_left);
        m_indices.push_back(upper_right);
        m_indices.push_back(lower_left);
        
        m_indices.push_back(lower_left);
        m_indices.push_back(upper_right);
        m_indices.push_back(lower_right);
      } else {
        m_indices.push_back(lower_left);
        m_indices.push_back(upper_right);
        m_indices.push_back(lower_right);
        
        m_indices.push_back(upper_left);
        m_indices.push_back(upper_right);
        m_indices.push_back(lower_left);
      }
    }
  }
  
  m_num_indices = (int)m_indices.size();
  
  // generate buffers and upload
  glGenVertexArrays(1, &this->m_vertex_array_object);
  glBindVertexArray(this->m_vertex_array_object);
  
  glGenBuffers(1, &this->m_vertex_array_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, this->m_vertex_array_buffer);
  glBufferData(GL_ARRAY_BUFFER, (int)m_vertices.size()*sizeof(Vertex), m_vertices.data(), GL_STATIC_DRAW);
  
  // describe vertex details
  glEnableVertexAttribArray(0); // position
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
  glEnableVertexAttribArray(1); // uv
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(glm::vec3));
  glEnableVertexAttribArray(2); // normal
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec3)+sizeof(glm::vec2)));
  glEnableVertexAttribArray(3); // texid
  glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, sizeof(Vertex), (void*)(sizeof(glm::vec3)+sizeof(glm::vec2)+(sizeof(glm::vec3))));
  
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
  this->m_shader->use();

  for (int w = 0; w < this->m_waters.size(); w++) {
    glBindVertexArray(this->m_waters[w].m_vao);

    glDrawArrays(GL_TRIANGLES, 0, this->m_waters[w].m_vertex_count);
  }

  glBindVertexArray(0);
}
