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
#include <vector>
#include <mutex>

// Forward declarations
class btRigidBody;
class btDiscreteDynamicsWorld;
class CEPhysicsWorld;
struct btManifoldPoint;
struct btCollisionObjectWrapper;

// Face intersection data for effects processing
struct FaceIntersection {
    glm::vec3 position;           // Exact intersection point
    glm::vec3 normal;            // Surface normal at intersection
    glm::vec3 incomingDirection; // Bullet direction when hitting
    std::string surfaceType;     // "terrain", "object", "water"
    float distance;              // Distance from spawn point
    
    // For terrain faces
    int tileX = -1, tileZ = -1;  // Which heightfield tile
    
    // For object faces  
    std::string objectName;
    int objectIndex = -1;
    int instanceIndex = -1;
    int faceIndex = -1;          // Which triangle/face on the object
    
    double timestamp;            // When intersection occurred
};

// Thread-safe queue for processing face intersections
class FaceIntersectionQueue {
private:
    std::vector<FaceIntersection> m_intersections;
    mutable std::mutex m_mutex;
    
public:
    void addIntersection(const FaceIntersection& intersection);
    std::vector<FaceIntersection> getAndClearIntersections();
    size_t size() const;
};

class CEBulletProjectile
{
private:
    btRigidBody* m_rigidBody;
    btDiscreteDynamicsWorld* m_dynamicsWorld; // Store reference for removal
    float m_damage;
    double m_spawnTime;
    float m_maxLifetime; // Maximum time before bullet is destroyed
    glm::vec3 m_spawnPosition; // Original spawn position for distance calculation
    
    // Trajectory tracking for visualization
    std::vector<glm::vec3> m_trajectoryPoints;
    glm::vec3 m_lastRecordedPosition;
    float m_trajectoryRecordDistance; // Minimum distance before recording new point
    
    bool m_hasImpacted;
    glm::vec3 m_impactPoint;
    glm::vec3 m_impactNormal;
    std::string m_impactSurfaceType;
    float m_impactDistance;
    
    // Object impact information
    std::string m_impactObjectName;
    int m_impactObjectIndex;
    int m_impactInstanceIndex;
    
    
    // Collision detection state
    bool m_checkedForContacts;
    
    // Face intersection tracking for effects
    std::vector<FaceIntersection> m_faceIntersections;
    glm::vec3 m_lastFramePosition;
    
    // Debug frame counter
    int m_frameCount;
    
    // Manual physics simulation
    glm::vec3 m_currentPosition;
    glm::vec3 m_currentVelocity;
    glm::vec3 m_initialVelocity;
    
public:
    CEBulletProjectile(btDiscreteDynamicsWorld* world, 
                       const glm::vec3& position, 
                       const glm::vec3& velocity, 
                       float damage,
                       float maxLifetime = 10.0f);
    ~CEBulletProjectile();
    
    // Update and check for impacts
    bool update(double currentTime, CEPhysicsWorld* physics);
    
    // Getters
    glm::vec3 getPosition() const;
    glm::vec3 getVelocity() const;
    float getDamage() const { return m_damage; }
    bool hasImpacted() const { return m_hasImpacted; }
    glm::vec3 getImpactPoint() const { return m_impactPoint; }
    glm::vec3 getImpactNormal() const { return m_impactNormal; }
    std::string getImpactSurfaceType() const { return m_impactSurfaceType; }
    float getImpactDistance() const { return m_impactDistance; }
    
    // Object impact information getters
    std::string getImpactObjectName() const { return m_impactObjectName; }
    int getImpactObjectIndex() const { return m_impactObjectIndex; }
    int getImpactInstanceIndex() const { return m_impactInstanceIndex; }
    
    // Trajectory access
    const std::vector<glm::vec3>& getTrajectoryPoints() const { return m_trajectoryPoints; }
    
    // Face intersection access
    const std::vector<FaceIntersection>& getFaceIntersections() const { return m_faceIntersections; }
    void clearFaceIntersections() { m_faceIntersections.clear(); }
    
    // Check if projectile should be destroyed
    bool shouldDestroy(double currentTime) const;
    
    // Get the rigid body (for removal from physics world)
    btRigidBody* getRigidBody() const { return m_rigidBody; }

private:
    // Helper method to stop projectile motion on impact
    void stopProjectileMotion();
};

#endif /* defined(__CE_Character_Lab__CEBulletProjectile__) */