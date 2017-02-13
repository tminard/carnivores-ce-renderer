//
//  CETerrainVertexBuffer.cpp
//  CE Character Lab
//
//  Created by Tyler Minard on 2/13/17.
//  Copyright © 2017 Tyler Minard. All rights reserved.
//

#include "CETerrainVertexBuffer.h"
#include "vertex.h"

void CETerrainVertexBuffer::uploadToGPU(const Vertex* vertex_data, int vertex_data_size, const unsigned int* indice_data, int indice_data_size)
{
  assert(!m_uploaded_to_gpu);

  glGenVertexArrays(1, &this->m_vertexArrayObject);
  glBindVertexArray(this->m_vertexArrayObject);

  glGenBuffers(NUM_BUFFERS, this->m_vertexArrayBuffers);

  glBindBuffer(GL_ARRAY_BUFFER, this->m_vertexArrayBuffers[VERTEX_VB]);
  glBufferData(GL_ARRAY_BUFFER, vertex_data_size*sizeof(Vertex), vertex_data, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0); // position
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
  glEnableVertexAttribArray(1); // uv
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(glm::vec3));
  glEnableVertexAttribArray(2); // normal
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec3)+sizeof(glm::vec2)));

  glEnableVertexAttribArray(3); // texid
  glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, sizeof(Vertex), (void*)(sizeof(glm::vec3)+sizeof(glm::vec2)+(sizeof(glm::vec3))));

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_vertexArrayBuffers[INDEX_VB]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indice_data_size*sizeof(unsigned int), indice_data, GL_STATIC_DRAW);

  glBindVertexArray(0);

  m_indice_data_size = indice_data_size;
  m_uploaded_to_gpu = true;
}
