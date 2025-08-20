#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

class ShaderProgram;
class CEWorldModel;
struct Transform;

class CEShadowManager
{
private:
    // Shadow map configuration
    static const int SHADOW_MAP_SIZE = 2048;
    static const float SHADOW_DISTANCE;
    
    // OpenGL objects
    unsigned int m_shadow_framebuffer;
    unsigned int m_shadow_depth_texture;
    
    // Light configuration  
    glm::vec3 m_light_direction;
    glm::vec3 m_light_position;
    glm::mat4 m_light_view_matrix;
    glm::mat4 m_light_projection_matrix;
    glm::mat4 m_light_space_matrix;
    
    // Shaders
    std::unique_ptr<ShaderProgram> m_shadow_shader;
    
    // Viewport restoration
    int m_saved_viewport[4];
    
    // Caching system
    std::string m_current_map_hash;
    std::unordered_map<std::string, std::vector<uint8_t>> m_cached_shadow_maps;
    
    // Internal methods
    void setupShadowFramebuffer();
    void calculateLightMatrices(const glm::vec3& center, float size);
    bool loadCachedShadowMap(const std::string& mapHash);
    void saveShadowMapCache(const std::string& mapHash);
    std::string generateMapHash(const std::vector<CEWorldModel*>& objects);
    
public:
    CEShadowManager();
    ~CEShadowManager();
    
    // Core functionality
    void initialize();
    void setLightDirection(const glm::vec3& direction);
    void setLightPosition(const glm::vec3& position);
    
    // Shadow map generation
    void generateShadowMap(const std::vector<CEWorldModel*>& shadowCasters, 
                          const glm::vec3& sceneCenter, 
                          float sceneRadius);
    
    // Rendering support
    void beginShadowPass();
    void endShadowPass();
    void renderShadowCaster(CEWorldModel* model, const Transform& transform);
    
    // Getters for rendering pipeline
    const glm::mat4& getLightSpaceMatrix() const { return m_light_space_matrix; }
    const glm::vec3& getLightDirection() const { return m_light_direction; }
    const glm::vec3& getLightPosition() const { return m_light_position; }
    unsigned int getShadowMapTexture() const { return m_shadow_depth_texture; }
    
    // Cache management
    void setCacheKey(const std::string& mapName, const std::vector<CEWorldModel*>& objects);
    bool hasCachedShadows() const;
    
    // Static utility
    static bool shouldCastShadow(CEWorldModel* model);
};
