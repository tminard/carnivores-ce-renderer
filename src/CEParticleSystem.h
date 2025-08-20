//
//  CEParticleSystem.h
//  CE Character Lab
//
//  Particle system manager for impact effects
//

#ifndef __CE_Character_Lab__CEParticleSystem__
#define __CE_Character_Lab__CEParticleSystem__

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include "CEParticle.h"

class ShaderProgram;
class Camera;

class CEParticleSystem {
private:
    std::vector<CEParticle> m_particles;
    size_t m_maxParticles;
    size_t m_nextParticleIndex;
    
    // OpenGL rendering resources
    GLuint m_VAO;
    GLuint m_VBO;
    std::unique_ptr<ShaderProgram> m_shader;
    GLuint m_textureID;
    GLuint m_bloodStreakTextureID;
    
    // Rendering data
    std::vector<float> m_instanceData; // Position, size, color data for instanced rendering
    
    void initializeOpenGL();
    void createDefaultTexture();
    void createBloodStreakTexture();
    void renderParticleBatch(const std::vector<float>& instanceData);
    size_t findDeadParticle();

public:
    CEParticleSystem(size_t maxParticles = 1000);
    ~CEParticleSystem();
    
    // Particle emission
    void emitGroundImpact(const glm::vec3& position, const glm::vec3& normal, int count = 20);
    void emitDustCloud(const glm::vec3& position, int count = 15);
    void emitDebris(const glm::vec3& position, const glm::vec3& impactDirection, int count = 10);
    void emitBloodSplash(const glm::vec3& position, const glm::vec3& normal, int count = 25);
    
    // System management
    void update(float deltaTime);
    void render(Camera* camera);
    void clear();
    
    // Configuration
    void setTexture(GLuint textureID) { m_textureID = textureID; }
    size_t getActiveParticleCount() const;
};

#endif /* defined(__CE_Character_Lab__CEParticleSystem__) */