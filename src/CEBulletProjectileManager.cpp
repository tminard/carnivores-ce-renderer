//
//  CEBulletProjectileManager.cpp
//  CE Character Lab
//
//  Bullet Physics-based projectile system manager
//

#include "CEBulletProjectileManager.h"
#include "CEBulletProjectile.h"
#include "CEPhysicsWorld.h"
#include "C2MapFile.h"
#include "C2MapRscFile.h"
#include "LocalAudioManager.hpp"
#include "CEAudioSource.hpp"
#include "dependency/libAF/af2-sound.h"

#include <iostream>
#include <algorithm>

// Forward declarations for impact event logging  
extern void addImpactEvent(const glm::vec3& location, const std::string& surfaceType, float distance, float damage, const std::string& impactType);
extern void addImpactEvent(const glm::vec3& location, const std::string& surfaceType, float distance, float damage, const std::string& impactType, const std::string& objectName, int objectIndex, int instanceIndex);
extern void addFaceIntersectionEvent(const glm::vec3& location, const std::string& surfaceType, float distance, const glm::vec3& normal, const glm::vec3& direction, int tileX, int tileZ, int faceIndex, const std::string& objectName, int objectIndex, int instanceIndex);
extern void addDebugSphere(const glm::vec3& position, float radius, const glm::vec3& color, const std::string& label);

using libAF2::Sound;

CEBulletProjectileManager::CEBulletProjectileManager(C2MapFile* map, C2MapRscFile* mapRsc, LocalAudioManager* audioManager)
    : m_map(map), m_mapRsc(mapRsc), m_audioManager(audioManager)
{
    // Initialize physics world with terrain, objects, and water
    m_physicsWorld = std::make_unique<CEPhysicsWorld>(map, mapRsc);
    
    // Bullet Physics projectile system initialized
}

CEBulletProjectileManager::~CEBulletProjectileManager()
{
    // Clean up all active projectiles
    for (auto& projectile : m_activeProjectiles) {
        if (projectile && projectile->getRigidBody()) {
            m_physicsWorld->removeRigidBody(projectile->getRigidBody());
        }
    }
    m_activeProjectiles.clear();
}

void CEBulletProjectileManager::spawnProjectile(const glm::vec3& origin, const glm::vec3& direction, 
                                               float muzzleVelocity, float damage, const std::string& type)
{
    // Spawning ballistic projectile
    
    // Calculate initial velocity vector
    glm::vec3 velocity = glm::normalize(direction) * muzzleVelocity;
    
    // Create new bullet projectile
    auto projectile = std::make_unique<CEBulletProjectile>(
        m_physicsWorld->getDynamicsWorld(),
        origin,
        velocity,
        damage
    );
    
    m_activeProjectiles.push_back(std::move(projectile));
    
    // Active projectiles count available via getActiveProjectileCount()
}

void CEBulletProjectileManager::update(double currentTime, double deltaTime)
{
    // Step the physics simulation
    m_physicsWorld->stepSimulation(static_cast<float>(deltaTime));
    
    // Update all projectiles and check for impacts
    for (auto it = m_activeProjectiles.begin(); it != m_activeProjectiles.end();) {
        auto& projectile = *it;
        
        // Update projectile and check for impact
        bool impacted = projectile->update(currentTime, m_physicsWorld.get());
        
        if (impacted) {
            handleImpact(*projectile);
        }
        
        // Process face intersections for effects (even if not impacted yet)
        processFaceIntersections(*projectile);
        
        // Remove projectiles that should be destroyed
        if (projectile->shouldDestroy(currentTime)) {
            // Remove from physics world first
            if (projectile->getRigidBody()) {
                m_physicsWorld->removeRigidBody(projectile->getRigidBody());
            }
            
            // Debug: Log projectile cleanup
            static int cleanupCount = 0;
            cleanupCount++;
            if (cleanupCount <= 5) {
                std::cout << "🗑️ Cleanup projectile " << cleanupCount << " - active count: " 
                          << m_activeProjectiles.size() << " -> " << (m_activeProjectiles.size() - 1) << std::endl;
            }
            
            it = m_activeProjectiles.erase(it);
        } else {
            ++it;
        }
    }
}

