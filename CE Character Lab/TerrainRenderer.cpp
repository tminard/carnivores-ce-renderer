//
//  TerrainRenderer.cpp
//  CE Character Lab
//
//  Created by Tyler Minard on 8/22/15.
//  Copyright (c) 2015 Tyler Minard. All rights reserved.
//

#include "TerrainRenderer.h"
#include "Vertex.h"

#include <glm/glm.hpp>
#include <cstdint>

TerrainRenderer::TerrainRenderer()
{
  // setup base vertices to store general terrain view
  
  // load into memory
  this->loadIntoHardwareMemory();
}

TerrainRenderer::~TerrainRenderer()
{
  
}

static float get_texture_y_offset(int tex_id, int num_textures, int texture_square_root)
{
  int tx_row = static_cast<int>(((float)tex_id/(float)(texture_square_root))); // base 0 index
  float tx_y_offset = ((float)tx_row)/(float)texture_square_root;
  
  return tx_y_offset;
}

static float get_texture_x_offset(int tex_id, int num_textures, int texture_square_root)
{
  int tx_col = tex_id%texture_square_root; // base 0 index
  float tx_x_offset = ((float)tx_col)/(float)texture_square_root;
  
  return tx_x_offset;
}

// From http://www.learnopengles.com/android-lesson-eight-an-introduction-to-index-buffer-objects-ibos/
void TerrainRenderer::loadIntoHardwareMemory()
{
  int width = WORLD_SIZE, height = WORLD_SIZE;
  float offset = WORLD_SIZE / 2.0f;
  int gridHeight = height - 1;
  int gridWidth = width - 1;
  
  // using http://stackoverflow.com/questions/10114577/a-method-for-indexing-triangles-from-a-loaded-heightmap
  // vertices
  for (int y=0; y < height; ++y) {
    for (int x=0; x < width; ++x) {
      // get the texture index to display in this tile
      // TODO: Move this to shader
      float tu, tv;
      bool y_even = (y%2 == 0);
      bool x_even = (x%2 == 0);
      int tex_id = 1;
      float tile_size = 2.f;
      
      // odd y: 0-0 1-0 0-0 1-0 (this causes odd stretching)
      // even y: 0-1 1-1 0-1 1-1
      
      // 0-0 1-0
      // 0-1 1-1
      
      // even y: (xev)0-1 (xod)1-1 0-1 1-1
      // odd y (xev)0-0 (xod)1-0 0-0 1-0
/*
      if (y_even) {
        if (x_even) {
          tu = 0.005f + get_texture_x_offset(tex_id, 1, 1);
          tv = (get_texture_y_offset(tex_id, 1, 1) + (1.f/1.f))- 0.005f;
        } else {
          tu = (get_texture_x_offset(tex_id, 1, 1) + (1.f/1.f))- 0.005f;
          tv = (get_texture_y_offset(tex_id, 1, 1) + (1.f/1.f))- 0.005f;
        }
      } else {
        if (x_even) {
          tu = 0.005f + get_texture_x_offset(tex_id, 1, 1);
          tv = 0.005f + get_texture_y_offset(tex_id, 1, 1);
        } else {
          tu = (get_texture_x_offset(tex_id, 1, 1) + (1.f/1.f))- 0.005f;
          tv = 0.005f + get_texture_y_offset(tex_id, 1, 1);
        }
      }*/

      //($value - $min) / ($max - $min);
      float max_x = get_texture_x_offset(tex_id, 58, 8) + (1.f/8.f);
      float min_x = get_texture_x_offset(tex_id, 58, 8);
      float max_y = get_texture_y_offset(tex_id, 58, 8) + (1.f/8.f);
      float min_y = get_texture_y_offset(tex_id, 58, 8);
      // (get loc within tile) * scale it
      tu = (((float)x/tile_size) * (1.f/8.f)) + (1.f/8.f);//get_texture_x_offset(tex_id, 58, 8);
      tv = (((float)y/tile_size) * (1.f/8.f)) + 0;//get_texture_y_offset(tex_id, 58, 8);

      Vertex v(glm::vec3((x*tile_size)+tile_size-offset,0,(y*tile_size)+tile_size-offset), glm::vec2(tu,tv), glm::vec3(0,0,0), false, 0);
      m_vertices.push_back(v);
    }
  }
  
  // indices
  for (int y=0; y < gridHeight; ++y) {
    for (int x = 0; x < gridWidth; ++x) {
      unsigned int start = y * width + x;
      m_indices.push_back(start);
      m_indices.push_back(start+1);
      m_indices.push_back(start+width);
      m_indices.push_back(start+1);
      m_indices.push_back(start+1+width);
      m_indices.push_back(start+width);
    }
  }

  m_num_indices = (int)m_indices.size();
  
  // generate buffers and upload
  glGenVertexArrays(1, &this->m_vertexArrayObject);
  glBindVertexArray(this->m_vertexArrayObject);
  
  glGenBuffers(1, &this->m_vertexArrayBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, this->m_vertexArrayBuffer);
  glBufferData(GL_ARRAY_BUFFER, (int)m_vertices.size()*sizeof(Vertex), m_vertices.data(), GL_STATIC_DRAW);
  
  // describe vertex details
  glEnableVertexAttribArray(0); // position
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
  glEnableVertexAttribArray(1); // uv
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(glm::vec3));
  glEnableVertexAttribArray(2); // normal
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec3)+sizeof(glm::vec2)));
  
  // indices
  glGenBuffers(1, &this->m_indicesArrayBuffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_indicesArrayBuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, (int)m_indices.size()*sizeof(unsigned int), m_indices.data(), GL_STATIC_DRAW);
  
  glBindVertexArray(0);
}

void TerrainRenderer::Render()
{
  glBindVertexArray(this->m_vertexArrayObject);

  glDrawElementsBaseVertex(GL_TRIANGLE_STRIP, m_num_indices, GL_UNSIGNED_INT, 0, 0);
  
  glBindVertexArray(0);
}

void TerrainRenderer::UpdateForPos()
{
  // update the terrian model based on current view
}
