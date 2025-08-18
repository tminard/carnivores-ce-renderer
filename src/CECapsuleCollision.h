//
//  CECapsuleCollision.h
//  CarnivoresRenderer
//
//  Bullet Physics implementation of capsule collision detection
//  Focuses on world object collision, not terrain collision
//

#ifndef CECapsuleCollision_h
#define CECapsuleCollision_h

#include "ICapsuleCollision.h"
#include <memory>

// Forward declarations for Bullet Physics
class btCapsuleShape;
class btRigidBody;
class btDefaultMotionState;
class btDiscreteDynamicsWorld;

/**
 * Bullet Physics implementation of capsule collision.
 * Creates a kinematic capsule body that moves with the entity and tests for collisions.
 * Designed specifically for world object collision (rocks, trees, buildings).
 */
class CECapsuleCollision : public ICapsuleCollision {
private:
    // Bullet Physics objects
    btCapsuleShape* m_capsuleShape;
    btRigidBody* m_capsuleBody;
    btDefaultMotionState* m_motionState;
    btDiscreteDynamicsWorld* m_dynamicsWorld; // Not owned
    
    // Collision filtering constants
    static const short PLAYER_COLLISION_GROUP = 1 << 4;  // PLAYER_GROUP (bit 4)
    static const short PLAYER_COLLISION_MASK = (1 << 2) | (1 << 3);  // Collide with OBJECT_GROUP and WATER_GROUP
    
    // Current state
    glm::vec3 m_position;
    float m_radius;
    float m_height;
    bool m_enabled;
    bool m_bodyCreated;
    bool m_isShuttingDown;
    
    // Helper methods
    void createCapsuleBody();
    void destroyCapsuleBody();
    glm::vec3 btVector3ToGlm(const class btVector3& vec) const;
    class btVector3 glmToBtVector3(const glm::vec3& vec) const;
    
public:
    /**
     * Create a capsule collision component.
     * @param dynamicsWorld Bullet Physics world to add the capsule to
     * @param radius Initial capsule radius
     * @param height Initial capsule height (cylindrical part)
     * @param initialPosition Starting position
     */
    CECapsuleCollision(btDiscreteDynamicsWorld* dynamicsWorld, 
                       float radius = 3.5f, 
                       float height = 18.0f, 
                       const glm::vec3& initialPosition = glm::vec3(0.0f));
    
    /**
     * Destructor - cleans up Bullet Physics objects
     */
    ~CECapsuleCollision() override;
    
    // ICapsuleCollision interface implementation
    void updatePosition(const glm::vec3& position) override;
    bool checkMovement(const glm::vec3& from, const glm::vec3& to) const override;
    void setDimensions(float radius, float height) override;
    glm::vec3 getPosition() const override;
    void setEnabled(bool enabled) override;
    bool isEnabled() const override;
    
    // Additional utility methods
    
    /**
     * Get the underlying Bullet Physics rigid body.
     * Useful for advanced collision queries.
     * @return Pointer to the rigid body (can be nullptr if disabled)
     */
    btRigidBody* getRigidBody() const { return m_capsuleBody; }
    
    /**
     * Check if the capsule is currently in contact with any objects.
     * @return true if touching objects, false if clear
     */
    bool hasContacts() const;
};

#endif /* CECapsuleCollision_h */