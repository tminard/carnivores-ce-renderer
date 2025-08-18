//
//  ICapsuleCollision.h
//  CarnivoresRenderer
//
//  Interface for capsule-based collision detection components
//  Provides object collision while maintaining separation from movement logic
//

#ifndef ICapsuleCollision_h
#define ICapsuleCollision_h

#include <glm/vec3.hpp>

/**
 * Interface for capsule collision detection components.
 * Allows different collision implementations while maintaining a consistent API.
 */
class ICapsuleCollision {
public:
    virtual ~ICapsuleCollision() = default;
    
    /**
     * Update the capsule's position in the collision world.
     * Should be called whenever the entity moves.
     * @param position The new world position for the capsule center
     */
    virtual void updatePosition(const glm::vec3& position) = 0;
    
    /**
     * Check if movement from one position to another is allowed.
     * Tests for collisions with world objects (not terrain).
     * @param from Starting position
     * @param to Target position
     * @return true if movement is clear, false if blocked by collision
     */
    virtual bool checkMovement(const glm::vec3& from, const glm::vec3& to) const = 0;
    
    /**
     * Check movement and get collision information for sliding.
     * @param from Starting position
     * @param to Target position
     * @param outCollisionNormal Normal vector of collision surface (only valid if collision detected)
     * @return true if movement is clear, false if blocked by collision
     */
    virtual bool checkMovementWithNormal(const glm::vec3& from, const glm::vec3& to, glm::vec3& outCollisionNormal) const = 0;
    
    /**
     * Set the dimensions of the collision capsule.
     * @param radius Capsule radius
     * @param height Capsule height (cylindrical part, not including hemispheres)
     */
    virtual void setDimensions(float radius, float height) = 0;
    
    /**
     * Get current position of the capsule.
     * @return Current world position
     */
    virtual glm::vec3 getPosition() const = 0;
    
    /**
     * Enable or disable collision detection.
     * Useful for temporary states like cutscenes or special abilities.
     * @param enabled true to enable collision, false to disable
     */
    virtual void setEnabled(bool enabled) = 0;
    
    /**
     * Check if collision detection is currently enabled.
     * @return true if enabled, false if disabled
     */
    virtual bool isEnabled() const = 0;
};

#endif /* ICapsuleCollision_h */