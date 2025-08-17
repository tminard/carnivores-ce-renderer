//
//  CEBulletProjectile.cpp
//  CE Character Lab
//
//  Bullet Physics-based projectile for realistic ballistics
//

#include "CEBulletProjectile.h"
#include "CEPhysicsWorld.h"
#include "CETerrainPartition.h"
#include "C2MapFile.h"
#include "CEWorldModel.h"

// Bullet Physics includes
#include <btBulletDynamicsCommon.h>

#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>
#include <unordered_map>

// Impact event logging is handled by the projectile manager

// Forward declaration for impact visualization
extern void addDebugSphere(const glm::vec3& position, float radius, const glm::vec3& color, const std::string& label);
extern void addProjectileTrajectory(const std::vector<glm::vec3>& trajectoryPoints, const std::string& surfaceType);

// FaceIntersectionQueue implementation
void FaceIntersectionQueue::addIntersection(const FaceIntersection& intersection) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_intersections.push_back(intersection);
}

std::vector<FaceIntersection> FaceIntersectionQueue::getAndClearIntersections() {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<FaceIntersection> result = std::move(m_intersections);
    m_intersections.clear();
    return result;
}

size_t FaceIntersectionQueue::size() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_intersections.size();
}


CEBulletProjectile::CEBulletProjectile(btDiscreteDynamicsWorld* world, 
                                     const glm::vec3& position, 
                                     const glm::vec3& velocity, 
                                     float damage,
                                     float maxLifetime)
    : m_rigidBody(nullptr)
    , m_dynamicsWorld(world)
    , m_damage(damage)
    , m_spawnTime(0.0) // Will be set when added to world
    , m_maxLifetime(maxLifetime)
    , m_spawnPosition(position)
    , m_hasImpacted(false)
    , m_impactPoint(0, 0, 0)
    , m_impactNormal(0, 1, 0)
    , m_impactSurfaceType("unknown")
    , m_impactDistance(0.0f)
    , m_impactObjectName("")
    , m_impactObjectIndex(-1)
    , m_impactInstanceIndex(-1)
    , m_checkedForContacts(false)
    , m_trajectoryRecordDistance(0.0625f) // Good density for visible trajectory lines
    , m_lastRecordedPosition(position)
    , m_lastFramePosition(position)
    , m_frameCount(0)
    , m_currentPosition(position)
    , m_currentVelocity(glm::normalize(velocity) * glm::length(velocity))
    , m_initialVelocity(glm::normalize(velocity) * glm::length(velocity))
{
    // Initialize trajectory with spawn position
    m_trajectoryPoints.push_back(position);
    // Create tiny bullet sphere to minimize bounce potential
    btSphereShape* bulletShape = new btSphereShape(0.001f); // Very small to minimize collision response
    
    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(btVector3(position.x, position.y, position.z));
    
    float mass = 0.001f; // Very light to minimize collision response force
    btVector3 localInertia;
    bulletShape->calculateLocalInertia(mass, localInertia);
    
    btDefaultMotionState* motionState = new btDefaultMotionState(transform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, bulletShape, localInertia);
    
    m_rigidBody = new btRigidBody(rbInfo);
    
    // Set initial velocity
    m_rigidBody->setLinearVelocity(btVector3(velocity.x, velocity.y, velocity.z));
    
    // Debug: Log initial projectile parameters (disabled for performance)
    // std::cout << "💥 Projectile created: pos[" << position.x << "," << position.y << "," << position.z 
    //           << "] vel[" << velocity.x << "," << velocity.y << "," << velocity.z 
    //           << "] speed=" << glm::length(velocity) << std::endl;
    
    // Set bullet properties to prevent any bouncing
    m_rigidBody->setRestitution(0.0f); // Absolutely no bounce
    m_rigidBody->setFriction(0.0f); // No friction to avoid sliding
    m_rigidBody->setRollingFriction(0.0f); // No rolling
    m_rigidBody->setSpinningFriction(0.0f); // No spinning
    
    // Light projectile with minimal collision response
    
    // Simplified bullet setup - no Bullet Physics integration needed
    m_rigidBody->setUserPointer(this);
    
    // Store world reference for manual physics simulation
    if (world) {
        m_spawnTime = glfwGetTime();
    }
    
    // Bullet projectile created and initialized
}

CEBulletProjectile::~CEBulletProjectile()
{
    // Note: Cleanup is handled by CEPhysicsWorld::removeRigidBody
    // to properly remove from physics world first
}

