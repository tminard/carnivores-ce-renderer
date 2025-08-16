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

using libAF2::Sound;

CEBulletProjectileManager::CEBulletProjectileManager(C2MapFile* map, C2MapRscFile* mapRsc, LocalAudioManager* audioManager)
    : m_map(map), m_mapRsc(mapRsc), m_audioManager(audioManager)
{
    // Initialize physics world with terrain, objects, and water
    m_physicsWorld = std::make_unique<CEPhysicsWorld>(map, mapRsc);
    
    std::cout << "Bullet Physics projectile system initialized" << std::endl;
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
    std::cout << "Spawning ballistic projectile from: [" << origin.x << ", " << origin.y << ", " << origin.z << "]" << std::endl;
    std::cout << "Direction: [" << direction.x << ", " << direction.y << ", " << direction.z << "]" << std::endl;
    std::cout << "Muzzle velocity: " << muzzleVelocity << " m/s" << std::endl;
    
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
    
    std::cout << "Active projectiles: " << m_activeProjectiles.size() << std::endl;
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
    
    std::cout << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << "🎯 BULLET IMPACT REPORT" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << "📍 Impact Location: [" << hitPoint.x << ", " << hitPoint.y << ", " << hitPoint.z << "]" << std::endl;
    std::cout << "🎲 Surface Hit: " << surfaceType << std::endl;
    std::cout << "📏 Travel Distance: " << distance << " meters" << std::endl;
    std::cout << "⚡ Damage Dealt: " << damage << " HP" << std::endl;
    std::cout << "═══════════════════════════════════════" << std::endl;
    std::cout << std::endl;
    
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
    
    // For now, just log that audio would play
    std::cout << "🔊 Impact Audio: " << surfaceType << " surface impact sound" << std::endl;
    
    // TODO: Load dedicated impact sound files and play them
    // auto impactSound = std::make_shared<Sound>("path/to/impact.wav");
    // auto audioSrc = std::make_shared<CEAudioSource>(impactSound);
    // audioSrc->setPosition(position);
    // audioSrc->setGain(1.0f);
    // m_audioManager->play(audioSrc);
}