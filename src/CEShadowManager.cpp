#include "CEShadowManager.h"
#include "shader_program.h"
#include "CEWorldModel.h"
#include "CEGeometry.h"
#include "transform.h"
#include "g_shared.h"
#include "DebugConfig.h"
#include <glad/glad.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
namespace fs = std::filesystem;

// Static member definition
const float CEShadowManager::SHADOW_DISTANCE = 500.0f;

CEShadowManager::CEShadowManager()
    : m_shadow_framebuffer(0)
    , m_shadow_depth_texture(0)
    , m_light_direction(0.5f, -1.0f, 0.3f)
    , m_light_position(0.0f, 1000.0f, 0.0f)
{
}

CEShadowManager::~CEShadowManager()
{
    if (m_shadow_framebuffer != 0) {
        glDeleteFramebuffers(1, &m_shadow_framebuffer);
    }
    if (m_shadow_depth_texture != 0) {
        glDeleteTextures(1, &m_shadow_depth_texture);
    }
}

void CEShadowManager::initialize()
{
    std::cout << "Initializing shadow manager..." << std::endl;
    
    setupShadowFramebuffer();
    
    // Check for OpenGL errors after framebuffer setup
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "OpenGL error after framebuffer setup: " << err << std::endl;
        return;
    }
    
    // Load shadow generation shader using the same pattern as TerrainRenderer
    try {
        std::ifstream f("config.json");
        if (!f.is_open()) {
            std::cerr << "Failed to open config.json for shader path" << std::endl;
            return;
        }
        
        json data = json::parse(f);
        fs::path basePath = fs::path(data["basePath"].get<std::string>());
        fs::path shaderPath = basePath / "shaders";
        
        std::string vsPath = (shaderPath / "shadow_depth.vs").string();
        std::string fsPath = (shaderPath / "shadow_depth.fs").string();
        
        std::cout << "Loading shadow shaders from: " << vsPath << " and " << fsPath << std::endl;
        
        m_shadow_shader = std::unique_ptr<ShaderProgram>(new ShaderProgram(vsPath, fsPath));
        std::cout << "Shadow shader loaded successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load shadow shader: " << e.what() << std::endl;
        return;
    }
    
    // Check for OpenGL errors after shader loading
    err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "OpenGL error after shader loading: " << err << std::endl;
        return;
    }
    
    // Set default light direction (similar to terrain shader)
    setLightDirection(glm::normalize(glm::vec3(0.5f, -1.0f, 0.3f)));
    
    std::cout << "Shadow manager initialized successfully" << std::endl;
    std::cout << "Shadow depth texture ID: " << m_shadow_depth_texture << std::endl;
}

