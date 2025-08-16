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
    m_dynamicsWorld->setGravity(btVector3(0, -9.81f, 0)); // Earth gravity
    
    // Setup collision geometry (terrain and world objects optimized for performance)
    // setupTerrain(mapFile);  // DISABLED: Too expensive, using height checks instead
    setupWorldObjects(mapRsc);  // RE-ENABLED: Optimized AABB-based hierarchical collision
    setupWaterPlanes(mapFile);  // RE-ENABLED: For water collision detection
    
    std::cout << "Physics world initialized with " << m_dynamicsWorld->getNumCollisionObjects() << " collision objects" << std::endl;
}

CEPhysicsWorld::~CEPhysicsWorld()
{
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
    float maxCollisionRange = 50.0f * 256.0f; // 50 tiles * tile size
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
            std::cout << "  📦 Processing object " << i << " with radius=" << radius << " height=" << (yHi - yLo) << " instances=" << model->getTransforms().size() << std::endl;
        }
        
        // Create cylindrical AABB using radius and height (same as visualization)
        glm::vec3 aabbMin(-radius, yLo, -radius);
        glm::vec3 aabbMax(radius, yHi, radius);
        
        // Calculate size and center
        glm::vec3 size = (aabbMax - aabbMin) * 0.5f;
        glm::vec3 center = (aabbMax + aabbMin) * 0.5f;
        
        btBoxShape* aabbShape = new btBoxShape(btVector3(size.x, size.y, size.z));
        
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
            
            // Position the AABB at the transform position plus center offset
            glm::vec3 aabbPosition = position + center;
            btRigidBody* objectBody = createStaticBody(aabbShape, aabbPosition);
            
            // Add collision filtering for objects
            short objectGroup = 1 << 2;  // Object collision group
            short objectMask = 1 << 0;   // Only collide with projectiles (group 0)
            m_dynamicsWorld->removeRigidBody(objectBody);
            m_dynamicsWorld->addRigidBody(objectBody, objectGroup, objectMask);
            
            // Debug: Log first few collision bodies
            if (m_objectBodies.size() < 3) {
                std::cout << "🎯 Created collision body " << m_objectBodies.size() + 1 << " at position [" 
                          << aabbPosition.x << ", " << aabbPosition.y << ", " << aabbPosition.z << "] with size [" 
                          << size.x << ", " << size.y << ", " << size.z << "]" << std::endl;
            }
            
            m_objectBodies.push_back(objectBody);
            
            // Store enhanced object info for hierarchical collision
            CollisionObjectInfo objectInfo;
            objectInfo.type = CollisionObjectType::WORLD_OBJECT;
            objectInfo.objectIndex = i;
            objectInfo.instanceIndex = instanceIndex;
            objectInfo.objectName = "WorldObject_" + std::to_string(i) + "_Instance_" + std::to_string(instanceIndex);
            objectInfo.worldModel = model; // Store reference for narrowphase testing
            objectInfo.instanceTransform = position;
            objectInfo.aabbCenter = center;
            m_objectInfoMap[objectBody] = objectInfo;
            
            instanceIndex++;
        }
        
        // Store shape for cleanup (shared by all instances)
        m_objectShapes.push_back(aabbShape);
        
        // Log performance info for this object type
        if (culledInstances > 0) {
            std::cout << "    Performance: " << culledInstances << " instances culled (beyond " << (maxCollisionRange/256.0f) << " tiles)" << std::endl;
        }
    }
    
    std::cout << "Optimized world objects physics setup complete: " << m_objectBodies.size() << " AABB instances" << std::endl;
    std::cout << "Performance: Objects limited to ~" << (maxCollisionRange/256.0f) << " tile radius for collision detection" << std::endl;
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
    
    // Create water plane at a reasonable water level (you can adjust this)
    float estimatedWaterLevel = 1000.0f; // Set higher to avoid interference with typical terrain heights
    
    btBoxShape* waterShape = new btBoxShape(btVector3(mapWidth / 2, 2.0f, mapHeight / 2)); // Thicker water plane
    
    // Position water plane
    glm::vec3 waterPosition(mapWidth / 2, estimatedWaterLevel, mapHeight / 2);
    btRigidBody* waterBody = createStaticBody(waterShape, waterPosition);
    
    // Set collision filtering to prevent interference with raycast detection
    // Use a specific collision group for water
    short waterGroup = 1 << 1;  // Water collision group
    short waterMask = 1 << 0;   // Only collide with projectiles (group 0)
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
        // Use reasonable time steps for good performance while maintaining accuracy
        // Reduced from 240Hz to 60Hz for much better performance
        float fixedTimeStep = 1.0f / 60.0f; // 60 Hz physics for good balance
        int maxSubSteps = 4; // Reduced substeps for better performance
        
        m_dynamicsWorld->stepSimulation(deltaTime, maxSubSteps, fixedTimeStep);
    }
}

CEPhysicsWorld::RaycastResult CEPhysicsWorld::raycast(const glm::vec3& from, const glm::vec3& to)
{
    RaycastResult result;
    
    btVector3 btFrom(from.x, from.y, from.z);
    btVector3 btTo(to.x, to.y, to.z);
    
    btCollisionWorld::ClosestRayResultCallback rayCallback(btFrom, btTo);
    
    // Set collision filtering to match projectile collision groups
    rayCallback.m_collisionFilterGroup = 1 << 0; // Projectile group (group 0)
    rayCallback.m_collisionFilterMask = (1 << 1) | (1 << 2); // Can hit water (group 1) and objects (group 2)
    
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
            // Fallback if not found in map
            result.objectInfo.type = CollisionObjectType::TERRAIN;
            result.objectInfo.objectName = "Unknown";
        }
    }
    
    return result;
}

bool CEPhysicsWorld::hasContacts(btRigidBody* body)
{
    if (!body || !m_dynamicsWorld) {
        return false;
    }
    
    // Check if this body has any contact manifolds
    int numManifolds = m_dynamicsWorld->getDispatcher()->getNumManifolds();
    
    // Debug: log manifold checking periodically
    static double lastManifoldLog = 0;
    double currentTime = glfwGetTime();
    if (currentTime - lastManifoldLog > 1.0) { // Log every second
        std::cout << "🔍 Checking " << numManifolds << " contact manifolds" << std::endl;
        lastManifoldLog = currentTime;
    }
    
    for (int i = 0; i < numManifolds; i++) {
        btPersistentManifold* contactManifold = m_dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
        
        const btCollisionObject* objA = contactManifold->getBody0();
        const btCollisionObject* objB = contactManifold->getBody1();
        
        // Check if either object in this manifold is our body
        if (objA == body || objB == body) {
            // Check if there are actual contact points
            int numContacts = contactManifold->getNumContacts();
            if (numContacts > 0) {
                std::cout << "🎯 Contact detected! Body has " << numContacts << " contact points" << std::endl;
                return true;
            }
        }
    }
    
    return false;
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
