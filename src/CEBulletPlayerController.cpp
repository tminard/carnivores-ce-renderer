//
//  CEBulletPlayerController.cpp
//  CE Character Lab
//
//  Physics-based player controller using Bullet Physics for realistic movement, gravity, and collision
//

#include "CEBulletPlayerController.h"
#include "CEPhysicsWorld.h"

// Bullet Physics includes
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionShapes/btCapsuleShape.h>

#include <iostream>
#include <algorithm>
#include <cmath>

CEBulletPlayerController::CEBulletPlayerController(CEPhysicsWorld* physicsWorld, const glm::vec3& initialPosition)
    : m_physicsWorld(physicsWorld)
    , m_playerBody(nullptr)
    , m_playerShape(nullptr)
    , m_lastJumpTime(0.0)
    , m_groundContactCount(0)
    , m_averageGroundNormal(0, 1, 0)
{
    if (!m_physicsWorld) {
        return;
    }
    
    // Player capsule dimensions (based on previous correct feel)
    // Previous 20 units eye level felt right for human, so 1 unit = 8.5cm
    // Human: 0.6m wide, 1.5m body height -> 7 units wide, 18 units tall
    m_capsuleRadius = 3.5f;     // 0.6m shoulder width (7 unit diameter)
    m_capsuleHeight = 18.0f;    // 1.5m body height (eye level at ~20 units)
    m_mass = 70.0f;             // 70kg realistic human mass
    
    // Movement parameters (scaled to world where 1 unit = 8.5cm)
    // Single speed should be running speed for outdoor exploration
    // Human running: 5-6 m/s = 59-71 units/s  
    m_walkSpeed = 60.0f;        // Running speed - only speed we use
    m_runSpeed = 60.0f;         // Same as walk speed
    m_jumpForce = 1400.0f;      // Realistic jump force for 0.4m jump height
    m_maxSlopeAngle = 45.0f;    // Standard slope limit
    m_airControl = 0.2f;        // Some air control for better feel
    
    // Ground detection for capsule shape
    m_groundCheckDistance = 2.0f;  // Good step-up capability for terrain irregularities
    m_jumpCooldown = 0.2; // 200ms jump cooldown
    
    // Create capsule collision shape for smooth terrain traversal (FPS best practice)
    m_playerShape = new btCapsuleShape(m_capsuleRadius, m_capsuleHeight);
    
    // Set initial transform
    btTransform startTransform;
    startTransform.setIdentity();
    startTransform.setOrigin(glmToBtVector3(initialPosition));
    
    // Create motion state (Bullet will take ownership)
    btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
    
    // Calculate local inertia
    btVector3 localInertia(0, 0, 0);
    m_playerShape->calculateLocalInertia(m_mass, localInertia);
    
    // Create rigid body (Bullet takes ownership of motion state)
    btRigidBody::btRigidBodyConstructionInfo rbInfo(m_mass, motionState, m_playerShape, localInertia);
    
    // Set physics properties for outdoor terrain (grass/rock/mud)
    rbInfo.m_friction = 0.3f;           // Moderate friction for natural terrain
    rbInfo.m_restitution = 0.0f;        // No bouncing
    rbInfo.m_linearDamping = 0.3f;      // Some damping for control
    rbInfo.m_angularDamping = 1.0f;     // Complete angular damping to prevent spinning
    
    m_playerBody = new btRigidBody(rbInfo);
    
    // Prevent player from rotating (stay upright)
    m_playerBody->setAngularFactor(btVector3(0, 1, 0)); // Only allow Y-axis rotation (turning)
    
    // Set collision filtering
    m_physicsWorld->getDynamicsWorld()->addRigidBody(
        m_playerBody, 
        PLAYER_COLLISION_GROUP, 
        PLAYER_COLLISION_MASK
    );
    
    // Initialize state
    m_state.isFlying = false;
    m_inputDirection = glm::vec3(0);
}

