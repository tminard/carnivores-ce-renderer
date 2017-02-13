//
//  TerrainRenderer.cpp
//  CE Character Lab
//
//  Created by Tyler Minard on 8/22/15.
//  Copyright (c) 2015 Tyler Minard. All rights reserved.
//

#include "TerrainRenderer.h"
#include "Vertex.h"

#include "C2MapFile.h"
#include "C2MapRscFile.h"

#include "CEGeometry.h"
#include "CETerrain.h"
#include "CETerrainVertexBuffer.h"

#include <cstdint>

TerrainRenderer::TerrainRenderer(C2MapFile* c_map_weak, C2MapRscFile* c_rsc_weak)
: m_cmap_data_weak(c_map_weak), m_crsc_data_weak(c_rsc_weak)
{
  this->loadIntoHardwareMemory();
}

TerrainRenderer::~TerrainRenderer()
{
  
}

glm::vec3 TerrainRenderer::calcWorldVertex(int tile_x, int tile_y)
{
  int width = this->m_cmap_data_weak->getWidth(), height = this->m_cmap_data_weak->getHeight();
  float tile_size = this->m_cmap_data_weak->getTileLength();

  // TODO: Define exceptions
  if (tile_x > width || tile_y > height) {
    throw 1;
  }

  float tile_height = this->m_cmap_data_weak->getHeightAt((tile_y * width) + tile_x);
  float vx = (tile_x*tile_size) + tile_size;
  float vy = (tile_y*tile_size) + tile_size;

  return glm::vec3(vx, tile_height, vy);
}

std::array<glm::vec2, 4> TerrainRenderer::calcUVMapForQuad(int x, int y, bool quad_reversed, int rotation_code)
{
  std::array<glm::vec2, 4> vertex_uv_mapping;
  x = 0; y = 0;

  /*
   00: no rotation (right-side-up)
   01: rotated 90 degrees clockwise
   10: rotated 180 degrees (upside-down)
   11: rotated 90 degrees counter-clockwise
   */

  if (!quad_reversed) {
    switch (rotation_code) {
      case 0:
        vertex_uv_mapping = {
          glm::vec2(x, y),
          glm::vec2(x + 1, y),
          glm::vec2(x, y + 1),
          glm::vec2(x + 1, y + 1)
        };
        break;
      case 1:
        vertex_uv_mapping = {
          glm::vec2(x, y + 1),
          glm::vec2(x, y),
          glm::vec2(x + 1, y + 1),
          glm::vec2(x + 1, y)
        };
        break;
      case 2:
        vertex_uv_mapping = {
          glm::vec2(x + 1, y + 1),
          glm::vec2(x, y + 1),
          glm::vec2(x + 1, y),
          glm::vec2(x, y)
        };
        break;
      case 3:
        vertex_uv_mapping = {
          glm::vec2(x + 1, y),
          glm::vec2(x + 1, y + 1),
          glm::vec2(x, y),
          glm::vec2(x, y + 1)
        };
        break;

      default:
        break;
    }
  } else {  // UL, UR, LL, LR
    switch (rotation_code) {
      case 0:
        vertex_uv_mapping = {
          glm::vec2(x, y),
          glm::vec2(x + 1, y),
          glm::vec2(x, y + 1),
          glm::vec2(x + 1, y + 1)
        };
        break;
      case 1:
        vertex_uv_mapping = {
          glm::vec2(x, y + 1),
          glm::vec2(x, y),
          glm::vec2(x + 1, y + 1),
          glm::vec2(x + 1, y)
        };
        break;
      case 2:
        vertex_uv_mapping = {
          glm::vec2(x + 1, y + 1),
          glm::vec2(x, y + 1),
          glm::vec2(x + 1, y),
          glm::vec2(x, y)
        };
        break;
      case 3:
        vertex_uv_mapping = {
          glm::vec2(x + 1, y),
          glm::vec2(x + 1, y + 1),
          glm::vec2(x, y),
          glm::vec2(x, y + 1)
        };
        break;

      default:
        break;
    }
  }

  return vertex_uv_mapping;
}


// From http://www.learnopengles.com/android-lesson-eight-an-introduction-to-index-buffer-objects-ibos/
// using http://stackoverflow.com/questions/10114577/a-method-for-indexing-triangles-from-a-loaded-heightmap
void TerrainRenderer::loadIntoHardwareMemory()
{
  int width = this->m_cmap_data_weak->getWidth(), height = this->m_cmap_data_weak->getHeight();

  // vertices and indices
  for (int y=0; y < height; y++) {
    for (int x=0; x < width; x++) {
      unsigned int base_index = (y * width) + x;

      int texID = this->m_cmap_data_weak->getTextureIDAt(base_index);
      uint16_t flags = this->m_cmap_data_weak->getFlagsAt(base_index);

      bool final_y_row = (y + 1 > height);
      bool final_x_column = (x + 1 > width);

      if (final_y_row || final_x_column) {
        continue;
      }

      glm::vec3 vpositionUL = this->calcWorldVertex(x, y); // upper left
      glm::vec3 vpositionUR = this->calcWorldVertex(x + 1, y); // upper right
      glm::vec3 vpositionLL = this->calcWorldVertex(x, y + 1); // lower left
      glm::vec3 vpositionLR = this->calcWorldVertex(x + 1, y + 1); // lower right

      bool quad_reverse = (flags & 0x0010);
      int texture_direction = (flags & 3);

      std::array<glm::vec2, 4> vertex_uv_mapping = this->calcUVMapForQuad(x, y, quad_reverse, texture_direction);

      Vertex v1(vpositionUL, vertex_uv_mapping[0], glm::vec3(0,0,0), false, texID);
      Vertex v2(vpositionUR, vertex_uv_mapping[1], glm::vec3(0,0,0), false, texID);
      Vertex v3(vpositionLL, vertex_uv_mapping[2], glm::vec3(0,0,0), false, texID);
      Vertex v4(vpositionLR, vertex_uv_mapping[3], glm::vec3(0,0,0), false, texID);

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

  std::unique_ptr<CETerrainVertexBuffer> terrBuffer = std::make_unique<CETerrainVertexBuffer>();
  std::unique_ptr<CEGeometry> terrain_geo = std::make_unique<CEGeometry>(m_vertices, m_indices, this->m_crsc_data_weak->getTextureForUse(0), std::move(terrBuffer));

  this->m_terrain = std::unique_ptr<CETerrain>(new CETerrain(std::move(terrain_geo)));
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
  this->m_terrain->Draw();
}

void TerrainRenderer::UpdateForPos()
{
}
