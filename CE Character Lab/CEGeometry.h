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
  std::unique_ptr<CETexture> m_texture;

public:
  CEGeometry(std::vector < Vertex > vertices, std::vector < unsigned int > indices, std::unique_ptr<CETexture> texture);
  ~CEGeometry();
  
  void loadObjectIntoMemoryBuffer(); // loads the object into OpenGL's memory
  void DEP_hint_ignoreLighting(); // instruct the geom to ignore lighting. Depreciated. Used with old C2 models.
  
  CETexture* getTexture();

  void exportAsOBJ(const std::string& file_name);
  void saveTextureAsBMP(const std::string& file_name );
  void Draw();
};