CEBulletPlayerController::~CEBulletPlayerController()
{
    // Proper Bullet Physics cleanup order
    if (m_physicsWorld && m_physicsWorld->getDynamicsWorld() && m_playerBody) {
        // Remove rigid body from world BEFORE destroying anything
        m_physicsWorld->getDynamicsWorld()->removeRigidBody(m_playerBody);
        
        // Bullet Physics will automatically delete the motion state when we delete the rigid body
        delete m_playerBody;
        m_playerBody = nullptr;
    }
    
    // Clean up collision shape
    if (m_playerShape) {
        delete m_playerShape;
        m_playerShape = nullptr;
    }
}

void CEBulletPlayerController::update(const MovementInput& input, double currentTime, float deltaTime, const glm::vec3& cameraForward)
{
    if (!m_playerBody || !m_physicsWorld) return;
    
    // Input processing
    
    // Update movement state
    m_state.isFlying = input.flying;
    
    // Update ground detection
    updateGroundState();
    
    // Calculate input direction based on camera
    glm::vec3 forward = glm::normalize(glm::vec3(cameraForward.x, 0, cameraForward.z)); // Remove Y component for ground movement
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
    
    m_inputDirection = glm::vec3(0);
    if (input.forward) m_inputDirection += forward;
    if (input.backward) m_inputDirection -= forward;
    if (input.right) m_inputDirection += right;
    if (input.left) m_inputDirection -= right;
    
    // Normalize diagonal movement
    if (glm::length(m_inputDirection) > 0.0f) {
        m_inputDirection = glm::normalize(m_inputDirection);
    }
    
    // Handle jumping
    if (input.jump) {
        if (m_state.isGrounded && !m_state.isFlying && (currentTime - m_lastJumpTime) >= m_jumpCooldown) {
            // Apply massive upward impulse to overcome strong gravity
            applyJumpForce();
            m_lastJumpTime = currentTime;
            m_state.isJumping = true;
            
            // Set proper jump velocity for realistic jump height (~0.4m)
            btVector3 currentVel = m_playerBody->getLinearVelocity();
            currentVel.setY(30.0f); // Realistic jump velocity for 0.4m jump (scaled to 1 unit = 8.5cm)
            m_playerBody->setLinearVelocity(currentVel);
        }
    }
    
    // Apply movement forces
    applyMovementForces(input, deltaTime);
    
    // Handle slope sliding
    if (!m_state.isFlying) {
        handleSlope(deltaTime);
    }
    
    // Limit velocity
    limitVelocity();
    
    // Update state
    glm::vec3 velocity = getVelocity();
    m_state.velocity = velocity;
    m_state.speed = glm::length(glm::vec2(velocity.x, velocity.z));
    
    // Reset jumping state when grounded
    if (m_state.isGrounded && velocity.y <= 0.1f) {
        m_state.isJumping = false;
    }
}

void CEBulletPlayerController::updateGroundState()
{
    if (!m_playerBody) return;
    
    glm::vec3 playerPos = getPosition();
    // For capsule: bottom is at center - (height/2 + radius)
    glm::vec3 rayStart = playerPos - glm::vec3(0, m_capsuleHeight * 0.5f + m_capsuleRadius, 0);
    glm::vec3 rayEnd = rayStart - glm::vec3(0, m_groundCheckDistance, 0);
    
    // Perform raycast to check ground
    auto rayResult = m_physicsWorld->raycast(rayStart, rayEnd);
    
    if (rayResult.hasHit) {
        float distance = glm::distance(rayStart, rayResult.hitPoint);
        m_state.isGrounded = (distance <= m_groundCheckDistance + m_capsuleRadius); // Account for capsule radius
        m_state.groundNormal = rayResult.hitNormal;
        
        // Calculate slope angle
        float dotProduct = glm::dot(m_state.groundNormal, glm::vec3(0, 1, 0));
        m_state.groundSlope = glm::degrees(glm::acos(glm::clamp(dotProduct, 0.0f, 1.0f)));
        
        // Check if slope is too steep (very forgiving now)
        if (m_state.groundSlope > m_maxSlopeAngle) {
            m_state.isSliding = true;
        } else {
            m_state.isSliding = false;
        }
    } else {
        m_state.isGrounded = false;
        m_state.groundNormal = glm::vec3(0, 1, 0);
        m_state.groundSlope = 0.0f;
        m_state.isSliding = false;
    }
    
    // Also check velocity for additional ground detection - use higher threshold for high jump speeds
    btVector3 velocity = m_playerBody->getLinearVelocity();
    if (m_state.isGrounded && velocity.getY() > 50.0f) {
        // If moving up very fast, consider not grounded (jumping/falling) - increased threshold for high jumps
        m_state.isGrounded = false;
    }
}

