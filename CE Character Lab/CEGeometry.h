/*
* This object is responsible for storing immutable geometry (vertices, indices, etc),
* and textures.
*
* Equivalent to SceneKit's SCNGeometry.
*/
#pragma once

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>

#include <memory>
#include <vector>
#include <cstdint>
#include <fstream>


class Vertex;
class CETexture;
class CEVertexBuffer;

class CEGeometry {
private:
  int m_texture_height;

  float m_vertice_light[4][1024];

  enum {
    VERTEX_VB=0,
    INDEX_VB=1,
    NUM_BUFFERS=2
  };

  GLuint m_vertexArrayObject;
  GLuint m_vertexArrayBuffers[NUM_BUFFERS];

  std::vector < Vertex > m_vertices;
  std::vector < unsigned int > m_indices;
  std::shared_ptr<CETexture> m_texture;
  std::unique_ptr<CEVertexBuffer> m_vertex_buffer;

public:
  CEGeometry(std::vector < Vertex > vertices, std::vector < unsigned int > indices, std::shared_ptr<CETexture> texture);
  CEGeometry(std::vector < Vertex > vertices, std::vector < unsigned int > indices, std::shared_ptr<CETexture> texture, std::unique_ptr<CEVertexBuffer> vertex_buffer);

  ~CEGeometry();

  void DEP_hint_ignoreLighting(); // instruct the geom to ignore lighting. Depreciated. Used with old C2 models.
  
  CETexture* getTexture();

  void exportAsOBJ(const std::string& file_name);
  void saveTextureAsBMP(const std::string& file_name );
  void Draw();
};

