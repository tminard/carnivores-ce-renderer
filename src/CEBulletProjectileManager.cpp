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
        
        // Remove projectiles that should be destroyed
        if (projectile->shouldDestroy(currentTime)) {
            // Remove from physics world first
            if (projectile->getRigidBody()) {
                m_physicsWorld->removeRigidBody(projectile->getRigidBody());
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