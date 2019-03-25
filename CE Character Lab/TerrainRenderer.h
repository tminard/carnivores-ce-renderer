/*
 * The purpose of this class is to handle the loading and rendering of the current
 * visible terrain.
 *
 * TODO:
 *  [] Generate BSP tree
 *      [] Generate multiple-LOD (MRBSP) trees (see: http://graphicsinterface.org/wp-content/uploads/gi1997-10.pdf)
 *      [] Merge in object data at runtime
 *      [] Include knowledge of the current camera location and handle perspective
 *  [] Properly implement texture mapping
 */

#ifndef __CE_Character_Lab__TerrainRenderer__
#define __CE_Character_Lab__TerrainRenderer__

#include <stdio.h>
#include <exception>

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include <vector>
#include <map>
#include <unordered_map>

#include "transform.h"

class Vertex;
class C2MapFile;
class C2MapRscFile;
class CEWorldModel;
class ShaderProgram;
class Transform;
class Camera;
class Shader; // TODO: replace global object shader with per object pool
class TerrainRenderer
{
private:
  struct _Water {
    GLuint m_vao;
    GLuint m_vab, m_iab;
    int m_water_id;
    int m_texture_id;
    float m_height;
    float m_transparency;
    int m_height_unscaled;
    std::vector < Vertex > m_vertices;
    std::vector < unsigned int > m_indices;
    int m_indices_count;
  };

  std::vector <_Water> m_waters;

  std::vector < Vertex > m_vertices;
  std::vector < unsigned int > m_indices;
  int m_num_indices;
  
  GLuint m_vertex_array_object;
  
  GLuint m_vertex_array_buffer;
  GLuint m_indices_array_buffer;
  
  std::unique_ptr<ShaderProgram> m_shader;
  
  C2MapFile* m_cmap_data_weak;
  C2MapRscFile* m_crsc_data_weak;
  
  void preloadObjectMap();
  void loadShader();
  void loadIntoHardwareMemory();
  void loadWaterIntoMemory();

  glm::vec2 calcAtlasUV(int texID, glm::vec2 uv);
  glm::vec2 scaleAtlasUV(glm::vec2 atlas_uv, int texture_id);
  glm::vec3 calcWorldVertex(int tile_x, int tile_y, bool water, float water_height_scaled);
  std::array<glm::vec2, 4> calcUVMapForQuad(int x, int y, bool quad_reversed, int rotation_code);
  
  void renderObjectsAtQuad(Camera& camera, int quad_x, int quad_y, float maxd, float lowrd, glm::vec3 cur_pos);
public:
  constexpr static const float TCMAX = 255.5f;
  constexpr static const float TCMIN = 0.5f;
  constexpr static const float _ZSCALE = (16.f*65534.f); // MAX_UNSIGNED_SHORT*16 - original engine used this for scaling heights
  TerrainRenderer(C2MapFile* cMapWeak, C2MapRscFile* cRscWeak);
  ~TerrainRenderer();
  
  void RenderObjects(Camera& camera);
  void Render();
  void Update(Transform& transform, Camera& camera);
  void RenderWater();

  void loadWaterAt(int x, int y);
};

#endif /* defined(__CE_Character_Lab__TerrainRenderer__) */
