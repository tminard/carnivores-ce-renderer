//
//  CEBulletHeightfield.h
//  CE Character Lab
//
//  Bullet Physics custom terrain triangle mesh system with exact subdivision matching
//

#ifndef __CE_Character_Lab__CEBulletHeightfield__
#define __CE_Character_Lab__CEBulletHeightfield__

#include <glm/glm.hpp>
#include <vector>
#include <memory>

// Forward declarations
class C2MapFile;
class CETerrainPartition;
class btDiscreteDynamicsWorld;
class btBvhTriangleMeshShape;
class btTriangleMesh;
class btRigidBody;
class btVector3;

class CEBulletHeightfield
{
public:
    // Simplified single terrain mesh structure
    struct TerrainMesh {
        btTriangleMesh* triangleMesh;
        btBvhTriangleMeshShape* terrainShape;
        btRigidBody* terrainBody;
        
        glm::vec3 worldMin, worldMax;
        float minHeight, maxHeight;
        int triangleCount;
        
        TerrainMesh() : triangleMesh(nullptr), terrainShape(nullptr), terrainBody(nullptr), triangleCount(0) {}
        
        ~TerrainMesh() {
            if (triangleMesh) delete triangleMesh;
            if (terrainShape) delete terrainShape;
            // terrainBody is managed by physics world
        }
    };

private:
    C2MapFile* m_mapFile;
    float m_tileSize;
    int m_mapWidth, m_mapHeight;
    
    std::unique_ptr<TerrainMesh> m_terrainMesh;
    
    void buildTerrainMesh();
    
public:
    CEBulletHeightfield(C2MapFile* mapFile);
    ~CEBulletHeightfield();
    
    // Add/remove terrain mesh from physics world
    void addToWorld(btDiscreteDynamicsWorld* world);
    void removeFromWorld(btDiscreteDynamicsWorld* world);
    
    // Debug info
    int getTriangleCount() const;
    const TerrainMesh* getTerrainMesh() const { return m_terrainMesh.get(); }
    
    // Collision groups for terrain (matching CEPhysicsWorld::CollisionGroups)
    static const short TERRAIN_COLLISION_GROUP = 1 << 1;  // TERRAIN_GROUP (bit 1)
    static const short TERRAIN_COLLISION_MASK = (1 << 0) | (1 << 4);   // Can collide with PROJECTILE_GROUP and PLAYER_GROUP
};

#endif /* defined(__CE_Character_Lab__CEBulletHeightfield__) */