void CEShadowManager::setupShadowFramebuffer()
{
    std::cout << "Setting up shadow framebuffer..." << std::endl;
    
    // Generate framebuffer
    glGenFramebuffers(1, &m_shadow_framebuffer);
    std::cout << "Generated framebuffer ID: " << m_shadow_framebuffer << std::endl;
    
    // Generate depth texture
    glGenTextures(1, &m_shadow_depth_texture);
    std::cout << "Generated depth texture ID: " << m_shadow_depth_texture << std::endl;
    
    glBindTexture(GL_TEXTURE_2D, m_shadow_depth_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
                 SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 0, 
                 GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "OpenGL error after depth texture creation: " << err << std::endl;
    }
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    
    // Attach depth texture to framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_shadow_framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
                          GL_TEXTURE_2D, m_shadow_depth_texture, 0);
    
    err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "OpenGL error after framebuffer texture attachment: " << err << std::endl;
    }
    
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    
    GLenum framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (framebufferStatus != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Error: Shadow framebuffer not complete! Status: " << framebufferStatus << std::endl;
        switch (framebufferStatus) {
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                std::cerr << "  GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT" << std::endl;
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                std::cerr << "  GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT" << std::endl;
                break;
            case GL_FRAMEBUFFER_UNSUPPORTED:
                std::cerr << "  GL_FRAMEBUFFER_UNSUPPORTED" << std::endl;
                break;
            default:
                std::cerr << "  Unknown framebuffer error: " << framebufferStatus << std::endl;
                break;
        }
    } else {
        std::cout << "Shadow framebuffer setup complete" << std::endl;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void CEShadowManager::setLightDirection(const glm::vec3& direction)
{
    m_light_direction = glm::normalize(direction);
}

void CEShadowManager::setLightPosition(const glm::vec3& position)
{
    m_light_position = position;
}

void CEShadowManager::calculateLightMatrices(const glm::vec3& center, float size)
{
    // Use standard orthographic projection bounds with much larger near/far range
    float halfSize = size;
    float nearPlane = -size * 2.0f;  // Allow objects to be behind the light
    float farPlane = size * 4.0f;
    glm::mat4 lightProjection = glm::ortho(-halfSize, halfSize, -halfSize, halfSize, nearPlane, farPlane);
    
    // Position light high above the scene center and look down at it
    glm::vec3 lightPos = center + glm::vec3(0.0f, size * 2.0f, 0.0f);
    
    // Create view matrix looking down from above the scene center
    glm::mat4 lightView = glm::lookAt(lightPos,                      // Eye position (above scene center)
                                     center,                         // Look at scene center
                                     glm::vec3(0.0f, 0.0f, -1.0f)); // Up vector (north)
    
    m_light_view_matrix = lightView;
    m_light_projection_matrix = lightProjection;
    m_light_space_matrix = lightProjection * lightView;
    m_light_position = lightPos;
    
    DebugConfig& debug = DebugConfig::getInstance();
    
    if (debug.isShadowDebugEnabled()) {
        // Debug: Print the matrices to understand what's happening
        std::cout << "Light matrix debug:" << std::endl;
        std::cout << "  Scene center: (" << center.x << ", " << center.y << ", " << center.z << ")" << std::endl;
        std::cout << "  Scene size: " << size << std::endl;
        std::cout << "  Light position: (" << lightPos.x << ", " << lightPos.y << ", " << lightPos.z << ")" << std::endl;
        std::cout << "  Ortho bounds: [" << -halfSize << " to " << halfSize << "] x [" << -halfSize << " to " << halfSize << "], near=" << nearPlane << ", far=" << farPlane << std::endl;
        
        // Test what coordinates we get for a point at scene center and some offset points
        glm::vec4 testPoint(center.x, center.y, center.z, 1.0f);
        glm::vec4 lightSpacePoint = m_light_space_matrix * testPoint;
        lightSpacePoint /= lightSpacePoint.w; // Perspective divide
        lightSpacePoint = lightSpacePoint * 0.5f + 0.5f; // Transform to [0,1]
        std::cout << "  Scene center transforms to shadow coords: (" 
                  << lightSpacePoint.x << ", " << lightSpacePoint.y << ", " << lightSpacePoint.z << ")" << std::endl;
                  
        // Test points at the edges of our radius to see if they're in bounds
        glm::vec4 edgePoint(center.x + size, center.y, center.z, 1.0f);
        glm::vec4 edgeLightSpace = m_light_space_matrix * edgePoint;
        edgeLightSpace /= edgeLightSpace.w;
        edgeLightSpace = edgeLightSpace * 0.5f + 0.5f;
        std::cout << "  Edge point (+X) transforms to shadow coords: (" 
                  << edgeLightSpace.x << ", " << edgeLightSpace.y << ", " << edgeLightSpace.z << ")" << std::endl;
    }
}

void CEShadowManager::generateShadowMap(const std::vector<CEWorldModel*>& shadowCasters,
                                       const glm::vec3& sceneCenter, 
                                       float sceneRadius)
{
    DebugConfig& debug = DebugConfig::getInstance();
    
    if (debug.isShadowDebugEnabled()) {
        std::cout << "Starting shadow map generation..." << std::endl;
        std::cout << "Shadow casters count: " << shadowCasters.size() << std::endl;
        std::cout << "Scene center: (" << sceneCenter.x << ", " << sceneCenter.y << ", " << sceneCenter.z << ")" << std::endl;
        std::cout << "Scene radius: " << sceneRadius << std::endl;
    }
    
    // Generate cache key for this configuration
    setCacheKey("current_map", shadowCasters);
    
    if (!m_shadow_shader) {
        std::cerr << "Error: Shadow shader not initialized!" << std::endl;
        return;
    }
    
    // Calculate light matrices for the scene
    calculateLightMatrices(sceneCenter, sceneRadius);
    
    // Begin shadow rendering pass
    beginShadowPass();
    
    // DEBUG: Check framebuffer status before rendering anything
    GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fboStatus != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "  ERROR: Shadow framebuffer not complete before rendering: " << fboStatus << std::endl;
    }
    if (debug.isShadowDebugEnabled()) {
        std::cout << "  Shadow framebuffer is active and ready" << std::endl;
    }
    
    int shadowCasterCount = 0;
    int totalTransforms = 0;
    
    // Store scene info for bounds checking
    glm::vec3 storedSceneCenter = sceneCenter;
    float storedSceneRadius = sceneRadius;
    
    // Render all shadow casting objects
    for (CEWorldModel* model : shadowCasters) {
        if (shouldCastShadow(model)) {
            shadowCasterCount++;
            // Render all transforms/instances of this model
            const auto& transforms = model->getTransforms();
            totalTransforms += transforms.size();
            
            // Reduced debug output - just show model count
            if (debug.isShadowDebugEnabled()) {
                std::cout << "  Model " << shadowCasterCount << " has " << transforms.size() << " instances" << std::endl;
            }
            
            int renderedInstances = 0;
            for (const Transform& transform : transforms) {
                // Only render objects within the shadow bounds
                Transform& t = const_cast<Transform&>(transform);
                glm::vec3 objPos = *t.GetPos();
                
                // Check if object is within shadow bounds
                float distFromCenter = glm::distance(glm::vec2(objPos.x, objPos.z), 
                                                   glm::vec2(storedSceneCenter.x, storedSceneCenter.z));
                
                if (distFromCenter <= storedSceneRadius) {
                    // Render ALL objects within bounds - no limits
                    renderShadowCaster(model, transform);
                    renderedInstances++;
                    
                    // No longer need per-instance depth checking
                }
            }
            
            if (debug.isShadowDebugEnabled()) {
                std::cout << "    Model type " << shadowCasterCount << ": rendered " << renderedInstances << " out of " << transforms.size() << " instances within bounds" << std::endl;
            }
        }
    }
    
    if (debug.isShadowDebugEnabled()) {
        std::cout << "Generated shadows for " << shadowCasterCount << " model types with " << totalTransforms << " total instances" << std::endl;
        
        // Quick check if shadow map has depth data
        float centerDepth;
        glReadPixels(SHADOW_MAP_SIZE/2, SHADOW_MAP_SIZE/2, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &centerDepth);
        if (centerDepth >= 0.999f) {
            std::cout << "  Warning: Shadow map may be empty (center depth = " << centerDepth << ")" << std::endl;
        } else {
            std::cout << "  Shadow map generated successfully (center depth = " << centerDepth << ")" << std::endl;
        }
    }
    
    endShadowPass();
    
    // Check for OpenGL errors after shadow generation
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "OpenGL error after shadow generation: " << err << std::endl;
    }
    
    // Cache the generated shadow map
    saveShadowMapCache(m_current_map_hash);
    
    if (debug.isShadowDebugEnabled()) {
        std::cout << "Shadow map generation complete" << std::endl;
    }
}

