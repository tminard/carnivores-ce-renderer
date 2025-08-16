//
//  CETerrainPartition.cpp
//  CE Character Lab
//
//  Simple terrain partitioning system for efficient collision detection
//

#include "CETerrainPartition.h"
#include "C2MapFile.h"
#include "CEBulletHeightfield.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <limits>

CETerrainPartition::CETerrainPartition(C2MapFile* mapFile, int partitionSize)
    : m_mapFile(mapFile)
    , m_heightfieldTerrain(nullptr)
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

CETerrainPartition::CETerrainPartition(C2MapFile* mapFile, CEBulletHeightfield* heightfieldTerrain, int partitionSize)
    : m_mapFile(mapFile)
    , m_heightfieldTerrain(heightfieldTerrain)
    , m_partitionSize(partitionSize)
{
    if (m_mapFile) {
        m_tileSize = m_mapFile->getTileLength();
        m_mapWidth = m_mapFile->getWidth();
        m_mapHeight = m_mapFile->getHeight();
        
        // Calculate number of partitions needed
        m_partitionsX = (m_mapWidth + m_partitionSize - 1) / m_partitionSize;
        m_partitionsZ = (m_mapHeight + m_partitionSize - 1) / m_partitionSize;
        
        std::cout << "🗺️ CETerrainPartition (with heightfield) initialized: " << m_mapWidth << "x" << m_mapHeight 
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
    
    // Convert to partition space
    float partitionWorldSize = m_partitionSize * m_tileSize;
    
    // Start and end partition coordinates
    int startPartX = (int)std::floor(rayStart.x / partitionWorldSize);
    int startPartZ = (int)std::floor(rayStart.z / partitionWorldSize);
    int endPartX = (int)std::floor(rayEnd.x / partitionWorldSize);
    int endPartZ = (int)std::floor(rayEnd.z / partitionWorldSize);
    
    // If ray is within single partition, return that
    if (startPartX == endPartX && startPartZ == endPartZ) {
        int index = getPartitionIndex(startPartX, startPartZ);
        if (index >= 0) {
            result.push_back(index);
        }
        return result;
    }
    
    // Use 2D DDA to traverse partition grid
    int currentPartX = startPartX;
    int currentPartZ = startPartZ;
    
    // Ray direction in partition space
    glm::vec2 rayDir2D = glm::vec2(rayEnd.x - rayStart.x, rayEnd.z - rayStart.z);
    float rayLength = glm::length(rayDir2D);
    
    if (rayLength < 0.001f) {
        // Degenerate ray
        int index = getPartitionIndex(startPartX, startPartZ);
        if (index >= 0) {
            result.push_back(index);
        }
        return result;
    }
    
    rayDir2D = glm::normalize(rayDir2D);
    
    // DDA setup
    int stepX = (rayDir2D.x > 0) ? 1 : -1;
    int stepZ = (rayDir2D.y > 0) ? 1 : -1;
    
    // Calculate delta distances (how far along ray to traverse one partition)
    float deltaDistX = (rayDir2D.x != 0) ? std::abs(partitionWorldSize / rayDir2D.x) : std::numeric_limits<float>::max();
    float deltaDistZ = (rayDir2D.y != 0) ? std::abs(partitionWorldSize / rayDir2D.y) : std::numeric_limits<float>::max();
    
    // Calculate initial distances to next partition boundaries
    float partCenterX = (currentPartX + 0.5f) * partitionWorldSize;
    float partCenterZ = (currentPartZ + 0.5f) * partitionWorldSize;
    
    float sideDistX, sideDistZ;
    if (rayDir2D.x < 0) {
        sideDistX = (rayStart.x - (partCenterX - partitionWorldSize * 0.5f)) / std::abs(rayDir2D.x);
    } else {
        sideDistX = ((partCenterX + partitionWorldSize * 0.5f) - rayStart.x) / std::abs(rayDir2D.x);
    }
    
    if (rayDir2D.y < 0) {
        sideDistZ = (rayStart.z - (partCenterZ - partitionWorldSize * 0.5f)) / std::abs(rayDir2D.y);
    } else {
        sideDistZ = ((partCenterZ + partitionWorldSize * 0.5f) - rayStart.z) / std::abs(rayDir2D.y);
    }
    
    // Traverse partitions along the ray
    float currentDistance = 0.0f;
    int maxPartitions = 20; // Safety limit
    
    for (int i = 0; i < maxPartitions; i++) {
        // Add current partition if valid
        int index = getPartitionIndex(currentPartX, currentPartZ);
        if (index >= 0) {
            result.push_back(index);
        }
        
        // Check if we've reached the end partition
        if (currentPartX == endPartX && currentPartZ == endPartZ) {
            break;
        }
        
        // Move to next partition
        if (sideDistX < sideDistZ) {
            sideDistX += deltaDistX;
            currentPartX += stepX;
            currentDistance = sideDistX - deltaDistX;
        } else {
            sideDistZ += deltaDistZ;
            currentPartZ += stepZ;
            currentDistance = sideDistZ - deltaDistZ;
        }
        
        // Stop if we've exceeded the ray length
        if (currentDistance > rayLength) {
            break;
        }
    }
    
    return result;
}

std::vector<CETerrainPartition::TileCandidate> CETerrainPartition::getTileCandidatesForRay(const glm::vec3& rayStart, const glm::vec3& rayEnd) const
{
    std::vector<TileCandidate> candidates;
    
    if (!m_mapFile) return candidates;
    
    // First get partitions that ray intersects
    auto partitionIndices = getPartitionsForRay(rayStart, rayEnd);
    
    // Convert ray to tile space for efficient traversal
    int startTileX = (int)std::floor(rayStart.x / m_tileSize);
    int startTileZ = (int)std::floor(rayStart.z / m_tileSize);
    int endTileX = (int)std::floor(rayEnd.x / m_tileSize);
    int endTileZ = (int)std::floor(rayEnd.z / m_tileSize);
    
    // Use 2D DDA to traverse tiles along ray (limited to ~16 tiles)
    int currentTileX = startTileX;
    int currentTileZ = startTileZ;
    
    glm::vec2 rayDir2D = glm::vec2(rayEnd.x - rayStart.x, rayEnd.z - rayStart.z);
    float rayLength = glm::length(rayDir2D);
    
    if (rayLength < 0.001f) return candidates;
    
    rayDir2D = glm::normalize(rayDir2D);
    
    // DDA setup for tile traversal
    int stepX = (rayDir2D.x > 0) ? 1 : -1;
    int stepZ = (rayDir2D.y > 0) ? 1 : -1;
    
    float deltaDistX = (rayDir2D.x != 0) ? std::abs(m_tileSize / rayDir2D.x) : std::numeric_limits<float>::max();
    float deltaDistZ = (rayDir2D.y != 0) ? std::abs(m_tileSize / rayDir2D.y) : std::numeric_limits<float>::max();
    
    // Calculate initial distances to next tile boundaries
    float tileCenterX = (currentTileX + 0.5f) * m_tileSize;
    float tileCenterZ = (currentTileZ + 0.5f) * m_tileSize;
    
    float sideDistX, sideDistZ;
    if (rayDir2D.x < 0) {
        sideDistX = (rayStart.x - (tileCenterX - m_tileSize * 0.5f)) / std::abs(rayDir2D.x);
    } else {
        sideDistX = ((tileCenterX + m_tileSize * 0.5f) - rayStart.x) / std::abs(rayDir2D.x);
    }
    
    if (rayDir2D.y < 0) {
        sideDistZ = (rayStart.z - (tileCenterZ - m_tileSize * 0.5f)) / std::abs(rayDir2D.y);
    } else {
        sideDistZ = ((tileCenterZ + m_tileSize * 0.5f) - rayStart.z) / std::abs(rayDir2D.y);
    }
    
    // Traverse tiles along ray (max 16 as requested)
    const int maxTiles = 16;
    float currentDistance = 0.0f;
    
    for (int i = 0; i < maxTiles; i++) {
        // Check if current tile is within any of our partitions
        for (int partitionIndex : partitionIndices) {
            if (partitionIndex < 0 || partitionIndex >= m_partitions.size()) continue;
            
            const Partition& partition = m_partitions[partitionIndex];
            
            // Find this tile within the partition
            for (const TileInfo& tile : partition.tiles) {
                if (tile.tileX == currentTileX && tile.tileZ == currentTileZ) {
                    TileCandidate candidate;
                    candidate.tileX = currentTileX;
                    candidate.tileZ = currentTileZ;
                    candidate.partitionIndex = partitionIndex;
                    candidate.minHeight = tile.minHeight;
                    candidate.maxHeight = tile.maxHeight;
                    
                    candidates.push_back(candidate);
                    goto next_tile; // Found tile, move to next
                }
            }
        }
        
        next_tile:
        // Check if we've reached the end tile
        if (currentTileX == endTileX && currentTileZ == endTileZ) {
            break;
        }
        
        // Move to next tile
        if (sideDistX < sideDistZ) {
            sideDistX += deltaDistX;
            currentTileX += stepX;
            currentDistance = sideDistX - deltaDistX;
        } else {
            sideDistZ += deltaDistZ;
            currentTileZ += stepZ;
            currentDistance = sideDistZ - deltaDistZ;
        }
        
        // Stop if we've exceeded the ray length
        if (currentDistance > rayLength) {
            break;
        }
    }
    
    return candidates;
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