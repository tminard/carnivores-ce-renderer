//
//  CEBulletPlayerController.h
//  CE Character Lab
//
//  Physics-based player controller using Bullet Physics for realistic movement, gravity, and collision
//

#ifndef __CE_Character_Lab__CEBulletPlayerController__
#define __CE_Character_Lab__CEBulletPlayerController__

#include <glm/glm.hpp>
#include <memory>

// Forward declarations
class CEPhysicsWorld;
class btRigidBody;
class btCapsuleShape;
class btDefaultMotionState;
class btVector3;

class CEBulletPlayerController
{
public:
    struct MovementInput {
        bool forward = false;
        bool backward = false;
        bool left = false;
        bool right = false;
        bool jump = false;
        bool flying = false;
    };

    struct MovementState {
        bool isGrounded = false;
        bool isJumping = false;
        bool isFlying = false;
        bool isSliding = false;
        float groundSlope = 0.0f;
        glm::vec3 groundNormal = glm::vec3(0, 1, 0);
        glm::vec3 velocity = glm::vec3(0);
        float speed = 0.0f;
    };

private:
    CEPhysicsWorld* m_physicsWorld;
    
    // Physics body components (using raw pointers - Bullet handles lifecycle)
    btRigidBody* m_playerBody;
    btCapsuleShape* m_playerShape;
    
    // Player parameters
    float m_capsuleRadius;
    float m_capsuleHeight;
    float m_mass;
    
    // Movement parameters
    float m_walkSpeed;
    float m_runSpeed;
    float m_jumpForce;
    float m_maxSlopeAngle;
    float m_airControl;
    
    // Movement state
    MovementState m_state;
    glm::vec3 m_inputDirection;
    double m_lastJumpTime;
    double m_jumpCooldown;
    
    // Ground detection
    float m_groundCheckDistance;
    int m_groundContactCount;
    glm::vec3 m_averageGroundNormal;
    
    // Physics update helpers
    void updateGroundState();
    void applyMovementForces(const MovementInput& input, float deltaTime);
    void applyJumpForce();
    void handleSlope(float deltaTime);
    void limitVelocity();
    
    // Collision detection helpers
    bool isOnGround() const;
    float getGroundSlope() const;
    glm::vec3 getGroundNormal() const;
    
    // Utility functions
    glm::vec3 btVector3ToGlm(const btVector3& vec) const;
    btVector3 glmToBtVector3(const glm::vec3& vec) const;

public:
    CEBulletPlayerController(CEPhysicsWorld* physicsWorld, 
                           const glm::vec3& initialPosition = glm::vec3(0, 10, 0));
    ~CEBulletPlayerController();
    
    // Core update loop
    void update(const MovementInput& input, double currentTime, float deltaTime, const glm::vec3& cameraForward = glm::vec3(0, 0, -1));
    
    // Position and state access
    glm::vec3 getPosition() const;
    void setPosition(const glm::vec3& position);
    glm::vec3 getVelocity() const;
    void setVelocity(const glm::vec3& velocity);
    
    // Movement state queries
    const MovementState& getMovementState() const { return m_state; }
    bool isGrounded() const { return m_state.isGrounded; }
    bool isJumping() const { return m_state.isJumping; }
    bool isFlying() const { return m_state.isFlying; }
    
    // Configuration
    void setWalkSpeed(float speed) { m_walkSpeed = speed; }
    void setRunSpeed(float speed) { m_runSpeed = speed; }
    void setJumpForce(float force) { m_jumpForce = force; }
    void setMaxSlopeAngle(float angle) { m_maxSlopeAngle = angle; }
    void setFlying(bool flying);
    
    // Physics body access (for camera tracking)
    btRigidBody* getRigidBody() const { return m_playerBody; }
    
    // Collision groups for player (matching CEPhysicsWorld::CollisionGroups)
    static const short PLAYER_COLLISION_GROUP = 1 << 4;  // PLAYER_GROUP
    static const short PLAYER_COLLISION_MASK = (1 << 1) | (1 << 2) | (1 << 3);  // Collide with terrain, objects, and water
};

#endif /* defined(__CE_Character_Lab__CEBulletPlayerController__) */