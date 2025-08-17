//
//  CEPhysicsWorld.cpp
//  CE Character Lab
//
//  Physics world wrapper for Bullet Physics integration
//

#include "CEPhysicsWorld.h"
#include "C2MapFile.h"
#include "C2MapRscFile.h"
#include "CEWorldModel.h"
#include "CEGeometry.h"
#include "CEBulletHeightfield.h"
#include "transform.h"
#include "vertex.h"

// Bullet Physics includes
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>
#include <BulletCollision/CollisionShapes/btTriangleMesh.h>

#include <iostream>
#include <GLFW/glfw3.h>

CEPhysicsWorld::CEPhysicsWorld(C2MapFile* mapFile, C2MapRscFile* mapRsc)
    : m_collisionConfig(nullptr)
    , m_dispatcher(nullptr)
    , m_broadphase(nullptr)
    , m_solver(nullptr)
    , m_dynamicsWorld(nullptr)
    , m_terrainMesh(nullptr)
    , m_terrainShape(nullptr)
    , m_terrainBody(nullptr)
    , m_mapFile(mapFile)
{
    // Initialize Bullet Physics world
    m_collisionConfig = new btDefaultCollisionConfiguration();
    m_dispatcher = new btCollisionDispatcher(m_collisionConfig);
    m_broadphase = new btDbvtBroadphase();
    m_solver = new btSequentialImpulseConstraintSolver();
    
    m_dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher, m_broadphase, m_solver, m_collisionConfig);
    // Use much weaker gravity for realistic high-velocity bullet ballistics
    // Normal gravity (48) makes bullets drop too fast at rifle velocities
    float gameGravity = -15.0f; // Weaker gravity for realistic bullet trajectories
    m_dynamicsWorld->setGravity(btVector3(0, gameGravity, 0));
    
    // Setup collision geometry (terrain and world objects optimized for performance)
    // setupTerrain(mapFile);  // DISABLED: Too expensive, using height checks instead
    setupHeightfieldTerrain(mapFile);  // NEW: Bullet Physics heightfield terrain
    setupWorldObjects(mapRsc);  // RE-ENABLED: Optimized AABB-based hierarchical collision
    setupWaterPlanes(mapFile);  // RE-ENABLED: For water collision detection
    
    std::cout << "Physics world initialized with " << m_dynamicsWorld->getNumCollisionObjects() << " collision objects" << std::endl;
}

CEPhysicsWorld::~CEPhysicsWorld()
{
    // Remove heightfield terrain from world before cleanup
    if (m_heightfieldTerrain) {
        m_heightfieldTerrain->removePartitionsFromWorld(m_dynamicsWorld);
        m_heightfieldTerrain.reset(); // This will trigger destruction
    }
    
    // Remove all rigid bodies
    for (int i = m_dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--) {
        btCollisionObject* obj = m_dynamicsWorld->getCollisionObjectArray()[i];
        btRigidBody* body = btRigidBody::upcast(obj);
        if (body && body->getMotionState()) {
            delete body->getMotionState();
        }
        m_dynamicsWorld->removeCollisionObject(obj);
        delete obj;
    }
    
    // Clean up collision shapes
    for (auto* shape : m_objectShapes) {
        delete shape;
    }
    for (auto* mesh : m_objectMeshes) {
        delete mesh;
    }
    for (auto* shape : m_waterShapes) {
        delete shape;
    }
    
    if (m_terrainShape) delete m_terrainShape;
    if (m_terrainMesh) delete m_terrainMesh;
    
    // Clean up Bullet world
    delete m_dynamicsWorld;
    delete m_solver;
    delete m_broadphase;
    delete m_dispatcher;
    delete m_collisionConfig;
}

