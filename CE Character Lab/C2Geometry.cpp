/*
 //initialization
 glGenVertexArrays
 glBindVertexArray
 
 glGenBuffers
 glBindBuffer
 glBufferData
 
 glVertexAttribPointer
 glEnableVertexAttribArray
 
 glBindVertexArray(0)
 
 glDeleteBuffers //you can already delete it after the VAO is unbound, since the
 //VAO still references it, keeping it alive (see comments below).
 
 ...
 
 //rendering
 glBindVertexArray
 glDrawWhatever
 */

#include <iterator>

#include "C2Geometry.h"
#include "C2Texture.h"
#include "Vertex.h"

/*
 * The old C2 model format supported ignoring lighting.
 * We carry this over as a hint, which may be ignored
 * in the modern renderer.
 */
void C2Geometry::hint_ignoreLighting()
{
  // Yeah, ignore this for now
}

C2Geometry::C2Geometry(std::vector < Vertex > vertices, std::vector < uint32_t > indices, std::unique_ptr<C2Texture> texture)
: m_vertices(vertices), m_indices(indices), m_texture(std::move(texture))
{
  this->loadObjectIntoMemoryBuffer();
}

C2Geometry::~C2Geometry()
{
  glDeleteBuffers(NUM_BUFFERS, m_vertexArrayBuffers);
  glDeleteVertexArrays(1, &this->m_vertexArrayObject);
}


void C2Geometry::saveTextureAsBMP(const std::string &file_name)
{
  this->m_texture->saveToBMPFile(file_name);
}

void C2Geometry::loadObjectIntoMemoryBuffer()
{
  glGenVertexArrays(1, &this->m_vertexArrayObject);
  glBindVertexArray(this->m_vertexArrayObject);
  
  glGenBuffers(NUM_BUFFERS, this->m_vertexArrayBuffers);

  glBindBuffer(GL_ARRAY_BUFFER, this->m_vertexArrayBuffers[VERTEX_VB]);
  glBufferData(GL_ARRAY_BUFFER, (int)this->m_vertices.size()*sizeof(Vertex), this->m_vertices.data(), GL_DYNAMIC_DRAW);
  
  glEnableVertexAttribArray(0); // position
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
  glEnableVertexAttribArray(1); // uv
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(glm::vec3));
  glEnableVertexAttribArray(2); // normal
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec3)+sizeof(glm::vec2)));

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_vertexArrayBuffers[INDEX_VB]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, (int)this->m_indices.size()*sizeof(unsigned int), this->m_indices.data(), GL_STATIC_DRAW);

  glBindVertexArray(0);
}

void C2Geometry::Draw()
{
  glBindVertexArray(this->m_vertexArrayObject);
  //glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

  //glDrawArrays(GL_TRIANGLES, 0, (int)this->m_indices.size());
  glDrawElementsBaseVertex(GL_TRIANGLES, (int)this->m_indices.size(), GL_UNSIGNED_INT, 0, 0);
  //glDrawElements(GL_TRIANGLES, this->m_indices.size(), GL_UNSIGNED_INT, 0);
  
  glBindVertexArray(0);
}

void C2Geometry::exportAsOBJ(const std::string& file_name)
{
  
}

