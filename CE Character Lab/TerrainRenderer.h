/*
 * The purpose of this class is to handle the loading and rendering of the current
 * visible terrain.
*/

#ifndef __CE_Character_Lab__TerrainRenderer__
#define __CE_Character_Lab__TerrainRenderer__

#include <stdio.h>

#include <OpenGL/gl3.h>
//#include <GLFW/glfw3.h>
#include <vector>

class Vertex;
class C2MapFile;
class TerrainRenderer
{
private:
  std::vector < Vertex > m_vertices;
  std::vector < unsigned int > m_indices;
  int m_num_indices;

  GLuint m_vertexArrayObject;

  GLuint m_vertexArrayBuffer;
  GLuint m_indicesArrayBuffer;
  
  C2MapFile* m_cmap_data_weak;
  
  void loadIntoHardwareMemory();
public:
  constexpr static const float WORLD_SIZE = 1024.f;
  constexpr static const float TILE_SIZE = 256.f;

  constexpr static const float TCMAX = 255.5f;
  constexpr static const float TCMIN = 0.5f;
  constexpr static const float _ZSCALE = (16.f*65534.f);

  TerrainRenderer(C2MapFile* cMapWeak);
  ~TerrainRenderer();

  void Render();
  void UpdateForPos();
};

#endif /* defined(__CE_Character_Lab__TerrainRenderer__) */