void CEShadowManager::beginShadowPass()
{
    DebugConfig& debug = DebugConfig::getInstance();
    
    if (debug.isShadowDebugEnabled()) {
        std::cout << "Beginning shadow pass..." << std::endl;
    }
    
    // Store current viewport to restore later
    glGetIntegerv(GL_VIEWPORT, m_saved_viewport);
    if (debug.isShadowDebugEnabled()) {
        std::cout << "Saved viewport: " << m_saved_viewport[0] << ", " << m_saved_viewport[1] << ", " << m_saved_viewport[2] << ", " << m_saved_viewport[3] << std::endl;
    }
    
    glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
    
    // Check for errors after viewport change
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "OpenGL error after viewport change: " << err << std::endl;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, m_shadow_framebuffer);
    
    // Check for errors after framebuffer binding
    err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "OpenGL error after framebuffer binding: " << err << std::endl;
    }
    
    // Clear depth buffer to far plane (1.0) - no shadows by default
    glClearDepth(1.0f);
    glClear(GL_DEPTH_BUFFER_BIT);
    
    // Ensure depth testing is enabled and configured properly for shadow rendering
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);  // Standard depth test
    glDepthMask(GL_TRUE);  // Enable depth writes
    
    // Disable scissor test in case it's clipping our geometry
    glDisable(GL_SCISSOR_TEST);
    
    // Check viewport is correct
    if (debug.isShadowDebugEnabled()) {
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        std::cout << "  Shadow pass viewport: (" << viewport[0] << ", " << viewport[1] << ", " << viewport[2] << ", " << viewport[3] << ")" << std::endl;
        std::cout << "  Cleared shadow map to depth 1.0 and enabled depth testing" << std::endl;
    }
    
    // Check for errors after clear
    err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "OpenGL error after depth buffer clear: " << err << std::endl;
    }
    
    // Enable face culling to reduce shadow acne
    glCullFace(GL_FRONT);
    
    if (!m_shadow_shader) {
        std::cerr << "Error: Shadow shader is null in beginShadowPass!" << std::endl;
        return;
    }
    
    m_shadow_shader->use();
    
    // Check for errors after shader use
    err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "OpenGL error after shadow shader use: " << err << std::endl;
    }
    
    m_shadow_shader->setMat4("lightSpaceMatrix", m_light_space_matrix);
    
    // Check for errors after uniform setting
    err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "OpenGL error after setting lightSpaceMatrix: " << err << std::endl;
    }
    
    if (debug.isShadowDebugEnabled()) {
        std::cout << "Shadow pass begun successfully" << std::endl;
    }
}

