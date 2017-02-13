//
//  CEVertexBuffer.cpp
//  CE Character Lab
//
//  Created by Tyler Minard on 2/12/17.
//  Copyright © 2017 Tyler Minard. All rights reserved.
//

#include "CEVertexBuffer.h"
#include "vertex.h"

CEVertexBuffer::CEVertexBuffer(const Vertex* vertex_data, int vertex_data_size, const unsigned int* indice_data, int indice_data_size)
: m_uploaded_to_gpu(false)
{
  this->uploadToGPU(vertex_data, vertex_data_size, indice_data, indice_data_size);
}

// Allow preparing the buffer prior to loading the data
CEVertexBuffer::CEVertexBuffer()
: m_uploaded_to_gpu(false)
{

}

CEVertexBuffer::~CEVertexBuffer()
{
  glDeleteBuffers(NUM_BUFFERS, m_vertexArrayBuffers);
  glDeleteVertexArrays(1, &this->m_vertexArrayObject);
}

void CEVertexBuffer::Draw()
{
  glBindVertexArray(this->m_vertexArrayObject);

  glDrawElementsBaseVertex(GL_TRIANGLES, this->m_indice_data_size, GL_UNSIGNED_INT, 0, 0);

  glBindVertexArray(0);
}

void CEVertexBuffer::uploadToGPU(const Vertex* vertex_data, int vertex_data_size, const unsigned int* indice_data, int indice_data_size)
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

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_vertexArrayBuffers[INDEX_VB]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indice_data_size*sizeof(unsigned int), indice_data, GL_STATIC_DRAW);

  glBindVertexArray(0);

  m_indice_data_size = indice_data_size;
  m_uploaded_to_gpu = true;
}
