//
//  CEPhysicsWorld.h
//  CE Character Lab
//
//  Physics world wrapper for Bullet Physics integration
//

#ifndef __CE_Character_Lab__CEPhysicsWorld__
#define __CE_Character_Lab__CEPhysicsWorld__

#include <memory>
#include <vector>
#include <map>
#include <string>
#include <glm/glm.hpp>

// Forward declarations for Bullet Physics
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btDbvtBroadphase;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
class btTriangleMesh;
class btBvhTriangleMeshShape;
class btRigidBody;
class btCollisionShape;

// Forward declarations for debug rendering
class CEBulletDebugDraw;

// Forward declarations for Carnivores classes
class C2MapFile;
class C2MapRscFile;
class CEWorldModel;
class CEBulletHeightfield;

class CEPhysicsWorld
{
public:
    // Collision object identification
    enum class CollisionObjectType {
        TERRAIN = 0,
        WORLD_OBJECT = 1,
        WATER_PLANE = 2,
        HEIGHTFIELD_TERRAIN = 3,
        AI_CHARACTER = 4
    };
    
    // Collision groups for different object types
    enum CollisionGroups {
        PROJECTILE_GROUP = 1 << 0,
        TERRAIN_GROUP = 1 << 1,
        OBJECT_GROUP = 1 << 2,
        WATER_GROUP = 1 << 3,
        PLAYER_GROUP = 1 << 4,
        AI_GROUP = 1 << 5
    };
    
    struct CollisionObjectInfo {
        CollisionObjectType type;
        int objectIndex = -1;  // For world objects: model index in RSC file
        int instanceIndex = -1; // For world objects: which instance of the model
        std::string objectName = ""; // For debugging/display
        
        // Enhanced data for hierarchical collision
        class CEWorldModel* worldModel = nullptr; // For narrowphase TBound testing
        glm::vec3 instanceTransform = glm::vec3(0); // Instance world position
        glm::vec3 aabbCenter = glm::vec3(0); // AABB center offset from transform
    };
    
    struct RaycastResult {
        bool hasHit = false;
        glm::vec3 hitPoint;
        glm::vec3 hitNormal;
        btRigidBody* hitBody = nullptr;
        float distance = 0.0f;
        CollisionObjectInfo objectInfo;
    };

private:
    // Bullet Physics core components
    btDefaultCollisionConfiguration* m_collisionConfig;
    btCollisionDispatcher* m_dispatcher;
    btDbvtBroadphase* m_broadphase;
    btSequentialImpulseConstraintSolver* m_solver;
    btDiscreteDynamicsWorld* m_dynamicsWorld;
    
    // Terrain collision (legacy - being replaced by heightfield)
    btTriangleMesh* m_terrainMesh;
    btBvhTriangleMeshShape* m_terrainShape;
    btRigidBody* m_terrainBody;
    
    // Heightfield terrain system
    std::unique_ptr<CEBulletHeightfield> m_heightfieldTerrain;
    
    // Debug rendering
    std::unique_ptr<CEBulletDebugDraw> m_debugDrawer;
    
    // World objects collision
    std::vector<btTriangleMesh*> m_objectMeshes;
    std::vector<btBvhTriangleMeshShape*> m_baseBvhShapes; // Base BVH shapes for scaling/instancing
    std::vector<btCollisionShape*> m_objectShapes; // Scaled instances and other shapes
    std::vector<btRigidBody*> m_objectBodies;
    
    // Water planes collision
    std::vector<btCollisionShape*> m_waterShapes;
    std::vector<btRigidBody*> m_waterBodies;
    
    // Object identification mapping
    std::map<btRigidBody*, CollisionObjectInfo> m_objectInfoMap;
    
    // Keep references to map files for calculations
    C2MapFile* m_mapFile;
    
    // Helper methods
    void setupTerrain(C2MapFile* mapFile);
    void setupHeightfieldTerrain(C2MapFile* mapFile);
    void setupWorldObjects(C2MapRscFile* mapRsc);
    void setupWaterPlanes(C2MapFile* mapFile);
    btRigidBody* createStaticBody(btCollisionShape* shape, const glm::vec3& position);
    btRigidBody* createStaticBody(btCollisionShape* shape, const glm::vec3& position, const glm::vec3& rotation);
    
public:
    CEPhysicsWorld(C2MapFile* mapFile, C2MapRscFile* mapRsc);
    ~CEPhysicsWorld();
    
    // Core physics operations
    void stepSimulation(float deltaTime);
    btDiscreteDynamicsWorld* getDynamicsWorld() { return m_dynamicsWorld; }
    
    // Projectile creation
    btRigidBody* createProjectile(const glm::vec3& position, const glm::vec3& velocity, float mass = 0.008f);
    
    // Raycasting for immediate hit detection
    RaycastResult raycast(const glm::vec3& from, const glm::vec3& to);
    
    // Check if a specific rigid body has any contacts
    bool hasContacts(btRigidBody* body);
    
    // Get contact information with object filtering for visible geometry
    struct ContactInfo {
        bool hasValidContact = false;
        glm::vec3 contactPoint;
        glm::vec3 contactNormal;
        CollisionObjectInfo objectInfo;
        btRigidBody* contactedBody = nullptr;
    };
    ContactInfo getFilteredContacts(btRigidBody* body);
    
    // Get the map reference for tile calculations
    C2MapFile* getMapFile() const { return m_mapFile; }
    
    // Heightfield terrain access
    CEBulletHeightfield* getHeightfieldTerrain() const { return m_heightfieldTerrain.get(); }
    void updateHeightfieldForPosition(const glm::vec3& position);
    
    // Debug rendering
    void enablePhysicsDebugRendering(bool enable);
    void renderPhysicsDebug(const glm::mat4& viewProjectionMatrix, const glm::vec3& cameraPosition);
    
    // Object registration for raycast identification
    void registerCollisionObject(btRigidBody* body, const CollisionObjectInfo& info);
    
    // Cleanup
    void removeRigidBody(btRigidBody* body);
};

#endif /* defined(__CE_Character_Lab__CEPhysicsWorld__) */