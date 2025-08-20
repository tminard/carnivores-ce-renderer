//
//  CEBulletProjectileManager.h
//  CE Character Lab
//
//  Bullet Physics-based projectile system manager
//

#ifndef __CE_Character_Lab__CEBulletProjectileManager__
#define __CE_Character_Lab__CEBulletProjectileManager__

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <string>

class CEBulletProjectile;
class CEPhysicsWorld;
class C2MapFile;
class C2MapRscFile;
class LocalAudioManager;
class Camera;
class CEParticleSystem;

class CEBulletProjectileManager
{
private:
    std::unique_ptr<CEPhysicsWorld> m_physicsWorld;
    std::vector<std::unique_ptr<CEBulletProjectile>> m_activeProjectiles;
    C2MapFile* m_map;
    C2MapRscFile* m_mapRsc;
    LocalAudioManager* m_audioManager;
    std::unique_ptr<CEParticleSystem> m_particleSystem;
    
    // Impact sound configuration - arrays for randomization
    std::vector<std::string> m_terrainSoundPaths;
    std::vector<std::string> m_objectSoundPaths;
    std::vector<std::string> m_waterSoundPaths;
    
    // Impact handling
    void handleImpact(const CEBulletProjectile& projectile, double currentTime);
    void playImpactAudio(const glm::vec3& position, const std::string& surfaceType);
    void loadImpactSoundConfig();
    
    // Face intersection processing
    void processFaceIntersections(CEBulletProjectile& projectile);
    
public:
    CEBulletProjectileManager(C2MapFile* map, C2MapRscFile* mapRsc, LocalAudioManager* audioManager);
    ~CEBulletProjectileManager();
    
    // Spawn a new realistic ballistic projectile
    void spawnProjectile(const glm::vec3& origin, const glm::vec3& direction, 
                        float muzzleVelocity, float damage, const std::string& type = "rifle");
    
    // Update all active projectiles (call each frame)
    void update(double currentTime, double deltaTime);
    
    // Render particle effects
    void renderParticles(Camera* camera);
    
    // Get count of active projectiles
    size_t getActiveProjectileCount() const { return m_activeProjectiles.size(); }
    
    // Get physics world for other systems
    CEPhysicsWorld* getPhysicsWorld() const { return m_physicsWorld.get(); }
    
    // Get particle system for external access
    CEParticleSystem* getParticleSystem() const { return m_particleSystem.get(); }
};

#endif /* defined(__CE_Character_Lab__CEBulletProjectileManager__) */