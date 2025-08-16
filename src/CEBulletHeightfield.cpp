//
//  CEBulletHeightfield.cpp
//  CE Character Lab
//
//  Bullet Physics heightfield terrain system with partition support
//

#include "CEBulletHeightfield.h"
#include "C2MapFile.h"
#include "CETerrainPartition.h"

// Bullet Physics includes
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>

#include <iostream>
#include <algorithm>
#include <cmath>
#include <limits>

CEBulletHeightfield::CEBulletHeightfield(C2MapFile* mapFile, int partitionSize, float activationRadius)
    : m_mapFile(mapFile)
    , m_partitionSize(partitionSize)
    , m_activationRadius(activationRadius)
    , m_lastQueryPosition(0, 0, 0)
{
    if (!m_mapFile) {
        std::cout << "❌ CEBulletHeightfield: Invalid map file" << std::endl;
        return;
    }
    
    m_tileSize = m_mapFile->getTileLength();
    m_mapWidth = m_mapFile->getWidth();
    m_mapHeight = m_mapFile->getHeight();
    
    // Calculate number of partitions needed
    int partitionsX = (m_mapWidth + m_partitionSize - 1) / m_partitionSize;
    int partitionsZ = (m_mapHeight + m_partitionSize - 1) / m_partitionSize;
    
    std::cout << "🏔️ CEBulletHeightfield: Creating " << partitionsX << "x" << partitionsZ 
              << " heightfield partitions (" << m_partitionSize << "x" << m_partitionSize << " tiles each)" << std::endl;
    
    m_heightfieldPartitions.reserve(partitionsX * partitionsZ);
    
    // Build all heightfield partitions
    for (int partZ = 0; partZ < partitionsZ; partZ++) {
        for (int partX = 0; partX < partitionsX; partX++) {
            buildHeightfieldPartition(partX, partZ);
        }
    }
    
    std::cout << "✅ Built " << m_heightfieldPartitions.size() << " heightfield partitions" << std::endl;
}

CEBulletHeightfield::~CEBulletHeightfield()
{
    // Cleanup is handled by removePartitionsFromWorld
    // Individual partition cleanup is automatic via unique_ptr
}