void CEShadowManager::endShadowPass()
{
    DebugConfig& debug = DebugConfig::getInstance();
    
    if (debug.isShadowDebugEnabled()) {
        std::cout << "Ending shadow pass..." << std::endl;
    }
    
    // Only set cull face if culling is actually enabled
    if (glIsEnabled(GL_CULL_FACE)) {
        glCullFace(GL_BACK);
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // Check for errors after framebuffer unbinding
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "OpenGL error after framebuffer unbinding: " << err << std::endl;
    }
    
    // Restore the original viewport
    glViewport(m_saved_viewport[0], m_saved_viewport[1], m_saved_viewport[2], m_saved_viewport[3]);
    if (debug.isShadowDebugEnabled()) {
        std::cout << "Restored viewport: " << m_saved_viewport[0] << ", " << m_saved_viewport[1] << ", " << m_saved_viewport[2] << ", " << m_saved_viewport[3] << std::endl;
    }
    
    // Check for errors after viewport restoration
    err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "OpenGL error after viewport restoration: " << err << std::endl;
    }
    if (debug.isShadowDebugEnabled()) {
        std::cout << "Shadow pass ended successfully" << std::endl;
    }
}

void CEShadowManager::renderShadowCaster(CEWorldModel* model, const Transform& transform)
{
    if (!model || !model->getGeometry()) return;
    
    // Debug: Check if we actually have geometry and if Draw() is being called
    CEGeometry* geom = model->getGeometry();
    if (!geom) {
        static int noGeomCount = 0;
        if (noGeomCount < 5) {
            std::cout << "        ERROR: Model has no geometry!" << std::endl;
            noGeomCount++;
        }
        return;
    }
    
    // Set model matrix 
    glm::mat4 modelMatrix = transform.GetStaticModel();
    m_shadow_shader->setMat4("model", modelMatrix);
    
    // Check if shader is bound correctly
    GLint currentProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
    if (currentProgram == 0) {
        std::cout << "        ERROR: No shader program bound!" << std::endl;
        return;
    }
    
    // Check if we're rendering to the shadow framebuffer
    GLint currentFB;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFB);
    if (currentFB != m_shadow_framebuffer) {
        std::cout << "        ERROR: Not rendering to shadow framebuffer! Current: " << currentFB << ", Expected: " << m_shadow_framebuffer << std::endl;
        return;
    }
    
    // Minimal debug output - only check for critical errors
    static int errorCheckCount = 0;
    if (errorCheckCount < 5) {
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cout << "        OpenGL error in renderShadowCaster: " << error << std::endl;
        }
        errorCheckCount++;
    }
    
    // Don't use Draw() because it switches to the geometry's own shader
    // Instead, manually bind the VAO and draw with our shadow shader
    glBindVertexArray(geom->GetVAO());
    glDrawElementsBaseVertex(GL_TRIANGLES, geom->GetIndexCount(), GL_UNSIGNED_INT, 0, 0);
    glBindVertexArray(0);
    
    // Check for critical errors after draw
    GLenum postDrawError = glGetError();
    if (postDrawError != GL_NO_ERROR) {
        std::cout << "        OpenGL error after shadow draw: " << postDrawError << std::endl;
    }
}

