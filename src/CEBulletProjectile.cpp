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
#include <unordered_map>

// Forward declaration for impact event logging  
extern void addImpactEvent(const glm::vec3& location, const std::string& surfaceType, float distance, float damage, const std::string& impactType);


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
    
    // Enable Continuous Collision Detection (CCD) with more relaxed settings for performance
    m_rigidBody->setCcdMotionThreshold(0.1f); // Less aggressive CCD for better performance
    m_rigidBody->setCcdSweptSphereRadius(0.01f); // Larger swept sphere, less computation
    
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
        // Log impact to GUI instead of console
        addImpactEvent(m_impactPoint, m_impactSurfaceType, m_impactDistance, m_damage, "Collision Detection");
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
        float rayDistance = glm::length(velocityVec) * 0.02f; // Check 20ms ahead
        glm::vec3 rayEnd = currentPos + rayDirection * rayDistance;
        
        auto rayResult = physics->raycast(currentPos, rayEnd);
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
                
                return true;
            }
        }
    }
    
    // Method 3: Efficient ground height check (replaces expensive terrain collision)
    if (physics->getMapFile()) {
        C2MapFile* mapFile = physics->getMapFile();
        float tileSize = mapFile->getTileLength();
        
        // Convert world coordinates to tile coordinates (same as getWorldPosition())
        int tileX = (int)floorf(currentPos.x / tileSize);
        int tileZ = (int)floorf(currentPos.z / tileSize);
        
        // Check if projectile is outside map bounds (in tile coordinates) - terminate it
        if (tileX < 0 || tileX >= mapFile->getWidth() || 
            tileZ < 0 || tileZ >= mapFile->getHeight()) {
            m_hasImpacted = true;
            m_checkedForContacts = true;
            m_impactPoint = currentPos;
            m_impactNormal = glm::vec3(0, 1, 0);
            m_impactDistance = glm::distance(m_impactPoint, m_spawnPosition);
            m_impactSurfaceType = "out-of-bounds";
            
            // Out of bounds - terminate projectile
            return true;
        }
        
        // Safe to check ground height now with tile coordinates
        float groundHeight = mapFile->getPlaceGroundHeight(tileX, tileZ);
        
        // Debug: Check if this is the first height check for this projectile
        static bool firstCheck = true;
        if (firstCheck) {
            std::cout << "🔍 First projectile height check:" << std::endl;
            std::cout << "   Projectile Y: " << currentPos.y << std::endl;
            std::cout << "   Ground Height: " << groundHeight << std::endl;
            std::cout << "   Tile Size: " << tileSize << std::endl;
            std::cout << "   Tile Coords: [" << tileX << ", " << tileZ << "]" << std::endl;
            firstCheck = false;
        }
        
        // Perform ray-terrain intersection for precise collision detection
        // Get previous position to create a ray segment
        static std::unordered_map<btRigidBody*, glm::vec3> previousPositions;
        glm::vec3 prevPos = previousPositions.count(m_rigidBody) ? previousPositions[m_rigidBody] : m_spawnPosition;
        previousPositions[m_rigidBody] = currentPos;
        
        // Perform ray-terrain intersection along the projectile path
        glm::vec3 rayStart = prevPos;
        glm::vec3 rayEnd = currentPos;
        glm::vec3 rayDir = rayEnd - rayStart;
        float rayLength = glm::length(rayDir);
        
        if (rayLength > 0.001f) { // Only check if projectile has moved
            rayDir = glm::normalize(rayDir);
            
            // Step along the ray checking for terrain intersection
            int steps = std::max(1, (int)(rayLength / 10.0f)); // Check every 10 units
            for (int i = 0; i <= steps; i++) {
                float t = (float)i / (float)steps;
                glm::vec3 testPos = rayStart + rayDir * (rayLength * t);
                
                // Get terrain height at test position
                int testTileX = (int)floorf(testPos.x / tileSize);
                int testTileZ = (int)floorf(testPos.z / tileSize);
                
                // Check bounds
                if (testTileX < 0 || testTileX >= mapFile->getWidth() || 
                    testTileZ < 0 || testTileZ >= mapFile->getHeight()) {
                    continue;
                }
                
                float testGroundHeight = mapFile->getPlaceGroundHeight(testTileX, testTileZ);
                
                // Bilinear interpolation for accurate terrain height
                float fracX = (testPos.x / tileSize) - testTileX;
                float fracZ = (testPos.z / tileSize) - testTileZ;
                
                if (testTileX < mapFile->getWidth() - 1 && testTileZ < mapFile->getHeight() - 1) {
                    float h00 = testGroundHeight;
                    float h10 = mapFile->getPlaceGroundHeight(testTileX + 1, testTileZ);
                    float h01 = mapFile->getPlaceGroundHeight(testTileX, testTileZ + 1);
                    float h11 = mapFile->getPlaceGroundHeight(testTileX + 1, testTileZ + 1);
                    
                    float h0 = h00 * (1.0f - fracX) + h10 * fracX;
                    float h1 = h01 * (1.0f - fracX) + h11 * fracX;
                    testGroundHeight = h0 * (1.0f - fracZ) + h1 * fracZ;
                }
                
                // Check if ray intersects terrain
                if (testPos.y <= testGroundHeight) {
                    std::cout << "🚨 COLLISION Method 3: Ray-terrain intersection at [" << testPos.x << ", " << testGroundHeight << ", " << testPos.z << "]" << std::endl;
                    
                    m_hasImpacted = true;
                    m_checkedForContacts = true;
                    m_impactPoint = glm::vec3(testPos.x, testGroundHeight, testPos.z);
                    
                    // Calculate surface normal
                    float normalX = 0.0f, normalZ = 0.0f;
                    if (testTileX > 0 && testTileX < mapFile->getWidth() - 1) {
                        float heightLeft = mapFile->getPlaceGroundHeight(testTileX - 1, testTileZ);
                        float heightRight = mapFile->getPlaceGroundHeight(testTileX + 1, testTileZ);
                        normalX = (heightLeft - heightRight) / (2.0f * tileSize);
                    }
                    if (testTileZ > 0 && testTileZ < mapFile->getHeight() - 1) {
                        float heightForward = mapFile->getPlaceGroundHeight(testTileX, testTileZ - 1);
                        float heightBackward = mapFile->getPlaceGroundHeight(testTileX, testTileZ + 1);
                        normalZ = (heightForward - heightBackward) / (2.0f * tileSize);
                    }
                    
                    m_impactNormal = glm::normalize(glm::vec3(normalX, 1.0f, normalZ));
                    m_impactDistance = glm::distance(m_impactPoint, m_spawnPosition);
                    m_impactSurfaceType = "terrain";
                    
                    return true;
                }
            }
        }
    }
    
    return false;
}