void CEBulletProjectileManager::handleImpact(const CEBulletProjectile& projectile)
{
    if (!projectile.hasImpacted()) return;
    
    const glm::vec3& hitPoint = projectile.getImpactPoint();
    const std::string& surfaceType = projectile.getImpactSurfaceType();
    float distance = projectile.getImpactDistance();
    float damage = projectile.getDamage();
    
    // Log impact to GUI with object information if available
    if (surfaceType == "object" && !projectile.getImpactObjectName().empty()) {
        addImpactEvent(hitPoint, surfaceType, distance, damage, "Bullet Impact", 
                      projectile.getImpactObjectName(), 
                      projectile.getImpactObjectIndex(), 
                      projectile.getImpactInstanceIndex());
    } else {
        addImpactEvent(hitPoint, surfaceType, distance, damage, "Bullet Impact");
    }
    
    // Play impact audio with correct surface type
    playImpactAudio(hitPoint, surfaceType);
    
    // TODO: Add visual impact effects here
    // TODO: Add damage calculation for hit objects/characters
}

void CEBulletProjectileManager::playImpactAudio(const glm::vec3& position, const std::string& surfaceType)
{
    if (!m_audioManager) {
        return;
    }
    
    // Audio is handled internally - no need for debug output
    
    // TODO: Load dedicated impact sound files and play them
    // auto impactSound = std::make_shared<Sound>("path/to/impact.wav");
    // auto audioSrc = std::make_shared<CEAudioSource>(impactSound);
    // audioSrc->setPosition(position);
    // audioSrc->setGain(1.0f);
    // m_audioManager->play(audioSrc);
}

void CEBulletProjectileManager::processFaceIntersections(CEBulletProjectile& projectile)
{
    const auto& intersections = projectile.getFaceIntersections();
    
    if (intersections.empty()) return;
    
    std::cout << "🔥 Processing " << intersections.size() << " face intersections for effects" << std::endl;
    
    for (const auto& intersection : intersections) {
        // Create effect markers at each face intersection point
        // This shows where the bullet passed through surfaces, even if it ended up below terrain
        
        // Choose effect color based on surface type
        glm::vec3 effectColor;
        std::string effectLabel;
        
        if (intersection.surfaceType == "terrain") {
            effectColor = glm::vec3(1.0f, 0.8f, 0.0f); // Orange for terrain hits
            effectLabel = "terrain_face_hit";
        } else if (intersection.surfaceType == "object") {
            effectColor = glm::vec3(1.0f, 0.0f, 1.0f); // Magenta for object hits
            effectLabel = "object_face_hit";
        } else if (intersection.surfaceType == "water") {
            effectColor = glm::vec3(0.0f, 0.8f, 1.0f); // Cyan for water hits
            effectLabel = "water_face_hit";
        } else {
            effectColor = glm::vec3(0.5f, 0.5f, 0.5f); // Gray for unknown
            effectLabel = "unknown_face_hit";
        }
        
        // Add face intersection event to UI with all detailed information
        addFaceIntersectionEvent(
            intersection.position,
            intersection.surfaceType,
            intersection.distance,
            intersection.normal,
            intersection.incomingDirection,
            intersection.tileX,
            intersection.tileZ,
            intersection.faceIndex,
            intersection.objectName,
            intersection.objectIndex,
            intersection.instanceIndex
        );
        
        std::cout << "  💥 " << intersection.surfaceType << " face hit at [" 
                  << intersection.position.x << ", " << intersection.position.y << ", " << intersection.position.z << "]";
        
        if (intersection.surfaceType == "terrain") {
            std::cout << " tile[" << intersection.tileX << "," << intersection.tileZ << "]";
        } else if (intersection.surfaceType == "object") {
            std::cout << " object: " << intersection.objectName;
        }
        
        std::cout << " normal: [" << intersection.normal.x << ", " << intersection.normal.y << ", " << intersection.normal.z << "]" << std::endl;
        
        // TODO: Add particle effects, decals, ricochet calculations based on normals
        // TODO: Add surface-specific impact sounds
        // TODO: Add damage calculations for objects
    }
    
    // Clear processed intersections to prevent reprocessing
    projectile.clearFaceIntersections();
}