void CEBulletHeightfield::buildHeightfieldPartition(int partitionX, int partitionZ)
{
    if (!m_mapFile) return;
    
    auto partition = std::make_unique<HeightfieldPartition>();
    partition->partitionX = partitionX;
    partition->partitionZ = partitionZ;
    
    // Calculate tile bounds for this partition
    partition->startTileX = partitionX * m_partitionSize;
    partition->startTileZ = partitionZ * m_partitionSize;
    partition->widthTiles = std::min(m_partitionSize, m_mapWidth - partition->startTileX);
    partition->heightTiles = std::min(m_partitionSize, m_mapHeight - partition->startTileZ);
    
    // Calculate world bounds
    partition->worldMin = glm::vec3(
        partition->startTileX * m_tileSize,
        -1000.0f, // Will be updated
        partition->startTileZ * m_tileSize
    );
    partition->worldMax = glm::vec3(
        (partition->startTileX + partition->widthTiles) * m_tileSize,
        1000.0f, // Will be updated
        (partition->startTileZ + partition->heightTiles) * m_tileSize
    );
    
    // Build height data for Bullet heightfield
    partition->heightData.reserve(partition->widthTiles * partition->heightTiles);
    partition->minHeight = std::numeric_limits<float>::max();
    partition->maxHeight = std::numeric_limits<float>::lowest();
    
    // Bullet expects height data in row-major order (Z then X)
    for (int localZ = 0; localZ < partition->heightTiles; localZ++) {
        for (int localX = 0; localX < partition->widthTiles; localX++) {
            int globalX = partition->startTileX + localX;
            int globalZ = partition->startTileZ + localZ;
            
            float height = m_mapFile->getPlaceGroundHeight(globalX, globalZ);
            partition->heightData.push_back(height);
            
            partition->minHeight = std::min(partition->minHeight, height);
            partition->maxHeight = std::max(partition->maxHeight, height);
        }
    }
    
    // Update world bounds with actual height range
    partition->worldMin.y = partition->minHeight;
    partition->worldMax.y = partition->maxHeight;
    
    // Create Bullet heightfield shape
    partition->terrainShape = new btHeightfieldTerrainShape(
        partition->widthTiles,
        partition->heightTiles,
        partition->heightData.data(),
        1.0f, // heightScale
        partition->minHeight,
        partition->maxHeight,
        1, // upAxis (Y)
        PHY_FLOAT,
        false // flipQuadEdges
    );
    
    // Configure heightfield properties
    partition->terrainShape->setUseDiamondSubdivision(true); // Better triangle quality
    partition->terrainShape->setLocalScaling(btVector3(m_tileSize, 1.0f, m_tileSize));
    
    // Create static rigid body for the heightfield
    btTransform transform;
    transform.setIdentity();
    
    // Position the heightfield at its world location
    // Bullet heightfield is centered, so we need to offset by half the size
    float centerOffsetX = (partition->widthTiles - 1) * m_tileSize * 0.5f;
    float centerOffsetZ = (partition->heightTiles - 1) * m_tileSize * 0.5f;
    
    transform.setOrigin(btVector3(
        partition->worldMin.x + centerOffsetX,
        (partition->minHeight + partition->maxHeight) * 0.5f,
        partition->worldMin.z + centerOffsetZ
    ));
    
    btDefaultMotionState* motionState = new btDefaultMotionState(transform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(0.0f, motionState, partition->terrainShape, btVector3(0, 0, 0));
    
    partition->terrainBody = new btRigidBody(rbInfo);
    partition->terrainBody->setRestitution(0.1f);
    partition->terrainBody->setFriction(0.8f);
    
    // Set user pointer for identification
    partition->terrainBody->setUserPointer(partition.get());
    
    std::cout << "🏔️ Built heightfield partition[" << partitionX << "," << partitionZ 
              << "] tiles=" << partition->widthTiles << "x" << partition->heightTiles
              << " height_range=[" << partition->minHeight << "," << partition->maxHeight << "]" << std::endl;
    
    m_heightfieldPartitions.push_back(std::move(partition));
}

void CEBulletHeightfield::addPartitionsToWorld(btDiscreteDynamicsWorld* world)
{
    if (!world) return;
    
    for (const auto& partition : m_heightfieldPartitions) {
        if (partition->terrainBody) {
            world->addRigidBody(partition->terrainBody, TERRAIN_COLLISION_GROUP, TERRAIN_COLLISION_MASK);
        }
    }
    
    std::cout << "✅ Added " << m_heightfieldPartitions.size() << " heightfield partitions to physics world" << std::endl;
}

void CEBulletHeightfield::removePartitionsFromWorld(btDiscreteDynamicsWorld* world)
{
    if (!world) return;
    
    for (const auto& partition : m_heightfieldPartitions) {
        if (partition->terrainBody) {
            world->removeRigidBody(partition->terrainBody);
        }
    }
    
    std::cout << "🗑️ Removed heightfield partitions from physics world" << std::endl;
}

void CEBulletHeightfield::updateActivePartitions(const glm::vec3& queryPosition)
{
    // Only update if position changed significantly
    float distanceMoved = glm::distance(queryPosition, m_lastQueryPosition);
    if (distanceMoved < m_activationRadius * 0.1f) {
        return; // Not enough movement to warrant update
    }
    
    m_lastQueryPosition = queryPosition;
    m_activePartitionIndices.clear();
    
    // Find partitions within activation radius
    for (size_t i = 0; i < m_heightfieldPartitions.size(); i++) {
        const auto& partition = m_heightfieldPartitions[i];
        
        // Calculate distance from query position to partition center
        glm::vec3 partitionCenter = (partition->worldMin + partition->worldMax) * 0.5f;
        partitionCenter.y = queryPosition.y; // Ignore height difference
        
        float distance = glm::distance(queryPosition, partitionCenter);
        
        if (distance <= m_activationRadius) {
            m_activePartitionIndices.push_back(static_cast<int>(i));
        }
    }
    
    std::cout << "🎯 Updated active partitions: " << m_activePartitionIndices.size() 
              << "/" << m_heightfieldPartitions.size() << " at position [" 
              << queryPosition.x << "," << queryPosition.y << "," << queryPosition.z << "]" << std::endl;
}

void CEBulletHeightfield::updateForPosition(const glm::vec3& position, btDiscreteDynamicsWorld* world)
{
    updateActivePartitions(position);
    // In the future, we could dynamically add/remove partitions from the world here
    // For now, all partitions remain in the world for simplicity
}

std::vector<int> CEBulletHeightfield::getActivePartitionsForRay(const glm::vec3& rayStart, const glm::vec3& rayEnd) const
{
    std::vector<int> result;
    
    // Check each active partition for ray intersection
    for (int partitionIndex : m_activePartitionIndices) {
        if (partitionIndex < 0 || partitionIndex >= static_cast<int>(m_heightfieldPartitions.size())) continue;
        
        const auto& partition = m_heightfieldPartitions[partitionIndex];
        
        // Simple AABB-ray intersection test
        glm::vec3 rayDir = rayEnd - rayStart;
        float rayLength = glm::length(rayDir);
        
        if (rayLength < 0.001f) continue;
        
        rayDir = glm::normalize(rayDir);
        
        // AABB intersection (simplified)
        glm::vec3 invRayDir = glm::vec3(1.0f / rayDir.x, 1.0f / rayDir.y, 1.0f / rayDir.z);
        
        glm::vec3 t1 = (partition->worldMin - rayStart) * invRayDir;
        glm::vec3 t2 = (partition->worldMax - rayStart) * invRayDir;
        
        glm::vec3 tMin = glm::min(t1, t2);
        glm::vec3 tMax = glm::max(t1, t2);
        
        float tNear = std::max({tMin.x, tMin.y, tMin.z});
        float tFar = std::min({tMax.x, tMax.y, tMax.z});
        
        if (tNear <= tFar && tFar >= 0.0f && tNear <= rayLength) {
            result.push_back(partitionIndex);
        }
    }
    
    return result;
}

std::vector<int> CEBulletHeightfield::getActivePartitionsForArea(const glm::vec3& center, float radius) const
{
    std::vector<int> result;
    
    for (int partitionIndex : m_activePartitionIndices) {
        if (partitionIndex < 0 || partitionIndex >= static_cast<int>(m_heightfieldPartitions.size())) continue;
        
        const auto& partition = m_heightfieldPartitions[partitionIndex];
        
        // Calculate distance from center to partition AABB
        glm::vec3 closestPoint = glm::clamp(center, partition->worldMin, partition->worldMax);
        float distance = glm::distance(center, closestPoint);
        
        if (distance <= radius) {
            result.push_back(partitionIndex);
        }
    }
    
    return result;
}

const CEBulletHeightfield::HeightfieldPartition* CEBulletHeightfield::getPartition(int index) const
{
    if (index < 0 || index >= static_cast<int>(m_heightfieldPartitions.size())) {
        return nullptr;
    }
    return m_heightfieldPartitions[index].get();
}