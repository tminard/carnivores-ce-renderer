#pragma once

#include <stdio.h>
#include <exception>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include <vector>
#include <map>
#include <unordered_map>
#include <memory>

#include "transform.h"

class Vertex;
class C2MapFile;
class C2MapRscFile;
class CEWorldModel;
class ShaderProgram;

struct CETerrainVertex;
struct Transform;
struct Camera;
class CEShadowManager;

class TerrainRenderer
{
private:
  struct _Water {
    GLuint m_vao = 0;
    GLuint m_vab = 0;
    GLuint m_iab = 0;
    int m_water_id = 0;
    int m_texture_id = 0;
    float m_height = 0.f;
    float m_transparency = 0.f; // 1.f = fully transparent on RGB color. 0.f = not transparent. 0.5 = half transparency
    int m_height_unscaled = 0;
    std::vector < Vertex > m_vertices;
    std::vector < unsigned int > m_indices;
    int m_vertex_count = 0;
    int m_num_indices = 0;
  };

  std::vector <_Water> m_waters;

  std::vector < CETerrainVertex > m_vertices;
  std::vector < unsigned int > m_indices;
  int m_num_indices;
  double m_last_update_time;
  
  GLuint m_vertex_array_object;
  
  GLuint m_vertex_array_buffer;
  GLuint m_indices_array_buffer;
  
  GLuint underwaterStateTexture;
  GLuint heightmapTexture;
  
  std::unique_ptr<ShaderProgram> m_shader;
  std::unique_ptr<ShaderProgram> m_water_shader;
  
  std::shared_ptr<C2MapFile> m_cmap_data_weak;
  std::shared_ptr<C2MapRscFile> m_crsc_data_weak;
  
  void preloadObjectMap();
  void loadShader();
  void loadIntoHardwareMemory();
  void exportAsRaw();
  
  void loadWaterIntoMemory();
  void loadWaterAt(int x, int y);
  void loadWaterAtWithIndex(int x, int y, int forceWaterIndex);

  glm::vec2 calcAtlasUV(int texID, glm::vec2 uv);
  glm::vec2 scaleAtlasUV(glm::vec2 atlas_uv, int texture_id);
  glm::vec4 getScaledAtlasUVQuad(glm::vec2 atlas_uv, int texture_id_1, int texture_id_2);

  glm::vec3 calcWorldVertex(int tile_x, int tile_y, bool water, float water_height_scaled);

  std::array<glm::vec2, 4> calcUVMapForQuad(int x, int y, bool quad_reversed, int rotation_code);
  void updateUnderwaterStateTexture(const std::vector<float>& data);
  void createHeightmapTexture();
public:
  constexpr static const float TCMAX = 255.5f;
  constexpr static const float TCMIN = 0.5f;
  constexpr static const float _ZSCALE = (16.f*65534.f); // MAX_UNSIGNED_SHORT*16 - original engine used this for scaling heights
  TerrainRenderer(std::shared_ptr<C2MapFile> cMapWeak, std::shared_ptr<C2MapRscFile> cRscWeak);
  ~TerrainRenderer();
  
  void RenderObjects(Camera& camera);
  void RenderObjectsWithShadows(Camera& camera, CEShadowManager* shadowManager);
  void Render();
  void RenderWithShadows(Camera& camera, CEShadowManager* shadowManager);
  void Update(Transform& transform, Camera& camera);
  void RenderWater();
};
