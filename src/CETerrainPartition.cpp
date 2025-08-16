//
//  CETerrainPartition.cpp
//  CE Character Lab
//
//  Simple terrain partitioning system for efficient collision detection
//

#include "CETerrainPartition.h"
#include "C2MapFile.h"
#include <iostream>
#include <algorithm>
#include <cmath>

CETerrainPartition::CETerrainPartition(C2MapFile* mapFile, int partitionSize)
    : m_mapFile(mapFile)
    , m_partitionSize(partitionSize)
{
    if (m_mapFile) {
        m_tileSize = m_mapFile->getTileLength();
        m_mapWidth = m_mapFile->getWidth();
        m_mapHeight = m_mapFile->getHeight();
        
        // Calculate number of partitions needed
        m_partitionsX = (m_mapWidth + m_partitionSize - 1) / m_partitionSize;
        m_partitionsZ = (m_mapHeight + m_partitionSize - 1) / m_partitionSize;
        
        std::cout << "🗺️ CETerrainPartition initialized: " << m_mapWidth << "x" << m_mapHeight 
                  << " tiles, " << m_partitionSize << "x" << m_partitionSize << " partition size"
                  << ", " << m_partitionsX << "x" << m_partitionsZ << " partitions" << std::endl;
        
        buildPartitions();
    }
}

CETerrainPartition::~CETerrainPartition()
{
    // Partitions will be automatically cleaned up
}

void CETerrainPartition::buildPartitions()
{
    if (!m_mapFile) return;
    
    std::cout << "🔨 Building terrain partitions..." << std::endl;
    
    m_partitions.clear();
    m_partitions.reserve(m_partitionsX * m_partitionsZ);
    
    for (int partZ = 0; partZ < m_partitionsZ; partZ++) {
        for (int partX = 0; partX < m_partitionsX; partX++) {
            Partition partition;
            partition.partitionX = partX;
            partition.partitionZ = partZ;
            
            // Calculate world bounds for this partition
            int startTileX = partX * m_partitionSize;
            int startTileZ = partZ * m_partitionSize;
            int endTileX = std::min(startTileX + m_partitionSize, m_mapWidth);
            int endTileZ = std::min(startTileZ + m_partitionSize, m_mapHeight);
            
            partition.worldMin = glm::vec3(
                startTileX * m_tileSize,
                -1000.0f, // Will be updated
                startTileZ * m_tileSize
            );
            partition.worldMax = glm::vec3(
                endTileX * m_tileSize,
                1000.0f, // Will be updated
                endTileZ * m_tileSize
            );
            
            // Add tiles to this partition
            float minHeight = std::numeric_limits<float>::max();
            float maxHeight = std::numeric_limits<float>::lowest();
            
            for (int tileZ = startTileZ; tileZ < endTileZ; tileZ++) {
                for (int tileX = startTileX; tileX < endTileX; tileX++) {
                    TileInfo tileInfo;
                    tileInfo.tileX = tileX;
                    tileInfo.tileZ = tileZ;
                    tileInfo.height = m_mapFile->getPlaceGroundHeight(tileX, tileZ);
                    
                    // For now, use simple height values - could expand to sample corners later
                    tileInfo.minHeight = tileInfo.height;
                    tileInfo.maxHeight = tileInfo.height;
                    
                    partition.tiles.push_back(tileInfo);
                    
                    minHeight = std::min(minHeight, tileInfo.height);
                    maxHeight = std::max(maxHeight, tileInfo.height);
                }
            }
            
            // Update partition bounds with actual height range
            partition.worldMin.y = minHeight;
            partition.worldMax.y = maxHeight;
            
            m_partitions.push_back(partition);
        }
    }
    
    std::cout << "✅ Built " << m_partitions.size() << " terrain partitions" << std::endl;
}

int CETerrainPartition::getPartitionIndex(int partitionX, int partitionZ) const
{
    if (partitionX < 0 || partitionX >= m_partitionsX || 
        partitionZ < 0 || partitionZ >= m_partitionsZ) {
        return -1;
    }
    return partitionZ * m_partitionsX + partitionX;
}

std::vector<int> CETerrainPartition::getPartitionsForPosition(const glm::vec3& worldPos) const
{
    std::vector<int> result;
    
    // Convert world position to partition coordinates
    int partitionX = (int)std::floor(worldPos.x / (m_partitionSize * m_tileSize));
    int partitionZ = (int)std::floor(worldPos.z / (m_partitionSize * m_tileSize));
    
    int index = getPartitionIndex(partitionX, partitionZ);
    if (index >= 0) {
        result.push_back(index);
    }
    
    return result;
}

std::vector<int> CETerrainPartition::getPartitionsForRay(const glm::vec3& rayStart, const glm::vec3& rayEnd) const
{
    std::vector<int> result;
    
    // For simplicity, just get partitions for start and end points
    // Could be expanded to use DDA traversal for more accuracy
    auto startPartitions = getPartitionsForPosition(rayStart);
    auto endPartitions = getPartitionsForPosition(rayEnd);
    
    // Combine and deduplicate
    result.insert(result.end(), startPartitions.begin(), startPartitions.end());
    for (int index : endPartitions) {
        if (std::find(result.begin(), result.end(), index) == result.end()) {
            result.push_back(index);
        }
    }
    
    return result;
}

bool CETerrainPartition::checkHeightCollision(const glm::vec3& position, const std::vector<int>& partitionIndices,
                                            glm::vec3& hitPoint, float& groundHeight) const
{
    if (!m_mapFile || partitionIndices.empty()) {
        return false;
    }
    
    // Convert world position to tile coordinates
    int tileX = (int)std::floor(position.x / m_tileSize);
    int tileZ = (int)std::floor(position.z / m_tileSize);
    
    // Check if tile is within any of the specified partitions
    for (int partitionIndex : partitionIndices) {
        if (partitionIndex < 0 || partitionIndex >= m_partitions.size()) {
            continue;
        }
        
        const Partition& partition = m_partitions[partitionIndex];
        
        // Quick bounds check
        if (position.x < partition.worldMin.x || position.x >= partition.worldMax.x ||
            position.z < partition.worldMin.z || position.z >= partition.worldMax.z) {
            continue;
        }
        
        // Find the specific tile within this partition
        for (const TileInfo& tile : partition.tiles) {
            if (tile.tileX == tileX && tile.tileZ == tileZ) {
                groundHeight = tile.height;
                
                // Simple height collision check
                if (position.y <= groundHeight) {
                    hitPoint = glm::vec3(position.x, groundHeight, position.z);
                    return true;
                }
                
                return false; // Found tile but no collision
            }
        }
    }
    
    return false; // Tile not found in any partition
}