void CEPhysicsWorld::setupHeightfieldTerrain(C2MapFile* mapFile)
{
    if (!mapFile) return;
    
    std::cout << "🏔️ Setting up Bullet heightfield terrain..." << std::endl;
    
    // Create heightfield terrain system with partitioning
    m_heightfieldTerrain = std::make_unique<CEBulletHeightfield>(mapFile, 32, 2000.0f);
    
    // Add all partitions to physics world
    m_heightfieldTerrain->addPartitionsToWorld(m_dynamicsWorld);
    
    // Register heightfield terrain bodies in object info map for proper collision detection
    for (int i = 0; i < m_heightfieldTerrain->getPartitionCount(); i++) {
        const auto* partition = m_heightfieldTerrain->getPartition(i);
        if (partition && partition->terrainBody) {
            CollisionObjectInfo heightfieldInfo;
            heightfieldInfo.type = CollisionObjectType::HEIGHTFIELD_TERRAIN;
            heightfieldInfo.objectName = "HeightfieldTerrain_Partition_" + std::to_string(i);
            m_objectInfoMap[partition->terrainBody] = heightfieldInfo;
        }
    }
    
    std::cout << "✅ Heightfield terrain setup complete with " 
              << m_heightfieldTerrain->getPartitionCount() << " partitions registered" << std::endl;
}

void CEPhysicsWorld::setupTerrain(C2MapFile* mapFile)
{
    if (!mapFile) return;
    
    m_terrainMesh = new btTriangleMesh();
    
    float tileLength = mapFile->getTileLength();
    int width = mapFile->getWidth();
    int height = mapFile->getHeight();
    
    std::cout << "Setting up terrain physics: " << width << "x" << height << " tiles" << std::endl;
    
    // Create triangle mesh from heightfield
    for (int y = 0; y < height - 1; y++) {
        for (int x = 0; x < width - 1; x++) {
            // Get height values for the four corners of this tile
            float h1 = mapFile->getPlaceGroundHeight(x, y);
            float h2 = mapFile->getPlaceGroundHeight(x + 1, y);
            float h3 = mapFile->getPlaceGroundHeight(x, y + 1);
            float h4 = mapFile->getPlaceGroundHeight(x + 1, y + 1);
            
            // Convert to world coordinates
            float worldX1 = x * tileLength;
            float worldZ1 = y * tileLength;
            float worldX2 = (x + 1) * tileLength;
            float worldZ2 = (y + 1) * tileLength;
            
            // Create two triangles for each tile
            // Triangle 1: (x,y), (x+1,y), (x,y+1)
            m_terrainMesh->addTriangle(
                btVector3(worldX1, h1, worldZ1),
                btVector3(worldX2, h2, worldZ1),
                btVector3(worldX1, h3, worldZ2)
            );
            
            // Triangle 2: (x+1,y), (x+1,y+1), (x,y+1)
            m_terrainMesh->addTriangle(
                btVector3(worldX2, h2, worldZ1),
                btVector3(worldX2, h4, worldZ2),
                btVector3(worldX1, h3, worldZ2)
            );
        }
    }
    
    // Create collision shape from mesh
    m_terrainShape = new btBvhTriangleMeshShape(m_terrainMesh, true);
    
    // Create rigid body for terrain (static)
    m_terrainBody = createStaticBody(m_terrainShape, glm::vec3(0, 0, 0));
    
    // Store terrain object info
    CollisionObjectInfo terrainInfo;
    terrainInfo.type = CollisionObjectType::TERRAIN;
    terrainInfo.objectName = "Terrain";
    m_objectInfoMap[m_terrainBody] = terrainInfo;
    
    std::cout << "Terrain physics setup complete with " << m_terrainMesh->getNumTriangles() << " triangles" << std::endl;
}

