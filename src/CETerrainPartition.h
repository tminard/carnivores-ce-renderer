//
//  CETerrainPartition.h  
//  CE Character Lab
//
//  Simple terrain partitioning system for efficient collision detection
//

#ifndef __CE_Character_Lab__CETerrainPartition__
#define __CE_Character_Lab__CETerrainPartition__

#include <glm/glm.hpp>
#include <vector>

// Forward declarations
class C2MapFile;

class CETerrainPartition
{
public:
    struct TileInfo {
        int tileX, tileZ;
        float height;
        float minHeight, maxHeight; // For quick rejection
    };
    
    struct Partition {
        int partitionX, partitionZ;
        glm::vec3 worldMin, worldMax;
        std::vector<TileInfo> tiles;
    };

private:
    C2MapFile* m_mapFile;
    float m_tileSize;
    int m_mapWidth, m_mapHeight;
    
    // Partitioning parameters
    int m_partitionSize; // Number of tiles per partition side (e.g., 32x32 tiles)
    int m_partitionsX, m_partitionsZ;
    
    std::vector<Partition> m_partitions;
    
    void buildPartitions();
    int getPartitionIndex(int partitionX, int partitionZ) const;
    
public:
    CETerrainPartition(C2MapFile* mapFile, int partitionSize = 32);
    ~CETerrainPartition();
    
    // Get partition(s) that a world position intersects
    std::vector<int> getPartitionsForPosition(const glm::vec3& worldPos) const;
    std::vector<int> getPartitionsForRay(const glm::vec3& rayStart, const glm::vec3& rayEnd) const;
    
    // Simple height check within specific partitions
    bool checkHeightCollision(const glm::vec3& position, const std::vector<int>& partitionIndices, 
                             glm::vec3& hitPoint, float& groundHeight) const;
    
    // Debug info
    int getPartitionCount() const { return m_partitions.size(); }
    const Partition& getPartition(int index) const { return m_partitions[index]; }
};

#endif /* defined(__CE_Character_Lab__CETerrainPartition__) */