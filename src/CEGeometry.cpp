#include <iterator>

#include "CEGeometry.h"
#include "CETexture.h"
#include "C2MapFile.h"
#include "C2MapRscFile.h"
#include "vertex.h"
#include "shader_program.h"
#include "CEAnimation.h"

#include "camera.h"
#include "transform.h"

#include <nlohmann/json.hpp>
#include <filesystem>

#include "IndexedMeshLoader.h"

namespace fs = std::filesystem;
using json = nlohmann::json;

CEGeometry::CEGeometry(std::vector < Vertex > vertices, std::vector < uint32_t > indices, std::shared_ptr<CETexture> texture, std::string shaderName)
: m_vertices(vertices), m_indices(indices), m_texture(texture)
{
  this->loadObjectIntoMemoryBuffer(shaderName);
  m_current_frame = 0;
}

CEGeometry::~CEGeometry()
{
  glDeleteBuffers(NUM_BUFFERS, m_vertexArrayBuffers);
  glDeleteVertexArrays(1, &this->m_vertexArrayObject);
}

std::weak_ptr<CETexture> CEGeometry::getTexture()
{
  return this->m_texture;
}

void CEGeometry::saveTextureAsBMP(const std::string &file_name)
{
  this->m_texture->saveToBMPFile(file_name);
}

void CEGeometry::loadObjectIntoMemoryBuffer(std::string shaderName)
{
  std::ifstream f("config.json");
  json data = json::parse(f);
  
  fs::path basePath = fs::path(data["basePath"].get<std::string>());
  fs::path shaderPath = basePath / "shaders";
  
  this->m_shader = std::unique_ptr<ShaderProgram>(new ShaderProgram((shaderPath / (shaderName + ".vs")).string(), (shaderPath / (shaderName + ".fs")).string()));
  this->m_shader->use();
  this->m_shader->setBool("enable_transparency", true);
  
  glGenVertexArrays(1, &this->m_vertexArrayObject);
  glBindVertexArray(this->m_vertexArrayObject);
  
  glGenBuffers(NUM_BUFFERS, this->m_vertexArrayBuffers);
  
  glBindBuffer(GL_ARRAY_BUFFER, this->m_vertexArrayBuffers[VERTEX_VB]);
  glBufferData(GL_ARRAY_BUFFER, (int)this->m_vertices.size()*sizeof(Vertex), this->m_vertices.data(), GL_STREAM_DRAW);
  
  glEnableVertexAttribArray(0); // position
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
  glEnableVertexAttribArray(1); // uv
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(glm::vec3));
  glEnableVertexAttribArray(2); // normal
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec3)+sizeof(glm::vec2)));
  glEnableVertexAttribArray(3); // face alpha (if 0, then face has transparency. Otherwise, it does not)
  glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec3)+sizeof(glm::vec2)+sizeof(glm::vec3)));
  
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_vertexArrayBuffers[INDEX_VB]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, (int)this->m_indices.size()*sizeof(unsigned int), this->m_indices.data(), GL_DYNAMIC_DRAW);
  
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

