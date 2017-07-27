/*
 * The purpose of this class is to handle the loading and rendering of the current
 * visible terrain.
*/

#ifndef __CE_Character_Lab__TerrainRenderer__
#define __CE_Character_Lab__TerrainRenderer__

#include <stdio.h>
#include <exception>

#include <OpenGL/gl3.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <map>
#include <vector>
#include <memory>

class Vertex;
class C2MapFile;
class C2MapRscFile;
class CETerrain;
class CETexture;
class CEMapBlock;
class Camera;
class Shader;

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
  C2MapRscFile* m_crsc_data_weak;
  
  void loadIntoHardwareMemory();
  void breakLoadedTerrainIntoChunks();

  glm::vec2 calcAtlasUV(int texID, glm::vec2 uv);
  glm::vec3 calcWorldVertex(int tile_x, int tile_y);
  std::array<glm::vec2, 4> calcUVMapForQuad(int x, int y, bool quad_reversed, int rotation_code);

  // WIP: Move to CETerrain blocks
  typedef std::pair<int, int> BlockKey;

  std::map<BlockKey, std::unique_ptr<CEMapBlock>> m_map_blocks;
  //std::unique_ptr<CETerrain> m_terrain;
  //std::vector<std::unique_ptr<CEMapBlock>> m_map_blocks;

public:

  constexpr static const float TCMAX = 255.5f;
  constexpr static const float TCMIN = 0.5f;
  constexpr static const float _ZSCALE = (16.f*65534.f);

  TerrainRenderer(C2MapFile* cMapWeak, C2MapRscFile* cRscWeak);
  ~TerrainRenderer();

  void Render(Camera* camera, Shader* obj_shader, Shader* terrain_shader);
  void UpdateForPos();
};

#endif /* defined(__CE_Character_Lab__TerrainRenderer__) */
