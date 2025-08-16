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

// Bullet Physics includes
#include <btBulletDynamicsCommon.h>

#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>
#include <unordered_map>

// Impact event logging is handled by the projectile manager


CEBulletProjectile::CEBulletProjectile(btDiscreteDynamicsWorld* world, 
                                     const glm::vec3& position, 
                                     const glm::vec3& velocity, 
                                     float damage,
                                     float maxLifetime)
    : m_rigidBody(nullptr)
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
{
    // Create small sphere for bullet
    btSphereShape* bulletShape = new btSphereShape(0.005f); // 5mm radius
    
    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(btVector3(position.x, position.y, position.z));
    
    float mass = 0.008f; // 8 grams for typical rifle bullet
    btVector3 localInertia;
    bulletShape->calculateLocalInertia(mass, localInertia);
    
    btDefaultMotionState* motionState = new btDefaultMotionState(transform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, bulletShape, localInertia);
    
    m_rigidBody = new btRigidBody(rbInfo);
    
    // Set initial velocity
    m_rigidBody->setLinearVelocity(btVector3(velocity.x, velocity.y, velocity.z));
    
    // Set bullet properties for realistic ballistics
    m_rigidBody->setRestitution(0.1f); // Low bounce
    m_rigidBody->setFriction(0.3f);
    
    // Enable Continuous Collision Detection (CCD) to prevent tunneling through objects
    m_rigidBody->setCcdMotionThreshold(0.001f); // Very aggressive CCD for small, fast projectiles
    m_rigidBody->setCcdSweptSphereRadius(0.02f); // Larger swept sphere to catch fast movement
    
    // Set user pointer for collision detection
    m_rigidBody->setUserPointer(this);
    
    // Add to physics world with collision filtering
    if (world) {
        // Use collision group 0 for projectiles
        short projectileGroup = 1 << 0;  // Projectile collision group  
        short projectileMask = (1 << 1) | (1 << 2); // Can collide with water (group 1) and objects (group 2)
        world->addRigidBody(m_rigidBody, projectileGroup, projectileMask);
        m_spawnTime = glfwGetTime(); // Set spawn time when added to world
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
    if (!m_rigidBody || !physics) {
        return false; // Projectile is invalid
    }
    
    // Get current position for logging
    btTransform transform;
    m_rigidBody->getMotionState()->getWorldTransform(transform);
    btVector3 pos = transform.getOrigin();
    glm::vec3 currentPos(pos.getX(), pos.getY(), pos.getZ());
    
    // Get current velocity for debugging
    btVector3 velocity = m_rigidBody->getLinearVelocity();
    float speed = velocity.length();
    
    // Projectile status tracking handled internally
    
    // Check for collisions using contact manifolds
    if (!m_hasImpacted && checkForCollisions(physics)) {
        // Impact detected - projectile manager will handle impact logging
        return true; // Impact detected
    }
    
    // No need for velocity debugging output - handled in GUI
    
    return false; // No impact yet
}

glm::vec3 CEBulletProjectile::getPosition() const
{
    if (!m_rigidBody) return glm::vec3(0, 0, 0);
    
    btTransform transform;
    m_rigidBody->getMotionState()->getWorldTransform(transform);
    btVector3 pos = transform.getOrigin();
    
    return glm::vec3(pos.getX(), pos.getY(), pos.getZ());
}

glm::vec3 CEBulletProjectile::getVelocity() const
{
    if (!m_rigidBody) return glm::vec3(0, 0, 0);
    
    btVector3 vel = m_rigidBody->getLinearVelocity();
    return glm::vec3(vel.getX(), vel.getY(), vel.getZ());
}

bool CEBulletProjectile::shouldDestroy(double currentTime) const
{
    // Destroy if impacted or exceeded maximum lifetime
    return m_hasImpacted || (currentTime - m_spawnTime) > m_maxLifetime;
}

bool CEBulletProjectile::checkForCollisions(CEPhysicsWorld* physics)
{
    if (!m_rigidBody || !physics || m_checkedForContacts || m_hasImpacted) {
        return false; // Don't check again if already impacted
    }
    
    // Get current position
    btTransform transform;
    m_rigidBody->getMotionState()->getWorldTransform(transform);
    btVector3 pos = transform.getOrigin();
    glm::vec3 currentPos(pos.getX(), pos.getY(), pos.getZ());
    
    // Method 1: Check if this projectile has contacts via manifolds
    bool hasContactManifolds = physics->hasContacts(m_rigidBody);
    
    // Debug: Log Method 1 attempts periodically
    static int method1Count = 0;
    method1Count++;
    if (method1Count % 120 == 0) { // Log every 120 checks to avoid spam
        std::cout << "🔍 Method 1 check " << method1Count << ": " << (hasContactManifolds ? "HAS CONTACTS" : "no contacts") << std::endl;
    }
    
    if (hasContactManifolds) {
        std::cout << "🚨 COLLISION Method 1: Contact manifold detected at position [" << currentPos.x << ", " << currentPos.y << ", " << currentPos.z << "]" << std::endl;
        
        m_hasImpacted = true;
        m_checkedForContacts = true;
        m_impactPoint = currentPos;
        m_impactDistance = glm::distance(m_impactPoint, m_spawnPosition);
        
        // Do a quick raycast to determine surface type
        glm::vec3 rayStart = m_impactPoint + glm::vec3(0, 1.0f, 0);
        glm::vec3 rayEnd = m_impactPoint - glm::vec3(0, 1.0f, 0);
        
        auto rayResult = physics->raycast(rayStart, rayEnd);
        if (rayResult.hasHit) {
            m_impactNormal = rayResult.hitNormal;
            
            // Use detailed object information
            switch (rayResult.objectInfo.type) {
                case CEPhysicsWorld::CollisionObjectType::TERRAIN:
                    m_impactSurfaceType = "terrain";
                    break;
                case CEPhysicsWorld::CollisionObjectType::WORLD_OBJECT:
                    m_impactSurfaceType = "object";
                    break;
                case CEPhysicsWorld::CollisionObjectType::WATER_PLANE:
                    m_impactSurfaceType = "water";
                    break;
                default:
                    m_impactSurfaceType = "unknown";
                    break;
            }
            
            std::cout << "   🎯 Target: " << rayResult.objectInfo.objectName << std::endl;
        } else {
            m_impactNormal = glm::vec3(0, 1, 0);
            m_impactSurfaceType = "terrain";
        }
        
        // Contact manifold impact detected
        return true;
    }
    
    // Method 2: Raycast ahead for water and object detection (with collision filtering)
    btVector3 velocity = m_rigidBody->getLinearVelocity();
    glm::vec3 velocityVec(velocity.getX(), velocity.getY(), velocity.getZ());
    
    if (glm::length(velocityVec) > 0.1f) {
        // Raycast ahead in movement direction
        glm::vec3 rayDirection = glm::normalize(velocityVec);
        float rayDistance = std::max(1000.0f, glm::length(velocityVec) * 0.1f); // Check 100ms ahead, minimum 1000 units
        glm::vec3 rayEnd = currentPos + rayDirection * rayDistance;
        
        auto rayResult = physics->raycast(currentPos, rayEnd);
        
        // Debug: Log raycast attempts for objects (periodically)
        static int raycastCount = 0;
        raycastCount++;
        if (raycastCount % 60 == 0) { // Log every 60 raycasts to avoid spam
            std::cout << "🔍 Raycast attempt " << raycastCount << " from [" << currentPos.x << "," << currentPos.y << "," << currentPos.z 
                      << "] to [" << rayEnd.x << "," << rayEnd.y << "," << rayEnd.z << "] distance=" << rayDistance 
                      << " - " << (rayResult.hasHit ? "HIT" : "MISS") << std::endl;
            if (rayResult.hasHit) {
                std::cout << "   🎯 Hit at [" << rayResult.hitPoint.x << "," << rayResult.hitPoint.y << "," << rayResult.hitPoint.z 
                          << "] type=" << (int)rayResult.objectInfo.type << " name=" << rayResult.objectInfo.objectName << std::endl;
            }
        }
        
        if (rayResult.hasHit) {
            // Register water and object collisions - terrain handled by Method 3
            if (rayResult.objectInfo.type == CEPhysicsWorld::CollisionObjectType::WATER_PLANE) {
                std::cout << "🚨 COLLISION Method 2: Water detected at position [" << rayResult.hitPoint.x << ", " << rayResult.hitPoint.y << ", " << rayResult.hitPoint.z << "]" << std::endl;
                
                m_hasImpacted = true;
                m_checkedForContacts = true;
                m_impactPoint = rayResult.hitPoint;
                m_impactNormal = rayResult.hitNormal;
                m_impactDistance = glm::distance(m_impactPoint, m_spawnPosition);
                m_impactSurfaceType = "water";
                
                return true;
            } else if (rayResult.objectInfo.type == CEPhysicsWorld::CollisionObjectType::WORLD_OBJECT) {
                std::cout << "🚨 COLLISION Method 2: Object AABB hit at [" << rayResult.hitPoint.x << ", " << rayResult.hitPoint.y << ", " << rayResult.hitPoint.z << "] - " << rayResult.objectInfo.objectName << std::endl;
                
                // TODO: Implement TBound narrowphase testing here
                // For now, just register the AABB hit
                m_hasImpacted = true;
                m_checkedForContacts = true;
                m_impactPoint = rayResult.hitPoint;
                m_impactNormal = rayResult.hitNormal;
                m_impactDistance = glm::distance(m_impactPoint, m_spawnPosition);
                m_impactSurfaceType = "object";
                
                // Store object information for GUI display
                m_impactObjectName = rayResult.objectInfo.objectName;
                m_impactObjectIndex = rayResult.objectInfo.objectIndex;
                m_impactInstanceIndex = rayResult.objectInfo.instanceIndex;
                
                return true;
            }
        }
    }
    
    // Method 3: Partitioned terrain collision
    if (physics->getMapFile()) {
        // Create terrain partition system (lightweight, could be cached)
        static std::unique_ptr<CETerrainPartition> terrainPartition = nullptr;
        if (!terrainPartition) {
            terrainPartition = std::make_unique<CETerrainPartition>(physics->getMapFile(), 32);
        }
        
        // Get partitions for current position
        auto partitionIndices = terrainPartition->getPartitionsForPosition(currentPos);
        
        if (partitionIndices.empty()) {
            // Out of bounds
            m_hasImpacted = true;
            m_checkedForContacts = true;
            m_impactPoint = currentPos;
            m_impactNormal = glm::vec3(0, 1, 0);
            m_impactDistance = glm::distance(m_impactPoint, m_spawnPosition);
            m_impactSurfaceType = "out-of-bounds";
            
            std::cout << "🚨 COLLISION Method 3: Out-of-bounds at [" << currentPos.x << ", " << currentPos.y << ", " << currentPos.z << "]" << std::endl;
            return true;
        }
        
        // Check for terrain collision within relevant partitions
        glm::vec3 hitPoint;
        float groundHeight;
        
        if (terrainPartition->checkHeightCollision(currentPos, partitionIndices, hitPoint, groundHeight)) {
            std::cout << "🚨 COLLISION Method 3: Partitioned terrain hit at [" << hitPoint.x << ", " << hitPoint.y << ", " << hitPoint.z 
                      << "] partitions:" << partitionIndices.size() << std::endl;
            
            m_hasImpacted = true;
            m_checkedForContacts = true;
            m_impactPoint = hitPoint;
            m_impactNormal = glm::vec3(0, 1, 0); // Simple upward normal for now
            m_impactDistance = glm::distance(m_impactPoint, m_spawnPosition);
            m_impactSurfaceType = "terrain";
            
            return true;
        }
    }
    
    return false;
}