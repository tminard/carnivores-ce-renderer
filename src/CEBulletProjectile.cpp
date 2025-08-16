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

// Forward declaration for trace line system
extern void addProjectileTraceLine(const glm::vec3& startPoint, const glm::vec3& endPoint, const std::string& surfaceType);


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
        // Use CEPhysicsWorld collision groups
        short projectileGroup = 1 << 0;  // PROJECTILE_GROUP
        short projectileMask = (1 << 1) | (1 << 2) | (1 << 3); // Can collide with TERRAIN_GROUP, OBJECT_GROUP, WATER_GROUP
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
                case CEPhysicsWorld::CollisionObjectType::HEIGHTFIELD_TERRAIN:
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
        
        // Add trace line from spawn to impact with correct surface type
        addProjectileTraceLine(m_spawnPosition, m_impactPoint, m_impactSurfaceType);
        
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
                
                // Add trace line from spawn to impact
                addProjectileTraceLine(m_spawnPosition, m_impactPoint, m_impactSurfaceType);
                
                return true;
            } else if (rayResult.objectInfo.type == CEPhysicsWorld::CollisionObjectType::WORLD_OBJECT) {
                std::cout << "🎯 AABB Hit: " << rayResult.objectInfo.objectName << " at [" << rayResult.hitPoint.x << ", " << rayResult.hitPoint.y << ", " << rayResult.hitPoint.z << "]" << std::endl;
                
                // Check if this object should block projectiles based on visibility flags
                if (rayResult.objectInfo.worldModel) {
                    TObjInfo* objInfo = rayResult.objectInfo.worldModel->getObjectInfo();
                    if (objInfo) {
                        // Objects with objectNOLIGHT flag are invisible bounding boxes - projectiles pass through
                        // This allows dinosaurs/players to hide behind solid objects but not behind invisible bounds
                        bool isInvisibleBoundingBox = (objInfo->flags & objectNOLIGHT) != 0;
                        
                        if (isInvisibleBoundingBox) {
                            std::cout << "🚶 Projectile passes through invisible bounding box: " << rayResult.objectInfo.objectName 
                                      << " (flags: 0x" << std::hex << objInfo->flags << std::dec << ")" << std::endl;
                            // Don't register as impact - let projectile continue to hit what's behind it
                            return false;
                        } else {
                            std::cout << "🛡️ Object is solid (flags: 0x" << std::hex << objInfo->flags << std::dec << "): " << rayResult.objectInfo.objectName << std::endl;
                        }
                    }
                }
                
                // Register solid object hit
                std::cout << "🚨 COLLISION Method 2: Solid object hit at [" << rayResult.hitPoint.x << ", " << rayResult.hitPoint.y << ", " << rayResult.hitPoint.z << "] - " << rayResult.objectInfo.objectName << std::endl;
                
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
                
                // Add trace line from spawn to impact
                addProjectileTraceLine(m_spawnPosition, m_impactPoint, m_impactSurfaceType);
                
                return true;
            }
        }
    }
    
    // Method 3: Terrain collision - now handled by Bullet heightfield if available
    if (physics->getMapFile()) {
        // Check if we have Bullet heightfield terrain available
        if (physics->getHeightfieldTerrain()) {
            // Bullet heightfield terrain is active - collision handled automatically by contact manifolds
            // No manual terrain collision needed - return false to rely on Method 1 (contact manifolds)
            return false;
        }
        
        // Fallback: Use legacy terrain partition system for manual collision detection
        static std::unique_ptr<CETerrainPartition> terrainPartition = nullptr;
        if (!terrainPartition) {
            terrainPartition = std::make_unique<CETerrainPartition>(physics->getMapFile(), 32);
        }
        
        // Get previous position to create a ray segment
        static std::unordered_map<btRigidBody*, glm::vec3> previousPositions;
        glm::vec3 prevPos = previousPositions.count(m_rigidBody) ? previousPositions[m_rigidBody] : m_spawnPosition;
        previousPositions[m_rigidBody] = currentPos;
        
        // Layer 1: Get tile candidates along ray path (max 16 tiles)
        auto tileCandidates = terrainPartition->getTileCandidatesForRay(prevPos, currentPos);
        
        // Debug: Log ray and tile info
        static int debugCount = 0;
        debugCount++;
        if (debugCount <= 3) { // Only log first few attempts
            std::cout << "🔍 Ray from [" << prevPos.x << "," << prevPos.y << "," << prevPos.z 
                      << "] to [" << currentPos.x << "," << currentPos.y << "," << currentPos.z 
                      << "] found " << tileCandidates.size() << " tile candidates" << std::endl;
        }
        
        if (tileCandidates.empty()) {
            // Check if we're actually out of bounds or if there's an issue with tile detection
            C2MapFile* mapFile = physics->getMapFile();
            float tileSize = mapFile->getTileLength();
            int tileX = (int)std::floor(currentPos.x / tileSize);
            int tileZ = (int)std::floor(currentPos.z / tileSize);
            
            bool actuallyOOB = (tileX < 0 || tileX >= mapFile->getWidth() || 
                               tileZ < 0 || tileZ >= mapFile->getHeight());
            
            if (actuallyOOB) {
                m_hasImpacted = true;
                m_checkedForContacts = true;
                m_impactPoint = currentPos;
                m_impactNormal = glm::vec3(0, 1, 0);
                m_impactDistance = glm::distance(m_impactPoint, m_spawnPosition);
                m_impactSurfaceType = "out-of-bounds";
                
                // Add trace line from spawn to impact
                addProjectileTraceLine(m_spawnPosition, m_impactPoint, m_impactSurfaceType);
                
                std::cout << "🚨 COLLISION Method 3: Actually out-of-bounds at tile[" << tileX << "," << tileZ 
                          << "] map size[" << mapFile->getWidth() << "," << mapFile->getHeight() << "]" << std::endl;
                return true;
            } else {
                // Not actually OOB, just no tile candidates found - fall back to simple check
                std::cout << "⚠️ No tile candidates but position is valid at tile[" << tileX << "," << tileZ << "]" << std::endl;
            }
        }
        
        // Layer 2: Test each tile candidate for precise ray-ground intersection
        for (const auto& candidate : tileCandidates) {
            // Quick height bounds check first
            if (std::min(prevPos.y, currentPos.y) > candidate.maxHeight) {
                continue; // Ray is entirely above this tile's max height
            }
            if (std::max(prevPos.y, currentPos.y) < candidate.minHeight) {
                continue; // Ray is entirely below this tile's min height  
            }
            
            // Precise ray-ground intersection for this tile
            float groundHeight = physics->getMapFile()->getPlaceGroundHeight(candidate.tileX, candidate.tileZ);
            
            // Check if ray segment crosses the ground plane
            bool prevAbove = prevPos.y > groundHeight;
            bool currentBelow = currentPos.y <= groundHeight;
            
            if (prevAbove && currentBelow) {
                // Calculate exact intersection point
                float t = (prevPos.y - groundHeight) / (prevPos.y - currentPos.y);
                glm::vec3 intersectionPoint = prevPos + t * (currentPos - prevPos);
                intersectionPoint.y = groundHeight; // Clamp to ground height
                
                std::cout << "🚨 COLLISION Method 3: Ray-ground intersection at tile[" << candidate.tileX << "," << candidate.tileZ 
                          << "] ground=" << groundHeight << " prev=" << prevPos.y << " curr=" << currentPos.y << std::endl;
                
                m_hasImpacted = true;
                m_checkedForContacts = true;
                m_impactPoint = intersectionPoint;
                m_impactNormal = glm::vec3(0, 1, 0); // Simple upward normal for now
                m_impactDistance = glm::distance(m_impactPoint, m_spawnPosition);
                m_impactSurfaceType = "terrain";
                
                // Add trace line from spawn to impact
                addProjectileTraceLine(m_spawnPosition, m_impactPoint, m_impactSurfaceType);
                
                return true;
            }
        }
        
        // Fallback: If no tile candidates but position is valid, use simple position check
        if (tileCandidates.empty()) {
            C2MapFile* mapFile = physics->getMapFile();
            float tileSize = mapFile->getTileLength();
            int tileX = (int)std::floor(currentPos.x / tileSize);
            int tileZ = (int)std::floor(currentPos.z / tileSize);
            
            if (tileX >= 0 && tileX < mapFile->getWidth() && 
                tileZ >= 0 && tileZ < mapFile->getHeight()) {
                
                float groundHeight = mapFile->getPlaceGroundHeight(tileX, tileZ);
                
                // Check if ray crosses ground plane  
                bool prevAbove = prevPos.y > groundHeight;
                bool currentBelow = currentPos.y <= groundHeight;
                
                if (prevAbove && currentBelow) {
                    float t = (prevPos.y - groundHeight) / (prevPos.y - currentPos.y);
                    glm::vec3 intersectionPoint = prevPos + t * (currentPos - prevPos);
                    intersectionPoint.y = groundHeight;
                    
                    std::cout << "🚨 COLLISION Method 3: Fallback terrain hit at tile[" << tileX << "," << tileZ 
                              << "] ground=" << groundHeight << std::endl;
                    
                    m_hasImpacted = true;
                    m_checkedForContacts = true;
                    m_impactPoint = intersectionPoint;
                    m_impactNormal = glm::vec3(0, 1, 0);
                    m_impactDistance = glm::distance(m_impactPoint, m_spawnPosition);
                    m_impactSurfaceType = "terrain";
                    
                    // Add trace line from spawn to impact
                    addProjectileTraceLine(m_spawnPosition, m_impactPoint, m_impactSurfaceType);
                    
                    return true;
                }
            }
        }
    }
    
    return false;
}