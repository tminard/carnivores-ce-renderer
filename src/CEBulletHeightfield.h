//
//  CEBulletHeightfield.h
//  CE Character Lab
//
//  Bullet Physics heightfield terrain system with partition support
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
class btHeightfieldTerrainShape;
class btRigidBody;
class btVector3;

class CEBulletHeightfield
{
public:
    struct HeightfieldPartition {
        int partitionX, partitionZ;
        int startTileX, startTileZ;
        int widthTiles, heightTiles;
        
        std::vector<float> heightData;
        btHeightfieldTerrainShape* terrainShape;
        btRigidBody* terrainBody;
        
        glm::vec3 worldMin, worldMax;
        float minHeight, maxHeight;
        
        HeightfieldPartition() : terrainShape(nullptr), terrainBody(nullptr) {}
    };

private:
    C2MapFile* m_mapFile;
    float m_tileSize;
    int m_mapWidth, m_mapHeight;
    int m_partitionSize;
    
    std::vector<std::unique_ptr<HeightfieldPartition>> m_heightfieldPartitions;
    
    // Active partition management for performance
    std::vector<int> m_activePartitionIndices;
    glm::vec3 m_lastQueryPosition;
    float m_activationRadius;
    
    void buildHeightfieldPartition(int partitionX, int partitionZ);
    void updateActivePartitions(const glm::vec3& queryPosition);
    
public:
    CEBulletHeightfield(C2MapFile* mapFile, int partitionSize = 32, float activationRadius = 2000.0f);
    ~CEBulletHeightfield();
    
    // Add/remove partitions from physics world
    void addPartitionsToWorld(btDiscreteDynamicsWorld* world);
    void removePartitionsFromWorld(btDiscreteDynamicsWorld* world);
    
    // Dynamic partition management based on position
    void updateForPosition(const glm::vec3& position, btDiscreteDynamicsWorld* world);
    
    // Get partitions that intersect with a ray or area
    std::vector<int> getActivePartitionsForRay(const glm::vec3& rayStart, const glm::vec3& rayEnd) const;
    std::vector<int> getActivePartitionsForArea(const glm::vec3& center, float radius) const;
    
    // Debug info
    int getPartitionCount() const { return static_cast<int>(m_heightfieldPartitions.size()); }
    int getActivePartitionCount() const { return static_cast<int>(m_activePartitionIndices.size()); }
    const HeightfieldPartition* getPartition(int index) const;
    
    // Collision groups for terrain (matching CEPhysicsWorld::CollisionGroups)
    static const short TERRAIN_COLLISION_GROUP = 1 << 1;  // TERRAIN_GROUP
    static const short TERRAIN_COLLISION_MASK = 1 << 0;   // Can collide with PROJECTILE_GROUP
};

#endif /* defined(__CE_Character_Lab__CEBulletHeightfield__) */