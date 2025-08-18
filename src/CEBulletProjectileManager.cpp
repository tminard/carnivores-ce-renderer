//
//  CEBulletProjectileManager.cpp
//  CE Character Lab
//
//  Bullet Physics-based projectile system manager
//

#include "CEBulletProjectileManager.h"
#include "CEBulletProjectile.h"
#include "CEPhysicsWorld.h"
#include "CEParticleSystem.h"
#include "C2MapFile.h"
#include "C2MapRscFile.h"
#include "LocalAudioManager.hpp"
#include "CEAudioSource.hpp"
#include "dependency/libAF/af2-sound.h"

#include <iostream>
#include <algorithm>
#include <fstream>
#include <filesystem>
#include <random>
#include <nlohmann/json.hpp>

// Forward declarations for impact event logging  
extern void addImpactEvent(const glm::vec3& location, const std::string& surfaceType, float distance, float damage, const std::string& impactType);
extern void addImpactEvent(const glm::vec3& location, const std::string& surfaceType, float distance, float damage, const std::string& impactType, const std::string& objectName, int objectIndex, int instanceIndex);
extern void addImpactEvent(const glm::vec3& location, const glm::vec3& surfaceNormal, const std::string& surfaceType, float distance, float damage, const std::string& impactType);
extern void addFaceIntersectionEvent(const glm::vec3& location, const std::string& surfaceType, float distance, const glm::vec3& normal, const glm::vec3& direction, int tileX, int tileZ, int faceIndex, const std::string& objectName, int objectIndex, int instanceIndex);
extern void addDebugSphere(const glm::vec3& position, float radius, const glm::vec3& color, const std::string& label);

using libAF2::Sound;
namespace fs = std::filesystem;
using json = nlohmann::json;

