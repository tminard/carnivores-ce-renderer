//
//  CEParticle.h
//  CE Character Lab
//
//  Basic particle system for impact effects
//

#ifndef __CE_Character_Lab__CEParticle__
#define __CE_Character_Lab__CEParticle__

#include <glm/glm.hpp>

enum class ParticleType {
    DEFAULT,
    BLOOD_STREAK
};

struct CEParticle {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec4 color;
    float life;        // Current life (0.0 = dead, 1.0 = fully alive)
    float maxLife;     // Maximum life duration
    float size;        // Particle size
    float gravity;     // Gravity effect strength
    ParticleType type; // Particle type for texture selection
    
    CEParticle() : position(0.0f), velocity(0.0f), color(1.0f), 
                   life(0.0f), maxLife(1.0f), size(1.0f), gravity(-9.8f), type(ParticleType::DEFAULT) {}
    
    bool isAlive() const { return life > 0.0f; }
    
    void update(float deltaTime) {
        if (!isAlive()) return;
        
        // Update position
        position += velocity * deltaTime;
        
        // Apply gravity
        velocity.y += gravity * deltaTime;
        
        // Apply air resistance
        velocity *= 0.98f;
        
        // Update life
        life -= deltaTime / maxLife;
        if (life < 0.0f) life = 0.0f;
        
        // Fade alpha over time
        color.a = life;
    }
};

#endif /* defined(__CE_Character_Lab__CEParticle__) */