/*
 * The purpose of this class is to handle the loading and rendering of the current
 * visible terrain.
*/

#ifndef __CE_Character_Lab__TerrainRenderer__
#define __CE_Character_Lab__TerrainRenderer__

#include <stdio.h>

#include <OpenGL/gl3.h>
#include <GLFW/glfw3.h>
#include <vector>

class Vertex;
class TerrainRenderer
{
private:
  constexpr static const float SIZE_PER_SIDE = 129;
  constexpr static const float MIN_POS = 0.f;
  constexpr static const float POS_RANGE = 10.f;
  constexpr static const float WORLD_SIZE = 500.f;
  std::vector < Vertex > m_vertices;
  std::vector < unsigned int > m_indices;
  int m_num_indices;

  GLuint m_vertexArrayObject;

  GLuint m_vertexArrayBuffer;
  GLuint m_indicesArrayBuffer;
  
  void loadIntoHardwareMemory();
public:
  TerrainRenderer();
  ~TerrainRenderer();

  void Render();
  void UpdateForPos();
};

#endif /* defined(__CE_Character_Lab__TerrainRenderer__) */