bool CEBulletProjectile::update(double currentTime, CEPhysicsWorld* physics)
{
    if (!physics || m_hasImpacted) {
        return m_hasImpacted;
    }
    
    m_frameCount++;
    
    // Simple physics simulation with gravity
    float deltaTime = 1.0f / 60.0f;
    static double lastTime = currentTime;
    if (m_frameCount > 1) {
        deltaTime = currentTime - lastTime;
    }
    lastTime = currentTime;
    
    // Apply gravity
    glm::vec3 gravity(0, -15.0f, 0);
    m_currentVelocity += gravity * deltaTime;
    
    // Calculate next position
    glm::vec3 nextPosition = m_currentPosition + m_currentVelocity * deltaTime;
    
    // Simple raycast collision detection
    auto rayResult = physics->raycast(m_currentPosition, nextPosition);
    
    if (rayResult.hasHit) {
        m_hasImpacted = true;
        m_impactPoint = rayResult.hitPoint;
        m_impactNormal = rayResult.hitNormal;
        m_impactDistance = glm::distance(m_impactPoint, m_spawnPosition);
        m_currentPosition = rayResult.hitPoint;
        
        // Set surface type
        switch (rayResult.objectInfo.type) {
            case CEPhysicsWorld::CollisionObjectType::TERRAIN:
            case CEPhysicsWorld::CollisionObjectType::HEIGHTFIELD_TERRAIN:
                m_impactSurfaceType = "terrain";
                break;
            case CEPhysicsWorld::CollisionObjectType::WORLD_OBJECT:
                m_impactSurfaceType = "object";
                m_impactObjectName = rayResult.objectInfo.objectName;
                m_impactObjectIndex = rayResult.objectInfo.objectIndex;
                m_impactInstanceIndex = rayResult.objectInfo.instanceIndex;
                break;
            case CEPhysicsWorld::CollisionObjectType::WATER_PLANE:
                m_impactSurfaceType = "water";
                break;
            default:
                m_impactSurfaceType = "unknown";
                break;
        }
        
        // Add visual feedback
        glm::vec3 impactColor = glm::vec3(1.0f, 0.5f, 0.0f);
        if (m_impactSurfaceType == "terrain") {
            impactColor = glm::vec3(0.0f, 1.0f, 0.5f);
        } else if (m_impactSurfaceType == "water") {
            impactColor = glm::vec3(0.0f, 0.8f, 1.0f);
        } else if (m_impactSurfaceType == "object") {
            impactColor = glm::vec3(1.0f, 0.0f, 0.5f);
        }
        
        addDebugSphere(m_impactPoint, 2.0f, impactColor, "impact");
        // Trajectory path rendering disabled - only final impact marker used
        
        return true;
    }
    
    // Update position
    m_currentPosition = nextPosition;
    
    // Track trajectory
    float distanceFromLastPoint = glm::distance(m_currentPosition, m_lastRecordedPosition);
    if (distanceFromLastPoint >= m_trajectoryRecordDistance) {
        m_trajectoryPoints.push_back(m_currentPosition);
        m_lastRecordedPosition = m_currentPosition;
        
        if (m_trajectoryPoints.size() > 1000) {
            m_trajectoryPoints.erase(m_trajectoryPoints.begin(), m_trajectoryPoints.begin() + 100);
        }
    }
    
    return false;
}

glm::vec3 CEBulletProjectile::getPosition() const
{
    return m_currentPosition;
}

glm::vec3 CEBulletProjectile::getVelocity() const
{
    return m_currentVelocity;
}

bool CEBulletProjectile::shouldDestroy(double currentTime) const
{
    // Destroy if impacted or exceeded maximum lifetime
    bool shouldDestroyDueToImpact = m_hasImpacted;
    bool shouldDestroyDueToTime = (currentTime - m_spawnTime) > m_maxLifetime;
    
    // Debug: Log destruction reasons for troubleshooting
    static int debugCount = 0;
    debugCount++;
    if (debugCount <= 5 && (shouldDestroyDueToImpact || shouldDestroyDueToTime)) {
        std::cout << "🗑️ Projectile " << debugCount << " shouldDestroy: impact=" << shouldDestroyDueToImpact 
                  << " timeout=" << shouldDestroyDueToTime << " age=" << (currentTime - m_spawnTime) 
                  << " maxLife=" << m_maxLifetime << std::endl;
    }
    
    return shouldDestroyDueToImpact || shouldDestroyDueToTime;
}



void CEBulletProjectile::stopProjectileMotion()
{
    if (m_rigidBody && m_dynamicsWorld) {
        // Remove the rigid body from the physics world immediately to prevent any bouncing
        m_dynamicsWorld->removeRigidBody(m_rigidBody);
        
        // Set the rigid body position to the exact impact point
        btTransform transform;
        transform.setIdentity();
        transform.setOrigin(btVector3(m_impactPoint.x, m_impactPoint.y, m_impactPoint.z));
        m_rigidBody->setWorldTransform(transform);
        
        // Stop all motion
        m_rigidBody->setLinearVelocity(btVector3(0, 0, 0));
        m_rigidBody->setAngularVelocity(btVector3(0, 0, 0));
    }
}
