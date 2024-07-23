#include <iterator>

#include "CEGeometry.h"
#include "CETexture.h"
#include "vertex.h"
#include "shader_program.h"

#include "camera.h"
#include "transform.h"

#include <nlohmann/json.hpp>
#include <filesystem>

namespace fs = std::filesystem;
using json = nlohmann::json;

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
  std::ifstream f("config.json");
  json data = json::parse(f);
  
  fs::path basePath = fs::path(data["basePath"].get<std::string>());
  fs::path shaderPath = basePath / "shaders";
  
  this->m_shader = std::unique_ptr<ShaderProgram>(new ShaderProgram((shaderPath / "basic_shader.vs").string(), (shaderPath / "basic_shader.fs").string()));

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

    // instanced vab
  this->m_num_instances = 0;
  std::vector<glm::mat4> instances;
  glGenBuffers(1, &this->m_instanced_vab);
  glBindBuffer(GL_ARRAY_BUFFER, this->m_instanced_vab);
  glBufferData(GL_ARRAY_BUFFER, m_num_instances*sizeof(glm::mat4), instances.data(), GL_STATIC_DRAW);

  glEnableVertexAttribArray(4);
  glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)0);
  glEnableVertexAttribArray(5);
  glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(sizeof(glm::vec4)));
  glEnableVertexAttribArray(6);
  glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(2 * sizeof(glm::vec4)));
  glEnableVertexAttribArray(7);
  glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(3 * sizeof(glm::vec4)));

  glVertexAttribDivisor(4, 1);
  glVertexAttribDivisor(5, 1);
  glVertexAttribDivisor(6, 1);
  glVertexAttribDivisor(7, 1);

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

void CEGeometry::Update(Camera &camera)
{
  this->m_shader->use();
  this->m_shader->setMat4("projection_view", camera.GetViewProjection());
  this->m_shader->setBool("enable_transparency", true);
}

void CEGeometry::DrawInstances()
{
  this->m_shader->use();
  this->m_texture->use();
  glBindVertexArray(this->m_vertexArrayObject);

  glDrawElementsInstancedBaseVertex(GL_TRIANGLES, (int)this->m_indices.size(), GL_UNSIGNED_INT, 0, this->m_num_instances, 0);

  glBindVertexArray(0);
}

void CEGeometry::UpdateInstances(std::vector<glm::mat4> transforms)
{
  this->m_num_instances = (int)transforms.size();
  glBindBuffer(GL_ARRAY_BUFFER, this->m_instanced_vab);
  glBufferData(GL_ARRAY_BUFFER, this->m_num_instances*sizeof(glm::mat4), transforms.data(), GL_STATIC_DRAW);
}