void CEPhysicsWorld::setupWorldObjects(C2MapRscFile* mapRsc)
{
    if (!mapRsc) return;
    
    int objectCount = mapRsc->getWorldModelCount();
    std::cout << "Setting up " << objectCount << " world object types for optimized physics" << std::endl;
    
    // Performance: Maximum collision range (~50 tiles as requested)
    float tileLength = m_mapFile->getTileLength(); // Get current tile size
    float maxCollisionRange = 50.0f * tileLength; // 50 tiles * current tile size
    float maxCollisionRangeSq = maxCollisionRange * maxCollisionRange;
    
    for (int i = 0; i < objectCount; i++) {
        CEWorldModel* model = mapRsc->getWorldModel(i);
        if (!model) continue;
        
        // Get object dimensions from TObjInfo (same as bounding box visualization)
        auto objectInfo = model->getObjectInfo();
        float radius = objectInfo->Radius;
        float yLo = objectInfo->YLo;
        float yHi = objectInfo->YHi;
        
        // Skip objects with no valid dimensions
        if (radius <= 0) {
            std::cout << "  Object " << i << " has no valid radius (" << radius << ") - skipping" << std::endl;
            continue;
        }
        
        // Debug: Log object processing
        if (i < 5) { // Log first 5 objects
            std::cout << "  📦 Processing object " << i << " with raw_radius=" << radius << " half_radius=" << (radius * 0.5f) << " height=" << (yHi - yLo) << " yLo=" << yLo << " yHi=" << yHi << " instances=" << model->getTransforms().size() << std::endl;
        }
        
        // Create cylindrical AABB using halved radius (same as visualization and contact filtering)
        float halfRadius = radius * 0.5f; // Match the visualization and contact filtering
        
        // Clamp height to reasonable values - many Carnivores objects have unrealistic height bounds
        // Most trees/rocks should be under 100 units tall in the scaled world
        float objectHeight = yHi - yLo;
        const float MAX_REASONABLE_HEIGHT = 100.0f; // Reasonable max height for objects
        
        if (objectHeight > MAX_REASONABLE_HEIGHT) {
            std::cout << "  ⚠️ Object " << i << " height " << objectHeight << " clamped to " << MAX_REASONABLE_HEIGHT << std::endl;
            // Keep the bottom, but clamp the top
            yHi = yLo + MAX_REASONABLE_HEIGHT;
        }
        
        glm::vec3 aabbMin(-halfRadius, yLo, -halfRadius);
        glm::vec3 aabbMax(halfRadius, yHi, halfRadius);
        
        // Calculate size and center (keep original size since positions are in original coordinate space)
        glm::vec3 size = (aabbMax - aabbMin) * 0.5f;
        glm::vec3 center = (aabbMax + aabbMin) * 0.5f;
        
        btBoxShape* aabbShape = new btBoxShape(btVector3(size.x, size.y, size.z));
        
        // Use the actual radius from object info to determine inner collision bounds
        glm::vec3 innerSize = size; // Default fallback
        
        // Get the object radius that defines the actual collision geometry
        TObjInfo* objInfo = model->getObjectInfo();
        if (objInfo && objInfo->Radius > 0) {
            // Use the object's radius to create inner collision geometry
            // The radius defines the actual collision boundary (like tree trunk radius)
            float objectRadius = objInfo->Radius * 0.5f; // Half radius as used in physics setup
            // Apply the same scaling as visual renderer: YHi and YLo are already *2, then scaled by 0.0625f
            float objectHeight = (objInfo->YHi - objInfo->YLo) * 0.0625f; // Match visual scaling
            
            // Create inner collision box based on actual object radius and height  
            innerSize.x = objectRadius;
            innerSize.y = objectHeight * 0.5f;  // Half height for box size
            innerSize.z = objectRadius;
        }
        
        // Create rigid bodies for each instance of this object
        const auto& transforms = model->getTransforms();
        int instanceIndex = 0;
        int culledInstances = 0;
        
        std::cout << "  Object " << i << " AABB: size[" << size.x << "," << size.y << "," << size.z 
                  << "] center[" << center.x << "," << center.y << "," << center.z 
                  << "] instances:" << transforms.size() << std::endl;
        
        for (const auto& transform : transforms) {
            glm::vec3 position = *const_cast<Transform&>(transform).GetPos();
            
            // Performance optimization: Only create collision for objects within shooting range
            // For now, disable distance culling entirely to ensure collision objects are created
            // TODO: Re-enable with proper player position-based culling later
            bool skipDistanceCheck = true; // Temporarily disable culling for debugging
            float distanceSq = 0; // Will be calculated if skipDistanceCheck is false
            
            if (!skipDistanceCheck && distanceSq > maxCollisionRangeSq) {
                culledInstances++;
                // Debug: Log first few culled instances
                if (culledInstances <= 3) {
                    float distance = sqrt(distanceSq);
                    std::cout << "    🚫 Culling instance at [" << position.x << "," << position.y << "," << position.z << "] distance=" << distance << " (max=" << maxCollisionRange << ")" << std::endl;
                }
                continue; // Skip distant objects for performance
            }
            
            // Position collision boxes using the EXACT same transform logic as visual rendering
            // Now using original GetObjectH algorithm for PLACEGROUND objects
            glm::vec3 visualPosition;
            if (objInfo->flags & objectPLACEGROUND) {
                // Use same algorithm as TerrainRenderer: getObjectHeightForRadius(x, y, GrRad)
                int tileX = (int)(position.x / 16.0f); // Convert world pos to tile coordinates
                int tileZ = (int)(position.z / 16.0f);
                float objectH = m_mapFile->getObjectHeightForRadius(tileX, tileZ, objInfo->GrRad);
                visualPosition = glm::vec3(position.x, objectH, position.z);
            } else {
                // Non-PLACEGROUND objects use the object height map
                visualPosition = position; // position.y already contains correct height from object height map
            }
            
            // Position outer AABB using the same logic as before (with center offset for AABB bounds)
            glm::vec3 aabbPosition = position + center;
            
            // Position inner box to match exactly where the visual object is rendered
            // For Bullet Physics, the position represents the CENTER of the box
            // For PLACEGROUND objects: visualPosition is at ground level, so we need to center the box above ground
            // For non-PLACEGROUND objects: visualPosition already includes YLo offset, so we center from there
            glm::vec3 innerPosition = visualPosition;
            innerPosition.y += innerSize.y; // Center the collision box (Bullet requires center position)
            btRigidBody* objectBody = createStaticBody(aabbShape, aabbPosition);
            
            // Debug: Log AABB details for problematic instances
            if (i == 2 && instanceIndex < 3) { // Log first few instances of object 2
                std::cout << "    📊 Object " << i << " Instance " << instanceIndex 
                          << " raw_position[" << position.x << "," << position.y << "," << position.z << "]"
                          << " aabb_position[" << aabbPosition.x << "," << aabbPosition.y << "," << aabbPosition.z << "]"
                          << " aabb_size[" << size.x << "," << size.y << "," << size.z << "]" << std::endl;
                          
                // Show the actual AABB bounds in world space
                glm::vec3 worldMin = aabbPosition - size;
                glm::vec3 worldMax = aabbPosition + size;
                std::cout << "    📐 World AABB: min[" << worldMin.x << "," << worldMin.y << "," << worldMin.z 
                          << "] max[" << worldMax.x << "," << worldMax.y << "," << worldMax.z << "]" << std::endl;
            }
            
            // Add collision filtering for OUTER objects - make them invisible to raycasts
            // They exist for other physics purposes but projectiles should pass through to inner geometry
            short outerObjectGroup = OBJECT_GROUP;    // Object collision group
            short outerObjectMask = 0; // Don't collide with anything - invisible to raycasts
            m_dynamicsWorld->removeRigidBody(objectBody);
            m_dynamicsWorld->addRigidBody(objectBody, outerObjectGroup, outerObjectMask);
            
            // Debug: Log first few collision bodies
            if (m_objectBodies.size() < 3) {
                std::cout << "🎯 Created collision body " << m_objectBodies.size() + 1 << " at position [" 
                          << aabbPosition.x << ", " << aabbPosition.y << ", " << aabbPosition.z << "] with size [" 
                          << size.x << ", " << size.y << ", " << size.z << "]" << std::endl;
            }
            
            m_objectBodies.push_back(objectBody);
            
            // Store enhanced object info for hierarchical collision (OUTER BOUND box)
            CollisionObjectInfo objectInfo;
            objectInfo.type = CollisionObjectType::WORLD_OBJECT;
            objectInfo.objectIndex = i;
            objectInfo.instanceIndex = instanceIndex;
            objectInfo.objectName = "WorldObject_" + std::to_string(i) + "_Instance_" + std::to_string(instanceIndex);
            objectInfo.worldModel = model; // Store reference for narrowphase testing
            objectInfo.instanceTransform = position;
            objectInfo.aabbCenter = center;
            m_objectInfoMap[objectBody] = objectInfo;
            
            // Create INNER collision box (solid geometry - the "yellow box") - one per instance
            btBoxShape* innerShape = new btBoxShape(btVector3(innerSize.x, innerSize.y, innerSize.z));
            btRigidBody* innerBody = createStaticBody(innerShape, innerPosition);
            
            // Add collision filtering for INNER objects - these should be hit by raycasts
            short innerObjectGroup = OBJECT_GROUP;    // Object collision group  
            short innerObjectMask = -1; // Collide with everything (so raycasts can hit them)
            m_dynamicsWorld->removeRigidBody(innerBody);
            m_dynamicsWorld->addRigidBody(innerBody, innerObjectGroup, innerObjectMask);
            
            // Debug: Log inner box positioning relative to outer box
            if (i < 10 && instanceIndex < 5) { // Expand logging to catch tree objects
                glm::vec3 innerWorldMin = innerPosition - innerSize;
                glm::vec3 innerWorldMax = innerPosition + innerSize;
                glm::vec3 outerWorldMin = aabbPosition - size;
                glm::vec3 outerWorldMax = aabbPosition + size;
                
                std::cout << "    🟡 INNER box " << instanceIndex << " for object " << i << ":" << std::endl;
                std::cout << "       Transform pos: [" << position.x << "," << position.y << "," << position.z << "] YLo=" << objInfo->YLo << " (scaled=" << (objInfo->YLo * 0.0625f) << ")" << std::endl;
                std::cout << "       PLACEGROUND flag: " << ((objInfo->flags & objectPLACEGROUND) ? "YES" : "NO") << " (flags=0x" << std::hex << objInfo->flags << std::dec << ")" << std::endl;
                std::cout << "       Visual pos: [" << visualPosition.x << "," << visualPosition.y << "," << visualPosition.z << "]" << std::endl;
                std::cout << "       Physics pos: [" << innerPosition.x << "," << innerPosition.y << "," << innerPosition.z << "]" << std::endl;
                std::cout << "       Size: [" << innerSize.x << "," << innerSize.y << "," << innerSize.z << "] (radius=" << objInfo->Radius << ")" << std::endl;
                std::cout << "       YHi=" << objInfo->YHi << " objectHeight=" << (objInfo->YHi - objInfo->YLo) << " innerSize.y=" << innerSize.y << std::endl;
                std::cout << "       Bounds: min[" << innerWorldMin.x << "," << innerWorldMin.y << "," << innerWorldMin.z << "] max[" << innerWorldMax.x << "," << innerWorldMax.y << "," << innerWorldMax.z << "]" << std::endl;
                std::cout << "    🔲 OUTER box bounds: min[" << outerWorldMin.x << "," << outerWorldMin.y << "," << outerWorldMin.z << "] max[" << outerWorldMax.x << "," << outerWorldMax.y << "," << outerWorldMax.z << "]" << std::endl;
            }
            
            m_objectBodies.push_back(innerBody);
            m_objectShapes.push_back(innerShape); // Store inner shape for cleanup
            
            // Store inner object info - mark as INNER solid geometry
            CollisionObjectInfo innerObjectInfo;
            innerObjectInfo.type = CollisionObjectType::WORLD_OBJECT;
            innerObjectInfo.objectIndex = i;
            innerObjectInfo.instanceIndex = instanceIndex;
            innerObjectInfo.objectName = "WorldObject_" + std::to_string(i) + "_Instance_" + std::to_string(instanceIndex) + "_INNER";
            innerObjectInfo.worldModel = model;
            innerObjectInfo.instanceTransform = position;
            innerObjectInfo.aabbCenter = center;
            m_objectInfoMap[innerBody] = innerObjectInfo;
            
            instanceIndex++;
        }
        
        // Store shape for cleanup (shared by all instances)
        m_objectShapes.push_back(aabbShape);
        
        // Log performance info for this object type
        if (culledInstances > 0) {
            std::cout << "    Performance: " << culledInstances << " instances culled (beyond " << (maxCollisionRange/tileLength) << " tiles)" << std::endl;
        }
    }
    
    std::cout << "Optimized world objects physics setup complete: " << m_objectBodies.size() << " AABB instances" << std::endl;
    std::cout << "Performance: Objects limited to ~" << (maxCollisionRange/tileLength) << " tile radius for collision detection" << std::endl;
    std::cout << "Total collision objects in world: " << m_dynamicsWorld->getNumCollisionObjects() << std::endl;
}

