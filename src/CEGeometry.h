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


class Vertex;
class CETexture;
class ShaderProgram;

struct Camera;
struct Transform;

class CEGeometry {
private:
  int m_texture_height;
  
  bool m_transparency;

  float m_vertice_light[4][1024];

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
  std::unique_ptr<CETexture> m_texture;
  std::unique_ptr<ShaderProgram> m_shader;

public:
  CEGeometry(std::vector < Vertex > vertices, std::vector < unsigned int > indices, std::unique_ptr<CETexture> texture);
  ~CEGeometry();
  
  void loadObjectIntoMemoryBuffer();
  
  CETexture* getTexture();

  void saveTextureAsBMP(const std::string& file_name );
  void Update(Transform& transform, Camera& camera);
  void Draw();

  void Update(Camera& camera);
  void UpdateInstances(std::vector<glm::mat4> transforms);
  void DrawInstances();
};