bool CEGeometry::SetAnimation(std::weak_ptr<CEAnimation> animation, double atTime, double startAt, double lastUpdateAt, bool deferUpdate, bool maxFPS, bool notVisible, float playbackSpeed, bool loop) {
  std::shared_ptr<CEAnimation> ani = animation.lock();
  if (!ani) {
    // Handle the case where the animation is no longer available or doesn't exist
    return false;
  }
  
  const double minInterval = 1.0 / 30.0;
  if (atTime - lastUpdateAt < minInterval) {
    return false;
  }
  
  double animationStartTime = startAt;
  int totalFrames = ani->m_number_of_frames;
  
  // Calculate the elapsed time since the animation started
  double elapsedTime = atTime - animationStartTime;
  double lastUpdateDelta = atTime - lastUpdateAt;
  
  // Calculate the time per frame based on KPS (Keyframes Per Second)
  double timePerFrame = 1.0 / double(ani->m_kps);
  
  // Optimization: do not update if time delta is < kps unless the player is very close
  // Also, run at minimum FPS unless very close
  double maxUpdateThreshold = maxFPS ? timePerFrame / 16.0 : timePerFrame;
  if (lastUpdateDelta < maxUpdateThreshold) {
    return false;
  }
  
  // Optimization: no need to run through animations if dino is very far or we're not visible
  if (deferUpdate && (notVisible || lastUpdateDelta < timePerFrame * 4.0)) {
    return false;
  }
  
  // Calculate the total time for the animation cycle
  double totalTime = totalFrames * timePerFrame;
  
  if (!loop && (elapsedTime * 1000.0) >= ani->m_total_time) {
    return false;
  }

  // Wrap the elapsed time around the total animation time
  double currentTime = fmod(elapsedTime, totalTime);
  
  // Determine the frame index and interpolation factor
  double exactFrameIndex = currentTime / timePerFrame;
  int currentFrame = static_cast<int>(exactFrameIndex) % totalFrames;
  int nextFrame = (currentFrame + 1) % totalFrames;
  
  m_current_frame = currentFrame;
  
  float k2 = static_cast<float>(exactFrameIndex - currentFrame); // Interpolation factor
  float k1 = 1.0f - k2;
  
  auto aniData = *ani->GetAnimationData();
  auto origVData = ani->GetOriginalVertices();
  assert(aniData.size() % 3 == 0);
  size_t numVertices = origVData->size();
  
  // We need to copy original vertices to updatedVertices for modification
  std::vector<TPoint3d> updatedVertices;
  updatedVertices.resize(numVertices);
  
  int aniOffset = currentFrame * (int)numVertices * 3;
  int nextFrameOffset = nextFrame * (int)numVertices * 3;
  
  for (size_t v = 0; v < numVertices; v++) {
    updatedVertices[v].x = (aniData[aniOffset + (v * 3 + 0)] * k1 + aniData[nextFrameOffset + (v * 3 + 0)] * k2) / 8.f;
    updatedVertices[v].y = (aniData[aniOffset + (v * 3 + 1)] * k1 + aniData[nextFrameOffset + (v * 3 + 1)] * k2) / 8.f;
    updatedVertices[v].z = ((aniData[aniOffset + (v * 3 + 2)] * k1 + aniData[nextFrameOffset + (v * 3 + 2)] * k2)) / 8.f;
  }
  
  // Recompute vertex and index buffer using face data
  std::unique_ptr<IndexedMeshLoader> m_loader(new IndexedMeshLoader(updatedVertices, *ani->GetFaces()));
  m_vertices = m_loader->getVertices();
  m_indices = m_loader->getIndices();
  
  // Update the vertex buffer
  glBindBuffer(GL_ARRAY_BUFFER, this->m_vertexArrayBuffers[VERTEX_VB]);
  glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizei>(m_vertices.size() * sizeof(Vertex)), m_vertices.data(), GL_DYNAMIC_DRAW);
  // Update the index buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_vertexArrayBuffers[INDEX_VB]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizei>(m_indices.size() * sizeof(unsigned int)), m_indices.data(), GL_DYNAMIC_DRAW);
  
  return true;
}

void CEGeometry::Draw()
{
  m_shader->use();
  m_texture->use();
  glBindVertexArray(this->m_vertexArrayObject);
  
  glDrawElementsBaseVertex(GL_TRIANGLES, (int)this->m_indices.size(), GL_UNSIGNED_INT, 0, 0);
  
  glBindVertexArray(0);
}

void CEGeometry::ConfigureShaderUniforms(C2MapFile* map, C2MapRscFile* rsc)
{
  auto color = rsc->getFadeColor();
  float r = color.r / 255.0f;
  float g = color.g / 255.0f;
  float b = color.b / 255.0f;
  float a = color.a;
  
  // Define a brightness factor
  float brightnessFactor = 1.2f; // Increase by 20%
  
  // Increase the brightness
  r = std::min(r * brightnessFactor, 1.0f);
  g = std::min(g * brightnessFactor, 1.0f);
  b = std::min(b * brightnessFactor, 1.0f);
  
  auto dist = (map->getTileLength() * (map->getWidth() / 8.f));
  auto dColor = glm::vec4(r, g, b, a);
  
  m_shader->use();
  m_shader->setFloat("view_distance", dist);
  m_shader->setVec4("distanceColor", dColor);
  m_shader->setFloat("terrainWidth", map->getWidth());
  m_shader->setFloat("terrainHeight", map->getHeight());
  m_shader->setFloat("tileWidth", map->getTileLength());
}

void CEGeometry::Update(Transform &transform, Camera &camera)
{
  this->m_shader->use();
  double t = glfwGetTime();
  glm::mat4 MVP = transform.GetStaticModelVP(camera);
  glm::mat4 model = transform.GetStaticModel();
  
  this->m_shader->setMat4("MVP", MVP);
  this->m_shader->setMat4("model", model);
  this->m_shader->setMat4("view", camera.GetVM());
  this->m_shader->setMat4("projection", camera.GetProjection());
  this->m_shader->setFloat("time", (float)t);
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
  glBufferData(GL_ARRAY_BUFFER, this->m_num_instances*sizeof(glm::mat4), transforms.data(), GL_DYNAMIC_DRAW);
}

const std::vector<Vertex>& CEGeometry::GetVertices() const {
  return m_vertices;
}

const int CEGeometry::GetCurrentFrame() const {
  return m_current_frame;
}