void CEPhysicsWorld::setupWaterPlanes(C2MapFile* mapFile)
{
    if (!mapFile) return;
    
    float tileLength = mapFile->getTileLength();
    int width = mapFile->getWidth();
    int height = mapFile->getHeight();
    
    std::cout << "Setting up water plane physics for " << width << "x" << height << " map" << std::endl;
    
    // For now, create a simplified water collision plane
    // In a more advanced implementation, you could scan the map for actual water tiles
    // and create individual water collision shapes for each water area
    
    // Create a large, thin box shape for general water collision
    float mapWidth = width * tileLength;
    float mapHeight = height * tileLength;
    
    // Create water plane at a low level to avoid interference with terrain shooting
    float estimatedWaterLevel = -500.0f; // Set much lower to avoid false water hits when shooting terrain
    
    btBoxShape* waterShape = new btBoxShape(btVector3(mapWidth / 2, 0.125f, mapHeight / 2)); // Scaled down 16x (was 2.0f)
    
    // Position water plane
    glm::vec3 waterPosition(mapWidth / 2, estimatedWaterLevel, mapHeight / 2);
    btRigidBody* waterBody = createStaticBody(waterShape, waterPosition);
    
    // Set collision filtering to prevent interference with raycast detection
    // Use a specific collision group for water
    short waterGroup = WATER_GROUP;      // Water collision group
    short waterMask = PROJECTILE_GROUP;  // Only collide with projectiles
    m_dynamicsWorld->removeRigidBody(waterBody);
    m_dynamicsWorld->addRigidBody(waterBody, waterGroup, waterMask);
    
    // Store object info for water plane
    CollisionObjectInfo waterInfo;
    waterInfo.type = CollisionObjectType::WATER_PLANE;
    waterInfo.objectName = "Water Plane";
    m_objectInfoMap[waterBody] = waterInfo;
    
    // Store for cleanup
    m_waterShapes.push_back(waterShape);
    m_waterBodies.push_back(waterBody);
    
    std::cout << "Water plane physics setup complete at estimated level: " << estimatedWaterLevel << std::endl;
}

