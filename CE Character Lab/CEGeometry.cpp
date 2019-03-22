#include <iterator>

#include "CEGeometry.h"
#include "CETexture.h"
#include "vertex.h"
#include "new_shader.h"

#include "camera.h"
#include "transform.h"

/*
 * The old C2 model format supported ignoring lighting.
 * We carry this over as a hint, which may be ignored
 * in the modern renderer.
 */
void CEGeometry::DEP_hint_ignoreLighting()
{
  // Yeah, ignore this for now
}

CEGeometry::CEGeometry(std::vector < Vertex > vertices, std::vector < uint32_t > indices, std::unique_ptr<CETexture> texture)
: m_vertices(vertices), m_indices(indices), m_texture(std::move(texture))
{
  this->loadObjectIntoMemoryBuffer();
}

CEGeometry::~CEGeometry()
{
  glDeleteBuffers(NUM_BUFFERS, m_vertexArrayBuffers);
  glDeleteVertexArrays(1, &this->m_vertexArrayObject);
}

CETexture* CEGeometry::getTexture()
{
  return this->m_texture.get();
}

void CEGeometry::saveTextureAsBMP(const std::string &file_name)
{
  this->m_texture->saveToBMPFile(file_name);
}

void CEGeometry::loadObjectIntoMemoryBuffer()
{
  this->m_shader = std::unique_ptr<NewShader>(new NewShader("resources/basicShader.vs", "resources/basicShader.fs"));

  glGenVertexArrays(1, &this->m_vertexArrayObject);
  glBindVertexArray(this->m_vertexArrayObject);
  
  glGenBuffers(NUM_BUFFERS, this->m_vertexArrayBuffers);

  glBindBuffer(GL_ARRAY_BUFFER, this->m_vertexArrayBuffers[VERTEX_VB]);
  glBufferData(GL_ARRAY_BUFFER, (int)this->m_vertices.size()*sizeof(Vertex), this->m_vertices.data(), GL_STATIC_DRAW);
  
  glEnableVertexAttribArray(0); // position
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
  glEnableVertexAttribArray(1); // uv
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(glm::vec3));
  glEnableVertexAttribArray(2); // normal
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec3)+sizeof(glm::vec2)));
  glEnableVertexAttribArray(3); // face alpha (if 0, then face has transparency. Otherwise, it does not)
  glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec3)+sizeof(glm::vec2)+sizeof(glm::vec3)));

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_vertexArrayBuffers[INDEX_VB]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, (int)this->m_indices.size()*sizeof(unsigned int), this->m_indices.data(), GL_STATIC_DRAW);

  glBindVertexArray(0);
}

void CEGeometry::Draw()
{
  m_shader->use();
  m_texture->use();
  glBindVertexArray(this->m_vertexArrayObject);

  glDrawElementsBaseVertex(GL_TRIANGLES, (int)this->m_indices.size(), GL_UNSIGNED_INT, 0, 0);

  glBindVertexArray(0);
}

void CEGeometry::Update(Transform &transform, Camera &camera)
{
  this->m_shader->use();
  glm::mat4 MVP = transform.GetStaticModelVP(camera);

  this->m_shader->setMat4("MVP", MVP);
  this->m_shader->setBool("enable_transparency", true);
}
