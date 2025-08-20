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

// Bullet Physics for mesh collision generation
#include <btBulletCollisionCommon.h>
#include <BulletCollision/Gimpact/btGImpactShape.h>

namespace fs = std::filesystem;
using json = nlohmann::json;

CEGeometry::CEGeometry(std::vector < Vertex > vertices, std::vector < uint32_t > indices, std::shared_ptr<CETexture> texture, std::string shaderName)
: m_vertices(vertices), m_indices(indices), m_texture(texture)
{
  this->loadObjectIntoMemoryBuffer(shaderName);
  m_current_frame = 0;
  m_has_physics = false;
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

bool CEGeometry::SetAnimation(std::weak_ptr<CEAnimation> animation, int atFrame) {
  std::shared_ptr<CEAnimation> ani = animation.lock();
  if (!ani) {
    // Handle the case where the animation is no longer available or doesn't exist
    return false;
  }
  
  if (m_current_frame == atFrame) {
    return true;
  }
  
  if (atFrame >= ani->m_number_of_frames || atFrame < 0) {
    return false;
  }

  double timePerFrame = 1.0 / double(ani->m_kps);
  double atTime = timePerFrame*atFrame;
  
  return SetAnimation(animation, atTime, 0, 0, false, true, false, 1.0f, false, true);
}

void CEGeometry::applyAnimFaceOrdered(std::vector<Vertex>& m_vertices,
                          const std::vector<TFace>& faces,
                          const short* aniData,
                          int numVerts, int frameA, int frameB, float t)
{
  const int stride = numVerts * 3; // units per frame
  const short* A = aniData + frameA * stride;
  const short* B = aniData + frameB * stride;
  const float k1 = 1.f - t, k2 = t;
  const bool doLoop = frameA != frameB;

  for (size_t f = 0; f < faces.size(); ++f) {
    const auto& fc = faces[f];

    auto fetch = [&](uint32_t vi) -> glm::vec3 {
      const int o = int(vi) * 3;
      if (doLoop) {
        const float x = (A[o+0]*k1 + B[o+0]*k2) / 8.f;
        const float y = (A[o+1]*k1 + B[o+1]*k2) / 8.f;
        const float z = (A[o+2]*k1 + B[o+2]*k2) / 8.f;
        
        return {x, y, z};
      }
      
      const float x = (A[o+0]*k1) / 8.f;
      const float y = (A[o+1]*k1) / 8.f;
      const float z = (A[o+2]*k1) / 8.f;
      return {x,y,z};
    };

    m_vertices[f*3 + 0].setPos(fetch(fc.v1));
    m_vertices[f*3 + 1].setPos(fetch(fc.v2));
    m_vertices[f*3 + 2].setPos(fetch(fc.v3));
  }
}

inline glm::vec3 fetchPosNoInterp(const short* A, int vi) {
  const int o = vi * 3;
  constexpr float s = 1.0f / 8.0f;
  return { A[o+0]*s, A[o+1]*s, A[o+2]*s };
}

void applyAnimFaceOrdered_NoInterp(std::vector<Vertex>& m_vertices,
                                   const std::vector<TFace>& faces,
                                   const short* aniData, int numVerts,
                                   int frameA)
{
  const int stride = numVerts * 3;
  const short* A = aniData + frameA * stride;

  for (size_t f = 0; f < faces.size(); ++f) {
    const auto& fc = faces[f];
    m_vertices[f*3 + 0].setPos(fetchPosNoInterp(A, fc.v1));
    m_vertices[f*3 + 1].setPos(fetchPosNoInterp(A, fc.v2));
    m_vertices[f*3 + 2].setPos(fetchPosNoInterp(A, fc.v3));
  }
}

bool CEGeometry::SetAnimation(std::weak_ptr<CEAnimation> animation, double atTime, double startAt, double lastUpdateAt, bool deferUpdate, bool maxFPS, bool notVisible, float playbackSpeed, bool loop, bool noInterpolation) {
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
  
  auto aniData = *ani->GetAnimationData();
  auto origVData = ani->GetOriginalVertices();
  assert(aniData.size() % 3 == 0);
  size_t numVertices = origVData->size();
  
  auto faces = *ani->GetFaces();
  if (noInterpolation) {
    // Use discrete frames without interpolation (for weapons)
    applyAnimFaceOrdered_NoInterp(m_vertices, faces, aniData.data(), static_cast<int>(numVertices), currentFrame);
  } else {
    float k2 = static_cast<float>(exactFrameIndex - currentFrame); // Interpolation factor
    applyAnimFaceOrdered(m_vertices, faces, aniData.data(), static_cast<int>(numVertices), currentFrame, nextFrame, k2);
  }
  
  glBindBuffer(GL_ARRAY_BUFFER, this->m_vertexArrayBuffers[VERTEX_VB]);
  auto sizeBytes = static_cast<GLsizei>(m_vertices.size() * sizeof(Vertex));
  void* ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0, sizeBytes,
      GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
  std::memcpy(ptr, m_vertices.data(), sizeBytes);
  glUnmapBuffer(GL_ARRAY_BUFFER);
  
  return true;
}

void CEGeometry::DrawNaked()
{
  m_texture->use();
  glBindVertexArray(this->m_vertexArrayObject);
  
  glDrawElementsBaseVertex(GL_TRIANGLES, (int)this->m_indices.size(), GL_UNSIGNED_INT, 0, 0);
  
  glBindVertexArray(0);
}

void CEGeometry::Draw()
{
  m_shader->use();
  this->DrawNaked();
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

void CEGeometry::DrawInstancesWithShader(ShaderProgram* externalShader)
{
  if (externalShader) {
    externalShader->use();
  } else {
    this->m_shader->use();
  }
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

const std::vector<unsigned int>& CEGeometry::GetIndices() const {
  return m_indices;
}

const int CEGeometry::GetCurrentFrame() const {
  return m_current_frame;
}

void CEGeometry::setShader(std::string shaderName)
{
  std::ifstream f("config.json");
  json data = json::parse(f);
  
  fs::path basePath = fs::path(data["basePath"].get<std::string>());
  fs::path shaderPath = basePath / "shaders";
  
  // Replace the current shader with a new one
  this->m_shader = std::unique_ptr<ShaderProgram>(new ShaderProgram((shaderPath / (shaderName + ".vs")).string(), (shaderPath / (shaderName + ".fs")).string()));
  this->m_shader->use();
  this->m_shader->setBool("enable_transparency", true);
}

void CEGeometry::EnablePhysics()
{
  if (m_has_physics) {
    return;
  }

  btTriangleIndexVertexArray* tiv = new btTriangleIndexVertexArray();

  btIndexedMesh im;
  im.m_numTriangles      = static_cast<int>(m_indices.size() / 3);
  im.m_triangleIndexBase = reinterpret_cast<const unsigned char*>(m_indices.data());
  im.m_triangleIndexStride = sizeof(uint32_t) * 3;
  im.m_indexType         = PHY_INTEGER; // 32-bit indices

  im.m_numVertices       = static_cast<int>(m_vertices.size());
  im.m_vertexBase        = reinterpret_cast<const unsigned char*>(m_vertices.data()); // position at offset 0
  im.m_vertexStride      = sizeof(Vertex);
  im.m_vertexType        = PHY_FLOAT;   // btScalar == float

  tiv->addIndexedMesh(im, PHY_INTEGER);
  this->m_bullet_tiv = tiv;
  
  // build the non-static model in case the game needs it
  m_gimpact = new btBvhTriangleMeshShape(tiv, true);
  m_gimpact->setMargin(0.01f);
  
  m_has_physics = true;
}

btTriangleIndexVertexArray* CEGeometry::getPhysicalMesh() const
{
  assert(m_has_physics);

  return m_bullet_tiv;
}

btBvhTriangleMeshShape* CEGeometry::getMeshShape() const
{
  return m_gimpact;
}

std::vector<glm::vec3> CEGeometry::getDebugPhysicsVertices() const
{
    std::vector<glm::vec3> debugVertices;
    
    // Extract vertex positions for debug visualization
    for (const auto& vertex : m_vertices) {
        debugVertices.push_back(vertex.getPos());
    }
    
    return debugVertices;
}