btRigidBody* CEPhysicsWorld::createStaticBody(btCollisionShape* shape, const glm::vec3& position)
{
    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(btVector3(position.x, position.y, position.z));
    
    // Static bodies have zero mass
    btVector3 localInertia(0, 0, 0);
    
    btDefaultMotionState* motionState = new btDefaultMotionState(transform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(0, motionState, shape, localInertia);
    btRigidBody* body = new btRigidBody(rbInfo);
    
    m_dynamicsWorld->addRigidBody(body);
    return body;
}

btRigidBody* CEPhysicsWorld::createProjectile(const glm::vec3& position, const glm::vec3& velocity, float mass)
{
    // Create small sphere for bullet
    btSphereShape* bulletShape = new btSphereShape(0.005f); // 5mm radius
    
    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(btVector3(position.x, position.y, position.z));
    
    btVector3 localInertia;
    bulletShape->calculateLocalInertia(mass, localInertia);
    
    btDefaultMotionState* motionState = new btDefaultMotionState(transform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, bulletShape, localInertia);
    btRigidBody* body = new btRigidBody(rbInfo);
    
    // Set initial velocity
    body->setLinearVelocity(btVector3(velocity.x, velocity.y, velocity.z));
    
    // Add to world
    m_dynamicsWorld->addRigidBody(body);
    
    return body;
}

void CEPhysicsWorld::stepSimulation(float deltaTime)
{
    if (m_dynamicsWorld) {
        // Optimized physics for scaled world (64 units/tile vs 256)
        // 240Hz provides excellent collision detection at reasonable performance cost
        float fixedTimeStep = 1.0f / 60.0f; // 60-240 Hz physics for scaled world
        int maxSubSteps = 4; // Reasonable substeps for good performance
        
        m_dynamicsWorld->stepSimulation(deltaTime, maxSubSteps, fixedTimeStep);
    }
}

CEPhysicsWorld::RaycastResult CEPhysicsWorld::raycast(const glm::vec3& from, const glm::vec3& to)
{
    RaycastResult result;
    
    // Validate ray
    float rayDistance = glm::distance(from, to);
    if (rayDistance < 0.001f) {
        result.hasHit = false;
        return result;
    }
    
    btVector3 btFrom(from.x, from.y, from.z);
    btVector3 btTo(to.x, to.y, to.z);
    
    btCollisionWorld::ClosestRayResultCallback rayCallback(btFrom, btTo);
    
    // Simple collision filtering - hit all visible objects
    rayCallback.m_collisionFilterGroup = PROJECTILE_GROUP;
    rayCallback.m_collisionFilterMask = TERRAIN_GROUP | OBJECT_GROUP | WATER_GROUP;
    
    m_dynamicsWorld->rayTest(btFrom, btTo, rayCallback);
    
    if (rayCallback.hasHit()) {
        result.hasHit = true;
        result.hitPoint = glm::vec3(
            rayCallback.m_hitPointWorld.getX(),
            rayCallback.m_hitPointWorld.getY(),
            rayCallback.m_hitPointWorld.getZ()
        );
        result.hitNormal = glm::vec3(
            rayCallback.m_hitNormalWorld.getX(),
            rayCallback.m_hitNormalWorld.getY(),
            rayCallback.m_hitNormalWorld.getZ()
        );
        result.hitBody = const_cast<btRigidBody*>(btRigidBody::upcast(rayCallback.m_collisionObject));
        result.distance = glm::distance(from, result.hitPoint);
        
        // Look up object information
        auto it = m_objectInfoMap.find(result.hitBody);
        if (it != m_objectInfoMap.end()) {
            result.objectInfo = it->second;
        } else {
            result.objectInfo.type = CollisionObjectType::TERRAIN;
            result.objectInfo.objectName = "Unknown";
        }
    }
    
    return result;
}

bool CEPhysicsWorld::hasContacts(btRigidBody* body)
{
    // This method is no longer used in the simplified collision system
    return false;
}

CEPhysicsWorld::ContactInfo CEPhysicsWorld::getFilteredContacts(btRigidBody* body)
{
    ContactInfo result;
    // This method is no longer used in the simplified collision system
    return result;
}

void CEPhysicsWorld::updateHeightfieldForPosition(const glm::vec3& position)
{
    if (m_heightfieldTerrain) {
        m_heightfieldTerrain->updateForPosition(position, m_dynamicsWorld);
    }
}

void CEPhysicsWorld::removeRigidBody(btRigidBody* body)
{
    if (body && m_dynamicsWorld) {
        m_dynamicsWorld->removeRigidBody(body);
        
        if (body->getMotionState()) {
            delete body->getMotionState();
        }
        
        if (body->getCollisionShape()) {
            delete body->getCollisionShape();
        }
        
        delete body;
    }
}
