//
//  CEBulletHeightfield.cpp
//  CE Character Lab
//
//  Bullet Physics custom terrain triangle mesh system with exact subdivision matching
//

#include "CEBulletHeightfield.h"
#include "C2MapFile.h"
#include "CETerrainPartition.h"

// Bullet Physics includes
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>
#include <BulletCollision/CollisionShapes/btTriangleMesh.h>

#include <iostream>
#include <algorithm>
#include <cmath>
#include <limits>

CEBulletHeightfield::CEBulletHeightfield(C2MapFile* mapFile)
    : m_mapFile(mapFile)
{
    if (!m_mapFile) {
        return;
    }
    
    m_tileSize = m_mapFile->getTileLength();
    m_mapWidth = m_mapFile->getWidth();
    m_mapHeight = m_mapFile->getHeight();
    
    
    // Build single terrain triangle mesh
    buildTerrainMesh();
    
}

CEBulletHeightfield::~CEBulletHeightfield()
{
    // Cleanup is automatic via unique_ptr
}

void CEBulletHeightfield::buildTerrainMesh()
{
    if (!m_mapFile) return;
    
    m_terrainMesh = std::make_unique<TerrainMesh>();
    
    // Calculate world bounds for entire map
    m_terrainMesh->worldMin = glm::vec3(0, -1000.0f, 0);
    m_terrainMesh->worldMax = glm::vec3(m_mapWidth * m_tileSize, 1000.0f, m_mapHeight * m_tileSize);
    
    // Create triangle mesh using EXACT same logic as TerrainRenderer::loadIntoHardwareMemory()
    m_terrainMesh->triangleMesh = new btTriangleMesh();
    m_terrainMesh->minHeight = std::numeric_limits<float>::max();
    m_terrainMesh->maxHeight = std::numeric_limits<float>::lowest();
    
    int triangleCount = 0;
    
    
    // Generate triangles for each tile quad (EXACT same loops as TerrainRenderer lines 934-1010)
    for (int y = 0; y < m_mapHeight - 1; y++) {
        for (int x = 0; x < m_mapWidth - 1; x++) {
            int base_index = (y * m_mapWidth) + x;
            
            // Use EXACT same logic as TerrainRenderer::calcWorldVertex()
            // LL (Lower Left): tile at (x, y)
            float h1 = m_mapFile->getHeightAt((y * m_mapWidth) + x);
            float worldX1 = (x * m_tileSize) + (m_tileSize / 2.0f);
            float worldZ1 = (y * m_tileSize) + (m_tileSize / 2.0f);
            btVector3 vpositionLL(worldX1, h1, worldZ1);
            
            // LR (Lower Right): tile at (x+1, y)
            float h2 = m_mapFile->getHeightAt((y * m_mapWidth) + (x + 1));
            float worldX2 = ((x + 1) * m_tileSize) + (m_tileSize / 2.0f);
            btVector3 vpositionLR(worldX2, h2, worldZ1);
            
            // UL (Upper Left): tile at (x, y+1)  
            float h3 = m_mapFile->getHeightAt(((y + 1) * m_mapWidth) + x);
            float worldZ2 = ((y + 1) * m_tileSize) + (m_tileSize / 2.0f);
            btVector3 vpositionUL(worldX1, h3, worldZ2);
            
            // UR (Upper Right): tile at (x+1, y+1)
            float h4 = m_mapFile->getHeightAt(((y + 1) * m_mapWidth) + (x + 1));
            btVector3 vpositionUR(worldX2, h4, worldZ2);
            
            // Update height bounds
            m_terrainMesh->minHeight = std::min({m_terrainMesh->minHeight, h1, h2, h3, h4});
            m_terrainMesh->maxHeight = std::max({m_terrainMesh->maxHeight, h1, h2, h3, h4});
            
            // Use EXACT same triangle subdivision logic as TerrainRenderer (lines 949-1002)
            bool quad_reverse = m_mapFile->isQuadRotatedAt(base_index);
            
            if (quad_reverse) {
                // Triangle 1: lower_left → upper_left → lower_right
                m_terrainMesh->triangleMesh->addTriangle(vpositionLL, vpositionUL, vpositionLR);
                // Triangle 2: lower_right → upper_left → upper_right  
                m_terrainMesh->triangleMesh->addTriangle(vpositionLR, vpositionUL, vpositionUR);
            } else {
                // Triangle 1: lower_left → upper_right → lower_right
                m_terrainMesh->triangleMesh->addTriangle(vpositionLL, vpositionUR, vpositionLR);
                // Triangle 2: lower_left → upper_left → upper_right
                m_terrainMesh->triangleMesh->addTriangle(vpositionLL, vpositionUL, vpositionUR);
            }
            
            
            triangleCount += 2;
        }
    }
    
    // Store triangle count
    m_terrainMesh->triangleCount = triangleCount;
    
    // Update world bounds with actual height range
    m_terrainMesh->worldMin.y = m_terrainMesh->minHeight;
    m_terrainMesh->worldMax.y = m_terrainMesh->maxHeight;
    
    // Create BVH triangle mesh shape from our custom triangle mesh
    // Bullet's BVH will handle spatial partitioning and culling internally
    m_terrainMesh->terrainShape = new btBvhTriangleMeshShape(m_terrainMesh->triangleMesh, true);
    
    // Create static rigid body for the triangle mesh
    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(btVector3(0, 0, 0));
    
    btDefaultMotionState* motionState = new btDefaultMotionState(transform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(0.0f, motionState, m_terrainMesh->terrainShape, btVector3(0, 0, 0));
    
    m_terrainMesh->terrainBody = new btRigidBody(rbInfo);
    m_terrainMesh->terrainBody->setRestitution(0.1f);
    m_terrainMesh->terrainBody->setFriction(0.8f);
    
    // Set user pointer for identification
    m_terrainMesh->terrainBody->setUserPointer(m_terrainMesh.get());
    
}

void CEBulletHeightfield::addToWorld(btDiscreteDynamicsWorld* world)
{
    if (!world || !m_terrainMesh || !m_terrainMesh->terrainBody) return;
    
    world->addRigidBody(m_terrainMesh->terrainBody, TERRAIN_COLLISION_GROUP, TERRAIN_COLLISION_MASK);
    
}

void CEBulletHeightfield::removeFromWorld(btDiscreteDynamicsWorld* world)
{
    if (!world || !m_terrainMesh || !m_terrainMesh->terrainBody) return;
    
    world->removeRigidBody(m_terrainMesh->terrainBody);
    
}

int CEBulletHeightfield::getTriangleCount() const
{
    return m_terrainMesh ? m_terrainMesh->triangleCount : 0;
}