void CEBulletPlayerController::applyMovementForces(const MovementInput& input, float deltaTime)
{
    if (!m_playerBody) return;
    
    btVector3 currentVelocity = m_playerBody->getLinearVelocity();
    glm::vec3 currentVel = btVector3ToGlm(currentVelocity);
    
    if (m_state.isFlying) {
        // Flying mode: smooth velocity interpolation
        glm::vec3 targetVelocity = m_inputDirection * m_walkSpeed;
        
        // Add vertical movement for flying
        if (input.jump) {
            targetVelocity.y = m_walkSpeed * 0.5f;
        }
        
        // Smooth interpolation to target velocity
        glm::vec3 newVelocity = glm::mix(currentVel, targetVelocity, deltaTime * 8.0f);
        m_playerBody->setLinearVelocity(glmToBtVector3(newVelocity));
        
    } else if (m_state.isGrounded) {
        // Ground movement: instant FPS-style direction changes
        glm::vec3 targetHorizontalVel = glm::vec3(0);
        
        if (glm::length(m_inputDirection) > 0.0f) {
            targetHorizontalVel = m_inputDirection * m_walkSpeed;
            
            // Simplified slope handling - just reduce speed on steep slopes
            if (m_state.groundSlope > 30.0f) {
                float slopeFactor = 1.0f - ((m_state.groundSlope - 30.0f) / 30.0f);
                slopeFactor = glm::clamp(slopeFactor, 0.3f, 1.0f);
                targetHorizontalVel *= slopeFactor;
            }
        }
        
        // Outdoor terrain movement: friction-based with responsive control
        if (glm::length(m_inputDirection) > 0.0f) {
            // Calculate desired velocity
            glm::vec3 targetVelocity = m_inputDirection * m_walkSpeed;
            glm::vec3 currentHorizontal = glm::vec3(currentVel.x, 0, currentVel.z);
            
            // Apply acceleration force when moving
            glm::vec3 velocityDifference = targetVelocity - currentHorizontal;
            glm::vec3 accelerationForce = velocityDifference * m_mass * 25.0f; // Very strong force to overcome friction
            m_playerBody->applyCentralForce(glmToBtVector3(accelerationForce));
            
            // Add small step-up force when moving and grounded
            if (m_state.isGrounded && currentVelocity.getY() < 1.0f) {
                m_playerBody->applyCentralForce(btVector3(0, m_mass * 2.0f, 0));
            }
        } else {
            // When no input, apply strong counter-force for immediate stopping
            glm::vec3 currentHorizontal = glm::vec3(currentVel.x, 0, currentVel.z);
            if (glm::length(currentHorizontal) > 0.1f) {
                glm::vec3 stopForce = -currentHorizontal * m_mass * 20.0f; // Very strong stopping force
                m_playerBody->applyCentralForce(glmToBtVector3(stopForce));
            }
        }
        
    } else {
        // Air movement: enhanced control for better aerial maneuverability
        if (glm::length(m_inputDirection) > 0.0f) {
            glm::vec3 airAccel = m_inputDirection * m_walkSpeed * m_airControl * deltaTime * 8.0f; // Much better air control
            glm::vec3 currentHorizontal = glm::vec3(currentVel.x, 0, currentVel.z);
            glm::vec3 newHorizontal = currentHorizontal + airAccel;
            
            // Limit air speed
            float maxAirSpeed = m_walkSpeed * 1.3f;
            if (glm::length(newHorizontal) > maxAirSpeed) {
                newHorizontal = glm::normalize(newHorizontal) * maxAirSpeed;
            }
            
            btVector3 newVelocity(newHorizontal.x, currentVelocity.getY(), newHorizontal.z);
            m_playerBody->setLinearVelocity(newVelocity);
        }
    }
}

void CEBulletPlayerController::applyJumpForce()
{
    if (!m_playerBody) return;
    
    // Apply upward impulse for jumping
    btVector3 jumpImpulse(0, m_jumpForce, 0);
    m_playerBody->applyCentralImpulse(jumpImpulse);
}

