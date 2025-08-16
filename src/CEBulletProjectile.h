//
//  CEBulletProjectile.h
//  CE Character Lab
//
//  Bullet Physics-based projectile for realistic ballistics
//

#ifndef __CE_Character_Lab__CEBulletProjectile__
#define __CE_Character_Lab__CEBulletProjectile__

#include <glm/glm.hpp>
#include <memory>
#include <string>

// Forward declarations
class btRigidBody;
class btDiscreteDynamicsWorld;
class CEPhysicsWorld;
struct btManifoldPoint;
struct btCollisionObjectWrapper;

class CEBulletProjectile
{
private:
    btRigidBody* m_rigidBody;
    float m_damage;
    double m_spawnTime;
    float m_maxLifetime; // Maximum time before bullet is destroyed
    glm::vec3 m_spawnPosition; // Original spawn position for distance calculation
    bool m_hasImpacted;
    glm::vec3 m_impactPoint;
    glm::vec3 m_impactNormal;
    std::string m_impactSurfaceType;
    float m_impactDistance;
    
    // Collision detection state
    bool m_checkedForContacts;
    
public:
    CEBulletProjectile(btDiscreteDynamicsWorld* world, 
                       const glm::vec3& position, 
                       const glm::vec3& velocity, 
                       float damage,
                       float maxLifetime = 10.0f);
    ~CEBulletProjectile();
    
    // Update and check for impacts
    bool update(double currentTime, CEPhysicsWorld* physics);
    
    // Check for collisions using contact manifolds
    bool checkForCollisions(CEPhysicsWorld* physics);
    
    // Getters
    glm::vec3 getPosition() const;
    glm::vec3 getVelocity() const;
    float getDamage() const { return m_damage; }
    bool hasImpacted() const { return m_hasImpacted; }
    glm::vec3 getImpactPoint() const { return m_impactPoint; }
    glm::vec3 getImpactNormal() const { return m_impactNormal; }
    std::string getImpactSurfaceType() const { return m_impactSurfaceType; }
    float getImpactDistance() const { return m_impactDistance; }
    
    // Check if projectile should be destroyed
    bool shouldDestroy(double currentTime) const;
    
    // Get the rigid body (for removal from physics world)
    btRigidBody* getRigidBody() const { return m_rigidBody; }
};

#endif /* defined(__CE_Character_Lab__CEBulletProjectile__) */