bool CEShadowManager::shouldCastShadow(CEWorldModel* model)
{
    if (!model || !model->getObjectInfo()) {
        return false;
    }
    
    TObjInfo* objInfo = model->getObjectInfo();
    
    // Don't cast shadows if the object has the NO_LIGHT flag
    if (objInfo->flags & objectNOLIGHT) {
        return false;
    }
    
    // Based on original Carnivores logic: Objects cast shadows if they have shadow properties
    // Check for original shadow properties (linelenght > 0 or circlerad > 0)
    if (objInfo->linelenght > 0 || objInfo->circlerad > 0) {
        return true;
    }
    
    // GRNDLIGHT objects interact with terrain lighting and should cast shadows
    if (objInfo->flags & objectGRNDLIGHT) {
        return true;
    }
    
    // Objects with no explicit lighting flags also cast shadows (trees, rocks, etc.)
    bool hasAnyLightFlag = (objInfo->flags & (objectNOLIGHT | objectDEFLIGHT | objectGRNDLIGHT)) != 0;
    return !hasAnyLightFlag;
}

void CEShadowManager::setCacheKey(const std::string& mapName, const std::vector<CEWorldModel*>& objects)
{
    m_current_map_hash = generateMapHash(objects);
    
    DebugConfig& debug = DebugConfig::getInstance();
    if (debug.isShadowDebugEnabled()) {
        std::cout << "  Generated map hash: " << m_current_map_hash << std::endl;
    }
}

std::string CEShadowManager::generateMapHash(const std::vector<CEWorldModel*>& objects)
{
    DebugConfig& debug = DebugConfig::getInstance();
    std::stringstream ss;
    
    // Include light direction in hash
    ss << m_light_direction.x << "," << m_light_direction.y << "," << m_light_direction.z << "|";
    
    int modelCount = 0;
    int transformCount = 0;
    
    // Include shadow casting objects and their transforms
    for (CEWorldModel* model : objects) {
        if (shouldCastShadow(model)) {
            modelCount++;
            ss << model->getObjectInfo()->flags << "|";
            
            const auto& transforms = model->getTransforms();
            transformCount += transforms.size();
            
            for (const Transform& t : transforms) {
                Transform& mutableT = const_cast<Transform&>(t);
                glm::vec3 pos = *mutableT.GetPos();
                glm::vec3 rot = *mutableT.GetRot();
                glm::vec3 scale = *mutableT.GetScale();
                ss << pos.x << "," << pos.y << "," << pos.z << ";";
                ss << rot.x << "," << rot.y << "," << rot.z << ";";
                ss << scale.x << "," << scale.y << "," << scale.z << "|";
            }
        }
    }
    
    if (debug.isShadowDebugEnabled()) {
        std::cout << "  Hash includes " << modelCount << " models with " << transformCount << " transforms" << std::endl;
    }
    
    // Simple hash of the concatenated string
    std::hash<std::string> hasher;
    std::string hash = std::to_string(hasher(ss.str()));
    
    if (debug.isShadowDebugEnabled()) {
        std::cout << "  Hash string length: " << ss.str().length() << std::endl;
        std::cout << "  Hash first 50 chars: " << ss.str().substr(0, 50) << "..." << std::endl;
    }
    
    return hash;
}

