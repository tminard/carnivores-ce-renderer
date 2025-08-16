//
//  CEBulletProjectile.cpp
//  CE Character Lab
//
//  Bullet Physics-based projectile for realistic ballistics
//

#include "CEBulletProjectile.h"
#include "CEPhysicsWorld.h"
#include "C2MapFile.h"

// Bullet Physics includes
#include <btBulletDynamicsCommon.h>

#include <GLFW/glfw3.h>
#include <iostream>


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
    
    // Enable Continuous Collision Detection (CCD) to prevent tunneling
    m_rigidBody->setCcdMotionThreshold(0.01f); // Very small threshold for high-speed objects
    m_rigidBody->setCcdSweptSphereRadius(0.002f); // Half the bullet radius for swept sphere
    
    // Set user pointer for collision detection
    m_rigidBody->setUserPointer(this);
    
    // Add to physics world
    if (world) {
        world->addRigidBody(m_rigidBody);
        m_spawnTime = glfwGetTime(); // Set spawn time when added to world
    }
    
    std::cout << "Bullet projectile created at [" << position.x << ", " << position.y << ", " << position.z << "]" << std::endl;
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
    
    // Debug: Log projectile state periodically
    static double lastLogTime = 0;
    if (currentTime - lastLogTime > 0.2) { // Log every 0.2 seconds for better tracking
        std::cout << "📊 Projectile Status: Pos[" << currentPos.x << ", " << currentPos.y << ", " << currentPos.z 
                  << "] Speed: " << speed << " m/s" << std::endl;
        
        // Check if projectile is below ground level (emergency fallback)
        if (currentPos.y < 0) {
            std::cout << "⚠️  Projectile below ground level! Y=" << currentPos.y << std::endl;
        }
        
        lastLogTime = currentTime;
    }
    
    // Check for collisions using contact manifolds
    if (!m_hasImpacted && checkForCollisions(physics)) {
        std::cout << "🎯 PROJECTILE IMPACT DETECTED!" << std::endl;
        std::cout << "   📍 Location: [" << m_impactPoint.x << ", " << m_impactPoint.y << ", " << m_impactPoint.z << "]" << std::endl;
        std::cout << "   🎲 Surface: " << m_impactSurfaceType << std::endl;
        std::cout << "   📏 Distance: " << m_impactDistance << "m" << std::endl;
        std::cout << "   ⚡ Damage: " << m_damage << std::endl;
        
        return true; // Impact detected
    }
    
    // Also check velocity-based fallback (for debugging)
    if (speed < 0.1f) {
        std::cout << "⚠️  Projectile velocity very low (" << speed << " m/s) but no collision detected" << std::endl;
    }
    
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
    if (!m_rigidBody || !physics || m_checkedForContacts) {
        return false;
    }
    
    // Get current position
    btTransform transform;
    m_rigidBody->getMotionState()->getWorldTransform(transform);
    btVector3 pos = transform.getOrigin();
    glm::vec3 currentPos(pos.getX(), pos.getY(), pos.getZ());
    
    // Method 1: Check if this projectile has contacts via manifolds
    if (physics->hasContacts(m_rigidBody)) {
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
        
        std::cout << "💥 CONTACT MANIFOLD IMPACT DETECTED!" << std::endl;
        return true;
    }
    
    // Method 2: Raycast fallback - check for collision in current movement path
    btVector3 velocity = m_rigidBody->getLinearVelocity();
    glm::vec3 velocityVec(velocity.getX(), velocity.getY(), velocity.getZ());
    
    if (glm::length(velocityVec) > 0.1f) {
        // Raycast ahead in movement direction
        glm::vec3 rayDirection = glm::normalize(velocityVec);
        float rayDistance = glm::length(velocityVec) * 0.02f; // Check 20ms ahead
        glm::vec3 rayEnd = currentPos + rayDirection * rayDistance;
        
        auto rayResult = physics->raycast(currentPos, rayEnd);
        if (rayResult.hasHit) {
            m_hasImpacted = true;
            m_checkedForContacts = true;
            m_impactPoint = rayResult.hitPoint;
            m_impactNormal = rayResult.hitNormal;
            m_impactDistance = glm::distance(m_impactPoint, m_spawnPosition);
            
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
            
            std::cout << "💥 RAYCAST IMPACT DETECTED!" << std::endl;
            std::cout << "   🎯 Target: " << rayResult.objectInfo.objectName << std::endl;
            
            // Add detailed target information
            if (rayResult.objectInfo.type == CEPhysicsWorld::CollisionObjectType::TERRAIN) {
                // Calculate terrain tile coordinates using actual tile size
                float tileSize = physics->getMapFile() ? physics->getMapFile()->getTileLength() : 256.0f;
                int tileX = static_cast<int>(m_impactPoint.x / tileSize);
                int tileZ = static_cast<int>(m_impactPoint.z / tileSize);
                std::cout << "   🗺️  Terrain Tile: [" << tileX << ", " << tileZ << "] (size: " << tileSize << ")" << std::endl;
                std::cout << "   🏔️  Ground Height: " << m_impactPoint.y << std::endl;
            } else if (rayResult.objectInfo.type == CEPhysicsWorld::CollisionObjectType::WORLD_OBJECT) {
                std::cout << "   🏗️  Object Index: " << rayResult.objectInfo.objectIndex << std::endl;
                std::cout << "   📦 Instance: " << rayResult.objectInfo.instanceIndex << std::endl;
                std::cout << "   📍 Object Position: [" << m_impactPoint.x << ", " << m_impactPoint.y << ", " << m_impactPoint.z << "]" << std::endl;
            } else if (rayResult.objectInfo.type == CEPhysicsWorld::CollisionObjectType::WATER_PLANE) {
                std::cout << "   🌊 Water Level: " << m_impactPoint.y << std::endl;
            }
            
            return true;
        }
    }
    
    // Method 3: Emergency ground check - if projectile is clearly underground
    if (currentPos.y < -100.0f) { // Well below any reasonable ground level
        m_hasImpacted = true;
        m_checkedForContacts = true;
        m_impactPoint = currentPos;
        m_impactNormal = glm::vec3(0, 1, 0);
        m_impactDistance = glm::distance(m_impactPoint, m_spawnPosition);
        m_impactSurfaceType = "terrain";
        
        std::cout << "💥 EMERGENCY GROUND IMPACT DETECTED! (Y=" << currentPos.y << ")" << std::endl;
        return true;
    }
    
    return false;
}