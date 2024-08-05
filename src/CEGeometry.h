/*
* This object is responsible for storing immutable geometry (vertices, indices, etc),
* and textures.
*
* Equivalent to SceneKit's SCNGeometry.
*/
#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <memory>
#include <vector>
#include <cstdint>
#include <fstream>
#include <string>

class Vertex;
class CETexture;
class ShaderProgram;
class C2MapFile;
class C2MapRscFile;
class CEAnimation;

struct Camera;
struct Transform;

class CEGeometry {
private:
  int m_texture_height;
  
  bool m_transparency;
  
  int m_current_frame;

  enum {
    VERTEX_VB=0,
    INDEX_VB=1,
    NUM_BUFFERS=2
  };

  GLuint m_instanced_vab;
  GLuint m_num_instances;

  GLuint m_vertexArrayObject;
  GLuint m_vertexArrayBuffers[NUM_BUFFERS];

  std::vector < Vertex > m_vertices;
  std::vector < unsigned int > m_indices;
  std::unique_ptr<ShaderProgram> m_shader;

  std::shared_ptr<CETexture> m_texture;
public:
  CEGeometry(std::vector < Vertex > vertices, std::vector < unsigned int > indices, std::shared_ptr<CETexture> texture, std::string shaderName);
  ~CEGeometry();
  
  void loadObjectIntoMemoryBuffer(std::string shaderName);
  
  std::weak_ptr<CETexture> getTexture();

  void saveTextureAsBMP(const std::string& file_name );
  void Update(Transform& transform, Camera& camera);
  bool SetAnimation(std::weak_ptr<CEAnimation> animation, double atTime, double startAt, double lastUpdateAt, bool deferUpdate, bool maxFPS, bool notVisible, float playbackSpeed);
  void Draw();
  
  void ConfigureShaderUniforms(C2MapFile* map, C2MapRscFile* rsc);
  
  const std::vector<Vertex>& GetVertices() const;
  
  const int GetCurrentFrame() const;

  void UpdateInstances(std::vector<glm::mat4> transforms);
  void DrawInstances();
};