void CEBulletPlayerController::handleSlope(float deltaTime)
{
    if (!m_playerBody || !m_state.isGrounded) return;
    
    // If on a steep slope, apply sliding force
    if (m_state.isSliding) {
        // Calculate slide direction (down the slope)
        glm::vec3 slideDirection = glm::normalize(glm::vec3(m_state.groundNormal.x, 0, m_state.groundNormal.z));
        
        // Apply sliding force proportional to slope steepness
        float slideStrength = (m_state.groundSlope - m_maxSlopeAngle) / 45.0f; // Normalize to 0-1
        slideStrength = glm::clamp(slideStrength, 0.0f, 1.0f);
        
        btVector3 slideForce = glmToBtVector3(slideDirection * m_mass * 20.0f * slideStrength);
        m_playerBody->applyCentralForce(slideForce);
    }
}

void CEBulletPlayerController::limitVelocity()
{
    if (!m_playerBody) return;
    
    btVector3 velocity = m_playerBody->getLinearVelocity();
    
    if (!m_state.isFlying) {
        // Limit horizontal velocity when not flying
        float maxHorizontalSpeed = m_runSpeed * 1.2f; // Allow some overspeed
        btVector3 horizontalVel(velocity.getX(), 0, velocity.getZ());
        
        if (horizontalVel.length() > maxHorizontalSpeed) {
            horizontalVel = horizontalVel.normalized() * maxHorizontalSpeed;
            velocity.setX(horizontalVel.getX());
            velocity.setZ(horizontalVel.getZ());
        }
        
        // Limit fall speed
        if (velocity.getY() < -50.0f) {
            velocity.setY(-50.0f);
        }
    } else {
        // Limit all velocity when flying
        float maxFlyingSpeed = m_runSpeed * 1.5f;
        if (velocity.length() > maxFlyingSpeed) {
            velocity = velocity.normalized() * maxFlyingSpeed;
        }
    }
    
    m_playerBody->setLinearVelocity(velocity);
}

void CEBulletPlayerController::setFlying(bool flying)
{
    m_state.isFlying = flying;
    
    if (m_playerBody) {
        if (flying) {
            // Disable gravity for flying
            m_playerBody->setGravity(btVector3(0, 0, 0));
            // Increase damping for more controlled flying
            m_playerBody->setDamping(0.3f, 0.9f);
        } else {
            // Restore normal gravity and damping
            m_playerBody->setGravity(m_physicsWorld->getDynamicsWorld()->getGravity());
            m_playerBody->setDamping(0.1f, 0.9f);
        }
    }
}

glm::vec3 CEBulletPlayerController::getPosition() const
{
    if (!m_playerBody) return glm::vec3(0);
    
    btTransform transform;
    m_playerBody->getMotionState()->getWorldTransform(transform);
    return btVector3ToGlm(transform.getOrigin());
}

void CEBulletPlayerController::setPosition(const glm::vec3& position)
{
    if (!m_playerBody) return;
    
    btTransform transform;
    m_playerBody->getMotionState()->getWorldTransform(transform);
    transform.setOrigin(glmToBtVector3(position));
    
    m_playerBody->setWorldTransform(transform);
    m_playerBody->getMotionState()->setWorldTransform(transform);
    
    // Clear velocities when teleporting
    m_playerBody->setLinearVelocity(btVector3(0, 0, 0));
    m_playerBody->setAngularVelocity(btVector3(0, 0, 0));
}

glm::vec3 CEBulletPlayerController::getVelocity() const
{
    if (!m_playerBody) return glm::vec3(0);
    
    return btVector3ToGlm(m_playerBody->getLinearVelocity());
}

void CEBulletPlayerController::setVelocity(const glm::vec3& velocity)
{
    if (!m_playerBody) return;
    
    m_playerBody->setLinearVelocity(glmToBtVector3(velocity));
}

glm::vec3 CEBulletPlayerController::btVector3ToGlm(const btVector3& vec) const
{
    return glm::vec3(vec.getX(), vec.getY(), vec.getZ());
}

btVector3 CEBulletPlayerController::glmToBtVector3(const glm::vec3& vec) const
{
    return btVector3(vec.x, vec.y, vec.z);
}
