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
#include "CEBulletDebugDraw.h"
#include "transform.h"
#include "vertex.h"

// Bullet Physics includes
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>
#include <BulletCollision/CollisionShapes/btScaledBvhTriangleMeshShape.h>
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
    // Use much stronger gravity for snappy FPS player movement feel
    // Bullets can have separate physics properties if needed
    float gameGravity = -115.3f; // Realistic Earth gravity (9.8 m/s² scaled to 1 unit = 8.5cm)
    m_dynamicsWorld->setGravity(btVector3(0, gameGravity, 0));
    
    // Initialize debug drawer for physics visualization
    m_debugDrawer = std::make_unique<CEBulletDebugDraw>();
    m_dynamicsWorld->setDebugDrawer(m_debugDrawer.get());
    
    // Setup collision geometry (terrain and world objects optimized for performance)
    // setupTerrain(mapFile);  // DISABLED: Too expensive, using height checks instead
    setupHeightfieldTerrain(mapFile);  // NEW: Bullet Physics heightfield terrain
    setupWorldObjects(mapRsc);  // RE-ENABLED: Optimized AABB-based hierarchical collision
    setupWaterPlanes(mapFile);  // RE-ENABLED: For water collision detection
    
}

CEPhysicsWorld::~CEPhysicsWorld()
{
    // Remove heightfield terrain from world before cleanup
    if (m_heightfieldTerrain) {
        m_heightfieldTerrain->removeFromWorld(m_dynamicsWorld);
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
    for (auto* baseShape : m_baseBvhShapes) {
        delete baseShape;
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
    
    
    // Create single terrain triangle mesh with exact subdivision matching
    m_heightfieldTerrain = std::make_unique<CEBulletHeightfield>(mapFile);
    
    // Add terrain mesh to physics world
    m_heightfieldTerrain->addToWorld(m_dynamicsWorld);
    
    // Register terrain body in object info map for proper collision detection
    const auto* terrainMesh = m_heightfieldTerrain->getTerrainMesh();
    if (terrainMesh && terrainMesh->terrainBody) {
        CollisionObjectInfo terrainInfo;
        terrainInfo.type = CollisionObjectType::HEIGHTFIELD_TERRAIN;
        terrainInfo.objectName = "TerrainTriangleMesh";
        m_objectInfoMap[terrainMesh->terrainBody] = terrainInfo;
    }
    
    int totalTriangles = m_heightfieldTerrain->getTriangleCount();
}

void CEPhysicsWorld::setupTerrain(C2MapFile* mapFile)
{
    if (!mapFile) return;
    
    m_terrainMesh = new btTriangleMesh();
    
    float tileLength = mapFile->getTileLength();
    int width = mapFile->getWidth();
    int height = mapFile->getHeight();
    
    
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
    
}

void CEPhysicsWorld::setupWorldObjects(C2MapRscFile* mapRsc)
{
    if (!mapRsc) return;
    
    int objectCount = mapRsc->getWorldModelCount();
    
    // Performance: Maximum collision range
    float tileLength = m_mapFile->getTileLength();
    float maxCollisionRange = 300.0f; // Reasonable range for bullet collision detection
    float maxCollisionRangeSq = maxCollisionRange * maxCollisionRange;
    
    for (int i = 0; i < objectCount; i++) {
        CEWorldModel* model = mapRsc->getWorldModel(i);
        if (!model) continue;
        
        // Get object dimensions
        auto objectInfo = model->getObjectInfo();
        float radius = objectInfo->Radius;
        
        // Skip objects with no valid dimensions
        if (radius <= 0) {
            continue;
        }
        
        // Create triangle mesh from geometry (shared by all instances)
        btTriangleMesh* triangleMesh = model->getGeometry()->createPhysicsTriangleMesh();
        if (!triangleMesh || triangleMesh->getNumTriangles() == 0) {
            delete triangleMesh;
            continue;
        }
        
        // Create base BVH triangle mesh shape (shared by all instances)
        btBvhTriangleMeshShape* baseBvhShape = new btBvhTriangleMeshShape(triangleMesh, true);
        
        // Store base shape and mesh for cleanup
        m_objectMeshes.push_back(triangleMesh);
        m_baseBvhShapes.push_back(baseBvhShape);
        
        
        // Create instances using btScaledBvhTriangleMeshShape
        const auto& transforms = model->getTransforms();
        int instanceIndex = 0;
        int culledInstances = 0;
        
        
        for (const auto& transform : transforms) {
            glm::vec3 position = *const_cast<Transform&>(transform).GetPos();
            glm::vec3 rotation = *const_cast<Transform&>(transform).GetRot(); // Extract rotation!

            // Calculate proper object position (same as visual rendering)
            glm::vec3 objectPosition;
            TObjInfo* objInfo = model->getObjectInfo();
            if (objInfo->flags & objectPLACEGROUND) {
                int tileX = (int)(position.x / 16.0f);
                int tileZ = (int)(position.z / 16.0f);
                float objectH = m_mapFile->getObjectHeightForRadius(tileX, tileZ, objInfo->GrRad);
                objectPosition = glm::vec3(position.x, objectH, position.z);
            } else {
                objectPosition = position;
            }
            
            // Create scaled instance of the base BVH shape
            // Use the scaling that matches visual rendering
            btVector3 scale(0.0625f, 0.0625f, 0.0625f);
            btScaledBvhTriangleMeshShape* scaledShape = new btScaledBvhTriangleMeshShape(baseBvhShape, scale);
            
            // Create rigid body for this instance WITH ROTATION to match visual rendering
            btRigidBody* instanceBody = createStaticBody(scaledShape, objectPosition, rotation);
            
            // Set collision filtering
            short objectGroup = OBJECT_GROUP;
            short objectMask = -1; // Collide with everything
            m_dynamicsWorld->removeRigidBody(instanceBody);
            m_dynamicsWorld->addRigidBody(instanceBody, objectGroup, objectMask);
            
            // Store for cleanup
            m_objectShapes.push_back(scaledShape);
            m_objectBodies.push_back(instanceBody);
            
            // Store object info
            CollisionObjectInfo instanceInfo;
            instanceInfo.type = CollisionObjectType::WORLD_OBJECT;
            instanceInfo.objectIndex = i;
            instanceInfo.instanceIndex = instanceIndex;
            instanceInfo.objectName = "ScaledBVH_" + std::to_string(i) + "_Instance_" + std::to_string(instanceIndex);
            instanceInfo.worldModel = model;
            instanceInfo.instanceTransform = objectPosition;
            m_objectInfoMap[instanceBody] = instanceInfo;
            
            instanceIndex++;
        }
        
    }
    
}

void CEPhysicsWorld::setupWaterPlanes(C2MapFile* mapFile)
{
    if (!mapFile) return;
    
    float tileLength = mapFile->getTileLength();
    int width = mapFile->getWidth();
    int height = mapFile->getHeight();
    
    
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

btRigidBody* CEPhysicsWorld::createStaticBody(btCollisionShape* shape, const glm::vec3& position, const glm::vec3& rotation)
{
    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(btVector3(position.x, position.y, position.z));
    
    // Convert Euler angles (GLM) to Bullet quaternion
    // Transform uses X,Y,Z rotation order
    btQuaternion quat;
    quat.setEulerZYX(rotation.z, rotation.y, rotation.x); // Bullet uses Z,Y,X order
    transform.setRotation(quat);
    
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
    rayCallback.m_collisionFilterMask = TERRAIN_GROUP | OBJECT_GROUP | WATER_GROUP | AI_GROUP;
    
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
    // No longer needed with single mesh - Bullet's BVH handles spatial optimization internally
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

void CEPhysicsWorld::enablePhysicsDebugRendering(bool enable)
{
    if (!m_debugDrawer) return;
    
    if (enable) {
        // Enable minimal debug drawing for performance
        // Only show wireframes - AABBs and contact points are too expensive for large scenes
        int debugMode = btIDebugDraw::DBG_DrawWireframe;
        
        m_debugDrawer->setDebugMode(debugMode);
    } else {
        m_debugDrawer->setDebugMode(btIDebugDraw::DBG_NoDebug);
    }
}

void CEPhysicsWorld::renderPhysicsDebug(const glm::mat4& viewProjectionMatrix, const glm::vec3& cameraPosition)
{
    if (!m_debugDrawer || !m_dynamicsWorld) return;
    
    // Set view-projection matrix and camera position for proper 3D rendering and culling
    m_debugDrawer->setViewProjectionMatrix(viewProjectionMatrix);
    m_debugDrawer->setCameraPosition(cameraPosition);
    
    // Begin debug frame
    m_debugDrawer->beginFrame();
    
    // Optimized selective debug drawing with distance culling
    int totalObjects = m_dynamicsWorld->getNumCollisionObjects();
    int drawnObjects = 0;
    int skippedTerrain = 0;
    int skippedDistance = 0;
    int skippedTerrainTriangles = 0;
    
    const float maxTerrainDebugDistance = 100.0f; // Only show terrain within 100 units of camera
    
    for (int i = 0; i < totalObjects; i++) {
        btCollisionObject* obj = m_dynamicsWorld->getCollisionObjectArray()[i];
        btVector3 objOrigin = obj->getWorldTransform().getOrigin();
        glm::vec3 objectPos(objOrigin.getX(), objOrigin.getY(), objOrigin.getZ());
        
        // Check if this is a terrain partition
        auto it = m_objectInfoMap.find(static_cast<btRigidBody*>(obj));
        bool isTerrain = (it != m_objectInfoMap.end() && it->second.type == CollisionObjectType::HEIGHTFIELD_TERRAIN);
        
        if (isTerrain) {
            // For terrain: check if camera is within reasonable range for debug rendering
            const auto* terrainMesh = m_heightfieldTerrain->getTerrainMesh();
            if (terrainMesh) {
                // Check if camera is within terrain bounds (inside or above the map)
                bool cameraInsideTerrainBounds = (
                    cameraPosition.x >= terrainMesh->worldMin.x && cameraPosition.x <= terrainMesh->worldMax.x &&
                    cameraPosition.z >= terrainMesh->worldMin.z && cameraPosition.z <= terrainMesh->worldMax.z
                );
                
                // If camera is outside terrain bounds, check distance to nearest edge
                float distanceToTerrain = 0.0f;
                if (!cameraInsideTerrainBounds) {
                    glm::vec3 closestPoint = glm::clamp(cameraPosition, terrainMesh->worldMin, terrainMesh->worldMax);
                    distanceToTerrain = glm::distance(cameraPosition, closestPoint);
                }
                
                // Show terrain if camera is inside bounds OR within distance limit
                bool showTerrain = cameraInsideTerrainBounds || (distanceToTerrain <= maxTerrainDebugDistance);
                
                if (!showTerrain) {
                    skippedTerrain++;
                    continue; // Skip terrain rendering
                }
                
                // Terrain is being rendered - count its triangles for debug info
                if (terrainMesh->terrainBody == obj) {
                    skippedTerrainTriangles += terrainMesh->triangleCount;
                }
            }
        }
        
        // Draw this collision object  
        btVector3 color = isTerrain ? btVector3(0.0f, 1.0f, 0.0f) : btVector3(1.0f, 1.0f, 0.0f); // Green for terrain, yellow for objects
        m_dynamicsWorld->debugDrawObject(obj->getWorldTransform(), obj->getCollisionShape(), color);
        drawnObjects++;
    }
    
    
    // Render accumulated debug lines
    m_debugDrawer->endFrame();
}

void CEPhysicsWorld::registerCollisionObject(btRigidBody* body, const CollisionObjectInfo& info)
{
    if (body) {
        m_objectInfoMap[body] = info;
    }
}