CEBulletProjectileManager::CEBulletProjectileManager(C2MapFile* map, C2MapRscFile* mapRsc, LocalAudioManager* audioManager)
    : m_map(map), m_mapRsc(mapRsc), m_audioManager(audioManager)
{
    // Initialize physics world with terrain, objects, and water
    m_physicsWorld.reset(new CEPhysicsWorld(map, mapRsc));
    
    // Initialize particle system for impact effects
    m_particleSystem.reset(new CEParticleSystem(2000)); // Max 2000 particles for intense effects
    
    // Load impact sound configuration from config.json
    loadImpactSoundConfig();
    
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

void CEBulletProjectileManager::loadImpactSoundConfig()
{
    try {
        std::ifstream f("config.json");
        if (!f.is_open()) {
            std::cout << "Warning: Could not open config.json for impact sound configuration" << std::endl;
            return;
        }
        
        json data = json::parse(f);
        
        // Get base path from config
        fs::path basePath = fs::path(data["basePath"].get<std::string>());
        
        // Load impact sound paths if they exist
        if (data.contains("impactSounds")) {
            const auto& impactSounds = data["impactSounds"];
            
            if (impactSounds.contains("terrain")) {
                for (const auto& soundFile : impactSounds["terrain"]) {
                    std::string fullPath = (basePath / soundFile.get<std::string>()).string();
                    m_terrainSoundPaths.push_back(fullPath);
                }
                std::cout << "🔊 Loaded " << m_terrainSoundPaths.size() << " terrain impact sounds" << std::endl;
            }
            
            if (impactSounds.contains("object")) {
                for (const auto& soundFile : impactSounds["object"]) {
                    std::string fullPath = (basePath / soundFile.get<std::string>()).string();
                    m_objectSoundPaths.push_back(fullPath);
                }
                std::cout << "🔊 Loaded " << m_objectSoundPaths.size() << " object impact sounds" << std::endl;
            }
            
            if (impactSounds.contains("water")) {
                for (const auto& soundFile : impactSounds["water"]) {
                    std::string fullPath = (basePath / soundFile.get<std::string>()).string();
                    m_waterSoundPaths.push_back(fullPath);
                }
                std::cout << "🔊 Loaded " << m_waterSoundPaths.size() << " water impact sounds" << std::endl;
            }
        } else {
            std::cout << "No impact sounds configuration found in config.json" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "Error loading impact sound configuration: " << e.what() << std::endl;
    }
}

void CEBulletProjectileManager::spawnProjectile(const glm::vec3& origin, const glm::vec3& direction, 
                                               float muzzleVelocity, float damage, const std::string& type)
{
    // Spawning ballistic projectile
    
    // Calculate initial velocity vector
    glm::vec3 velocity = glm::normalize(direction) * muzzleVelocity;
    
    // Create new bullet projectile
    std::unique_ptr<CEBulletProjectile> projectile(new CEBulletProjectile(
        m_physicsWorld->getDynamicsWorld(),
        origin,
        velocity,
        damage
    ));
    
    m_activeProjectiles.push_back(std::move(projectile));
    
    // Active projectiles count available via getActiveProjectileCount()
}

void CEBulletProjectileManager::update(double currentTime, double deltaTime)
{
    // Step the physics simulation
    m_physicsWorld->stepSimulation(static_cast<float>(deltaTime));
    
    // Update particle system
    if (m_particleSystem) {
        m_particleSystem->update(static_cast<float>(deltaTime));
    }
    
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
    
    // Log impact to GUI with proper surface normal for orientation
    glm::vec3 impactNormal = projectile.getImpactNormal();
    
    if (surfaceType == "object" && !projectile.getImpactObjectName().empty()) {
        // Use the surface normal version for proper orientation, even for objects
        addImpactEvent(hitPoint, impactNormal, surfaceType, distance, damage, "Bullet Impact");
    } else {
        // Use the new function with surface normal for proper orientation
        addImpactEvent(hitPoint, impactNormal, surfaceType, distance, damage, "Bullet Impact");
    }
    
    // Play impact audio with correct surface type
    playImpactAudio(hitPoint, surfaceType);
    
    // Add visual impact effects
    if (m_particleSystem) {
        std::cout << "🎆 Emitting particles for " << surfaceType << " impact at [" 
                  << hitPoint.x << ", " << hitPoint.y << ", " << hitPoint.z << "]" << std::endl;
        glm::vec3 impactNormal = projectile.getImpactNormal();
        
        if (surfaceType == "terrain" || surfaceType == "ground") {
            // Ground impact: much more dramatic dirt and dust
            std::cout << "   Emitting 60 ground impact + 40 dust particles" << std::endl;
            m_particleSystem->emitGroundImpact(hitPoint, impactNormal, 60);
            m_particleSystem->emitDustCloud(hitPoint, 40);
        } else if (surfaceType == "object") {
            // Object impact: explosive debris and sparks
            glm::vec3 impactDirection = glm::normalize(projectile.getVelocity());
            m_particleSystem->emitDebris(hitPoint, impactDirection, 35);
            m_particleSystem->emitGroundImpact(hitPoint, impactNormal, 25); // Heavy dust
            m_particleSystem->emitDustCloud(hitPoint, 20); // Extra smoke
        } else if (surfaceType == "water") {
            // Water impact: massive splash effects
            m_particleSystem->emitDustCloud(hitPoint, 80); // Major water spray
            m_particleSystem->emitGroundImpact(hitPoint, impactNormal, 40); // Heavy splash particles
        } else {
            // Default impact: heavy debris
            glm::vec3 impactDirection = glm::normalize(projectile.getVelocity());
            m_particleSystem->emitGroundImpact(hitPoint, impactNormal, 40);
            m_particleSystem->emitDebris(hitPoint, impactDirection, 25);
        }
    }
    
    // TODO: Add damage calculation for hit objects/characters
}

void CEBulletProjectileManager::playImpactAudio(const glm::vec3& position, const std::string& surfaceType)
{
    if (!m_audioManager) {
        return;
    }
    
    // Determine which sound array to use based on surface type
    std::vector<std::string>* soundPaths = nullptr;
    if (surfaceType == "terrain" || surfaceType == "ground") {
        soundPaths = &m_terrainSoundPaths;
    } else if (surfaceType == "object") {
        soundPaths = &m_objectSoundPaths;
    } else if (surfaceType == "water") {
        soundPaths = &m_waterSoundPaths;
    }
    
    // Only play sound if sounds are configured for this surface type
    if (soundPaths && !soundPaths->empty()) {
        // Randomly select a sound from the array
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, soundPaths->size() - 1);
        int randomIndex = dis(gen);
        
        const std::string& soundPath = (*soundPaths)[randomIndex];
        
        // Only play if the file exists
        if (fs::exists(soundPath)) {
            try {
                auto impactSound = std::make_shared<Sound>(soundPath);
                auto audioSrc = std::make_shared<CEAudioSource>(impactSound);
                audioSrc->setPosition(position);
                audioSrc->setGain(0.3f); // Moderate volume for impact sounds
                m_audioManager->play(audioSrc);
                
                std::cout << "🔊 Playing " << surfaceType << " impact sound [" << (randomIndex + 1) << "/" << soundPaths->size() << "]: " << soundPath << std::endl;
            } catch (const std::exception& e) {
                std::cout << "Error playing impact sound: " << e.what() << std::endl;
            }
        } else {
            std::cout << "Warning: Impact sound file not found: " << soundPath << std::endl;
        }
    }
    // If no sounds configured for this surface type, fail silently
}

void CEBulletProjectileManager::renderParticles(Camera* camera)
{
    if (m_particleSystem && camera) {
        size_t activeCount = m_particleSystem->getActiveParticleCount();
        if (activeCount > 0) {
          m_particleSystem->render(camera);
        }
    }
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