bool CEShadowManager::loadCachedShadowMap(const std::string& mapHash)
{
    DebugConfig& debug = DebugConfig::getInstance();
    
    std::filesystem::path cacheDir = "runtime/cache/shadows";
    std::filesystem::create_directories(cacheDir);
    
    std::filesystem::path cachePath = cacheDir / (mapHash + ".shadowmap");
    
    if (debug.isShadowDebugEnabled()) {
        std::cout << "  Looking for cached shadow map: " << cachePath << std::endl;
    }
    
    if (!std::filesystem::exists(cachePath)) {
        if (debug.isShadowDebugEnabled()) {
            std::cout << "  Cache file does not exist" << std::endl;
        }
        return false;
    }
    
    try {
        std::ifstream file(cachePath, std::ios::binary);
        if (!file.is_open()) {
            if (debug.isShadowDebugEnabled()) {
                std::cout << "  Failed to open cache file" << std::endl;
            }
            return false;
        }
        
        // Read shadow map data
        std::vector<float> shadowData(SHADOW_MAP_SIZE * SHADOW_MAP_SIZE);
        file.read(reinterpret_cast<char*>(shadowData.data()), 
                  shadowData.size() * sizeof(float));
        
        if (debug.isShadowDebugEnabled()) {
            std::cout << "  Loaded " << shadowData.size() << " depth values from cache" << std::endl;
            
            // Check if the loaded data is valid
            float minDepth = *std::min_element(shadowData.begin(), shadowData.end());
            float maxDepth = *std::max_element(shadowData.begin(), shadowData.end());
            std::cout << "  Cached depth range: " << minDepth << " to " << maxDepth << std::endl;
        }
        
        // Upload to texture - make sure we set up all the same parameters as during creation
        glBindTexture(GL_TEXTURE_2D, m_shadow_depth_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
                     SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 0,
                     GL_DEPTH_COMPONENT, GL_FLOAT, shadowData.data());
        
        // Re-apply texture parameters (these might be needed after glTexImage2D)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        
        float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        
        if (debug.isShadowDebugEnabled()) {
            std::cout << "  Successfully loaded cached shadow map with texture parameters" << std::endl;
        }
        
        // Check for OpenGL errors after texture upload
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            std::cerr << "  OpenGL error after cached shadow map upload: " << err << std::endl;
            return false;
        }
        
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to load cached shadow map: " << e.what() << std::endl;
        return false;
    }
}

void CEShadowManager::saveShadowMapCache(const std::string& mapHash)
{
    DebugConfig& debug = DebugConfig::getInstance();
    
    std::filesystem::path cacheDir = "runtime/cache/shadows";
    std::filesystem::create_directories(cacheDir);
    
    std::filesystem::path cachePath = cacheDir / (mapHash + ".shadowmap");
    
    if (debug.isShadowDebugEnabled()) {
        std::cout << "  Saving shadow map cache to: " << cachePath << std::endl;
    }
    
    try {
        std::vector<float> shadowData(SHADOW_MAP_SIZE * SHADOW_MAP_SIZE);
        
        // Read shadow map from GPU
        glBindTexture(GL_TEXTURE_2D, m_shadow_depth_texture);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, shadowData.data());
        
        if (debug.isShadowDebugEnabled()) {
            float minDepth = *std::min_element(shadowData.begin(), shadowData.end());
            float maxDepth = *std::max_element(shadowData.begin(), shadowData.end());
            std::cout << "  Saving depth range: " << minDepth << " to " << maxDepth << std::endl;
        }
        
        // Write to cache file
        std::ofstream file(cachePath, std::ios::binary);
        if (file.is_open()) {
            file.write(reinterpret_cast<const char*>(shadowData.data()), 
                      shadowData.size() * sizeof(float));
            if (debug.isShadowDebugEnabled()) {
                std::cout << "  Successfully saved " << shadowData.size() << " depth values to cache" << std::endl;
            }
        } else {
            if (debug.isShadowDebugEnabled()) {
                std::cout << "  Failed to open cache file for writing" << std::endl;
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to save shadow map cache: " << e.what() << std::endl;
    }
}

bool CEShadowManager::hasCachedShadows() const
{
    std::filesystem::path cacheDir = "runtime/cache/shadows";
    std::filesystem::path cachePath = cacheDir / (m_current_map_hash + ".shadowmap");
    return std::filesystem::exists(cachePath);
}
