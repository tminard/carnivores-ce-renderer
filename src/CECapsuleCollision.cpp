//
//  CECapsuleCollision.cpp
//  CarnivoresRenderer
//
//  Bullet Physics implementation of capsule collision detection
//

#include "CECapsuleCollision.h"
#include <btBulletDynamicsCommon.h>
#include <iostream>

CECapsuleCollision::CECapsuleCollision(btDiscreteDynamicsWorld* dynamicsWorld, 
                                       float radius, 
                                       float height, 
                                       const glm::vec3& initialPosition)
    : m_dynamicsWorld(dynamicsWorld)
    , m_capsuleShape(nullptr)
    , m_capsuleBody(nullptr)
    , m_motionState(nullptr)
    , m_position(initialPosition)
    , m_radius(radius)
    , m_height(height)
    , m_enabled(true)
    , m_bodyCreated(false)
    , m_isShuttingDown(false)
{
    if (!m_dynamicsWorld) {
        std::cerr << "CECapsuleCollision: Warning - null dynamics world provided" << std::endl;
        m_enabled = false;
        return;
    }
    
    createCapsuleBody();
}

CECapsuleCollision::~CECapsuleCollision()
{
    m_isShuttingDown = true;
    destroyCapsuleBody();
}

void CECapsuleCollision::createCapsuleBody()
{
    if (!m_dynamicsWorld || !m_enabled) return;
    
    // Clean up existing body if any
    destroyCapsuleBody();
    
    // Create capsule collision shape
    m_capsuleShape = new btCapsuleShape(m_radius, m_height);
    
    // Set initial transform
    btTransform startTransform;
    startTransform.setIdentity();
    startTransform.setOrigin(glmToBtVector3(m_position));
    
    // Create motion state
    m_motionState = new btDefaultMotionState(startTransform);
    
    // Create kinematic rigid body (zero mass = kinematic)
    // Kinematic bodies are moved manually and don't respond to forces
    btVector3 localInertia(0, 0, 0);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(0.0f, m_motionState, m_capsuleShape, localInertia);
    
    // Set properties for smooth movement
    rbInfo.m_friction = 0.0f;        // No friction for kinematic body
    rbInfo.m_restitution = 0.0f;     // No bouncing
    rbInfo.m_linearDamping = 0.0f;   // No damping
    rbInfo.m_angularDamping = 0.0f;  // No angular damping
    
    m_capsuleBody = new btRigidBody(rbInfo);
    
    // Configure as kinematic object
    m_capsuleBody->setCollisionFlags(m_capsuleBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
    m_capsuleBody->setActivationState(DISABLE_DEACTIVATION);
    
    // Prevent rotation
    m_capsuleBody->setAngularFactor(btVector3(0, 0, 0));
    
    // Add to physics world with proper collision filtering
    m_dynamicsWorld->addRigidBody(m_capsuleBody, PLAYER_COLLISION_GROUP, PLAYER_COLLISION_MASK);
    
    m_bodyCreated = true;  // Mark as successfully created
    
    std::cout << "CECapsuleCollision: Created capsule body (radius=" << m_radius 
              << ", height=" << m_height << ") at position " << m_position.x 
              << ", " << m_position.y << ", " << m_position.z << std::endl;
}

void CECapsuleCollision::destroyCapsuleBody()
{
    // Only attempt cleanup if we successfully created the body
    if (!m_bodyCreated) {
        return;
    }
    
    // If we're shutting down (destructor), be extra careful
    if (m_isShuttingDown) {
        // During shutdown, just set everything to null without calling methods
        // The physics world destructor will handle cleanup
        m_capsuleBody = nullptr;
        m_motionState = nullptr; 
        m_capsuleShape = nullptr;
        m_bodyCreated = false;
        return;
    }
    
    // Normal cleanup during runtime
    if (m_dynamicsWorld && m_capsuleBody) {
        try {
            // Check if the body is actually in the world before trying to remove it
            btCollisionObjectArray& objectArray = m_dynamicsWorld->getCollisionObjectArray();
            bool bodyInWorld = false;
            for (int i = 0; i < objectArray.size(); i++) {
                if (objectArray[i] == m_capsuleBody) {
                    bodyInWorld = true;
                    break;
                }
            }
            
            if (bodyInWorld) {
                m_dynamicsWorld->removeRigidBody(m_capsuleBody);
            }
        } catch (...) {
            // If world access fails, just continue with cleanup
            std::cerr << "Warning: Failed to remove capsule body from physics world" << std::endl;
        }
    }
    
    // Clean up Bullet objects in correct order
    if (m_capsuleBody) {
        try {
            delete m_capsuleBody;
        } catch (...) {
            std::cerr << "Warning: Failed to delete capsule body safely" << std::endl;
        }
        m_capsuleBody = nullptr;
    }
    
    if (m_motionState) {
        try {
            delete m_motionState;
        } catch (...) {
            std::cerr << "Warning: Failed to delete motion state safely" << std::endl;
        }
        m_motionState = nullptr;
    }
    
    if (m_capsuleShape) {
        try {
            delete m_capsuleShape;
        } catch (...) {
            std::cerr << "Warning: Failed to delete capsule shape safely" << std::endl;
        }
        m_capsuleShape = nullptr;
    }
    
    m_bodyCreated = false;  // Mark as no longer created
}

void CECapsuleCollision::updatePosition(const glm::vec3& position)
{
    m_position = position;
    
    if (!m_enabled || !m_capsuleBody) return;
    
    // Update the kinematic body's position
    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(glmToBtVector3(position));
    
    m_capsuleBody->setWorldTransform(transform);
    m_capsuleBody->getMotionState()->setWorldTransform(transform);
    m_capsuleBody->activate(true);
}

bool CECapsuleCollision::checkMovement(const glm::vec3& from, const glm::vec3& to) const
{
    if (!m_enabled || !m_dynamicsWorld) {
        return true; // Allow movement if disabled
    }
    
    // Use convex sweep test to check if movement is clear
    btTransform fromTransform, toTransform;
    fromTransform.setIdentity();
    toTransform.setIdentity();
    fromTransform.setOrigin(glmToBtVector3(from));
    toTransform.setOrigin(glmToBtVector3(to));
    
    // Create a temporary capsule shape for the sweep test
    btCapsuleShape testShape(m_radius, m_height);
    
    // Perform convex sweep test
    btCollisionWorld::ClosestConvexResultCallback sweepCallback(
        glmToBtVector3(from), 
        glmToBtVector3(to)
    );
    
    // Set collision filtering to match our capsule body
    sweepCallback.m_collisionFilterGroup = PLAYER_COLLISION_GROUP;
    sweepCallback.m_collisionFilterMask = PLAYER_COLLISION_MASK;
    
    m_dynamicsWorld->convexSweepTest(&testShape, fromTransform, toTransform, sweepCallback);
    
    // Return true if no collision (movement is allowed)
    return !sweepCallback.hasHit();
}

void CECapsuleCollision::setDimensions(float radius, float height)
{
    if (m_radius == radius && m_height == height) {
        return; // No change needed
    }
    
    m_radius = radius;
    m_height = height;
    
    if (m_enabled) {
        // Recreate the capsule body with new dimensions
        glm::vec3 currentPos = m_position;
        createCapsuleBody();
        updatePosition(currentPos);
    }
}

glm::vec3 CECapsuleCollision::getPosition() const
{
    return m_position;
}

void CECapsuleCollision::setEnabled(bool enabled)
{
    if (m_enabled == enabled) return;
    
    m_enabled = enabled;
    
    if (enabled) {
        createCapsuleBody();
        updatePosition(m_position);
    } else {
        destroyCapsuleBody();
    }
}

bool CECapsuleCollision::isEnabled() const
{
    return m_enabled;
}

bool CECapsuleCollision::hasContacts() const
{
    if (!m_enabled || !m_capsuleBody || !m_dynamicsWorld) {
        return false;
    }
    
    // Check collision manifolds to see if we have any contacts
    int numManifolds = m_dynamicsWorld->getDispatcher()->getNumManifolds();
    for (int i = 0; i < numManifolds; i++) {
        btPersistentManifold* contactManifold = m_dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
        
        const btCollisionObject* obA = contactManifold->getBody0();
        const btCollisionObject* obB = contactManifold->getBody1();
        
        // Check if our capsule body is involved in this contact
        if (obA == m_capsuleBody || obB == m_capsuleBody) {
            int numContacts = contactManifold->getNumContacts();
            if (numContacts > 0) {
                return true;
            }
        }
    }
    
    return false;
}

// Helper methods for coordinate conversion
glm::vec3 CECapsuleCollision::btVector3ToGlm(const btVector3& vec) const
{
    return glm::vec3(vec.getX(), vec.getY(), vec.getZ());
}

btVector3 CECapsuleCollision::glmToBtVector3(const glm::vec3& vec) const
{
    return btVector3(vec.x, vec.y, vec.z);
}