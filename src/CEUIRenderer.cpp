//
//  CEUIRenderer.cpp
//  CE Character Lab
//
//  Created by Claude Code for 2D UI rendering system
//

#include "CEUIRenderer.h"
#include "vertex.h"
#include "CEGeometry.h"
#include "CETexture.h"
#include "CEAnimation.h"
#include "LocalAudioManager.hpp"
#include "CEAudioSource.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <filesystem>


namespace fs = std::filesystem;
using json = nlohmann::json;

CEUIRenderer::CEUIRenderer(int screenWidth, int screenHeight)
    : m_screenWidth(screenWidth), m_screenHeight(screenHeight), m_weaponDrawn(false),
      m_weaponState(WeaponState::HOLSTERED), m_currentWeaponAnimation(""),
      m_weaponAnimationStartTime(0.0), m_weaponAnimationLastUpdate(0.0), 
      m_weaponAnimationLoop(false), m_weaponGeometryInitialized(false), m_audioManager(nullptr)
{
    setScreenSize(screenWidth, screenHeight);
    initializeUI2DCamera();
}

CEUIRenderer::~CEUIRenderer()
{
    // Smart pointers handle cleanup
}

void CEUIRenderer::initializeUI2DCamera()
{
    // For now, keep using perspective camera but we'll override the projection in rendering
    glm::vec3 cameraPos(0.0f, 0.0f, 100.0f);
    float aspectRatio = static_cast<float>(m_screenWidth) / static_cast<float>(m_screenHeight);
    m_ui2DCamera = std::make_unique<Camera>(cameraPos, glm::radians(60.0f), aspectRatio, 0.1f, 1000.0f);
    
    // Point camera towards negative Z to look at the 2D plane
    m_ui2DCamera->SetLookAt(glm::vec3(0.0f, 0.0f, -1.0f));
    
    std::cout << "UI 2D camera initialized (will use orthographic projection manually)" << std::endl;
}

void CEUIRenderer::setScreenSize(int width, int height)
{
    m_screenWidth = width;
    m_screenHeight = height;
    
    // Recreate the 2D camera with new dimensions
    if (m_ui2DCamera) {
        initializeUI2DCamera();
    }
}

void CEUIRenderer::begin2DRendering()
{
    // Set up 2D rendering state
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);  // Disable face culling for UI
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void CEUIRenderer::end2DRendering()
{
    // Restore 3D rendering state
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);  // Re-enable face culling
}


void CEUIRenderer::renderCompass(C2CarFile* compass, float rotation)
{
    if (!compass || !m_ui2DCamera) {
        std::cerr << "Failed compass render check: compass=" << (compass ? "valid" : "null") 
                  << " camera=" << (m_ui2DCamera ? "valid" : "null") << std::endl;
        return;
    }
    glm::vec3 position(m_screenWidth - 128.f, m_screenHeight - 75.f, 0.0f);
    
    // Original lower-left positioning (commented for debugging):
    // glm::vec3 position(margin + compassSize/2.0f, m_screenHeight - margin - compassSize/2.0f, 0.0f);
    // Rotate compass to face camera with top visible (30 degrees around X-axis) plus user rotation around Y
    glm::vec3 rotationVec(glm::radians(-30.0f), rotation, 0.0f);
    glm::vec3 scale(6.f, 6.f, 6.f);  // Heavy scaling to reduce CAR geometry to UI size
    
    // Create Transform for the UI element
    Transform uiTransform(position, rotationVec, scale);
    
    // Set up 2D rendering
    begin2DRendering();
    
    // Now render the actual compass geometry using proper Update/Draw pattern
    renderCompassGeometry(compass, uiTransform);
    
    // Restore 3D state
    end2DRendering();
}

void CEUIRenderer::renderCompassGeometry(C2CarFile* compass, Transform& uiTransform)
{
    // Get the CAR file's geometry directly
    auto geometry = compass->getGeometry();
    if (!geometry) {
        std::cerr << "CAR file has no geometry" << std::endl;
        return;
    }

    // Switch to UI shader for UI rendering (avoids instanced rendering issues)
    geometry->setShader("ui");
    
    // Don't use Camera's Update - we'll set our own orthographic MVP
    auto position = uiTransform.GetPos();
    
    // Create orthographic projection matrix for UI
    // Left, Right, Bottom, Top, Near, Far
    glm::mat4 orthoProjection = glm::ortho(0.0f, (float)m_screenWidth, 0.0f, (float)m_screenHeight, -100.f, 10000.0f);
    
    // Create view matrix (identity for orthographic UI)
    glm::mat4 view = glm::mat4(1.0f);
    
    // Get model matrix from transform
    glm::mat4 model = uiTransform.GetStaticModel();
    
    // Calculate MVP manually
    glm::mat4 mvp = orthoProjection * view * model;
    
    // Set the orthographic MVP matrix manually
    auto shader = geometry->getShader();
    if (shader) {
        shader->use();
        shader->setMat4("MVP", mvp);
    } else {
        std::cerr << "ERROR: Geometry has no shader!" << std::endl;
        return;
    }
    
    // Then call Draw() which will use the standard shader system
    geometry->Draw();
}

void CEUIRenderer::renderCompass(C2CarFile* compass, Camera* gameCamera)
{
    if (!compass || !m_ui2DCamera || !gameCamera) {
        std::cerr << "Failed compass render check: compass=" << (compass ? "valid" : "null") 
                  << " ui_camera=" << (m_ui2DCamera ? "valid" : "null")
                  << " game_camera=" << (gameCamera ? "valid" : "null") << std::endl;
        return;
    }
    
    // Calculate rotation to keep compass aligned with world directions
    // We need to counter-rotate against the camera's Y rotation (yaw)
    
    // Get the camera's forward vector and calculate its Y rotation
    glm::vec3 cameraForward = gameCamera->GetForward();
    
    // Calculate the yaw angle from the forward vector
    // atan2 gives us the angle in the XZ plane (Y rotation)
    float cameraYaw = atan2(cameraForward.x, cameraForward.z);
    
    // Counter-rotate the compass to keep it world-aligned
    float compassRotation = cameraYaw;
    
    // Call the original method with calculated rotation
    renderCompass(compass, compassRotation);
}

void CEUIRenderer::renderTestSquare()
{
    // Simple test square to verify rendering pipeline
    // Use unit coordinates (0-1) that will be scaled by MVP matrix
    float vertices[] = {
        // positions   // texture coords
        0.0f, 1.0f, 0.0f,  0.0f, 1.0f,  // top left
        1.0f, 1.0f, 0.0f,  1.0f, 1.0f,  // top right
        1.0f, 0.0f, 0.0f,  1.0f, 0.0f,  // bottom right
        0.0f, 0.0f, 0.0f,  0.0f, 0.0f   // bottom left
    };
    
    unsigned int indices[] = {
        0, 1, 2,  // first triangle
        2, 3, 0   // second triangle
    };
    
    std::cout << "Rendering test square" << std::endl;
    
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Create a simple white texture
    GLuint textureID;
    glGenTextures(1, &textureID);
    glActiveTexture(GL_TEXTURE0);  // Activate texture unit 0
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    unsigned char whitePixel[4] = {255, 255, 255, 255}; // White pixel
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Draw the square
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    
    // Cleanup
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteTextures(1, &textureID);
    
    std::cout << "Test square rendered" << std::endl;
}

void CEUIRenderer::toggleWeapon()
{
    switch (m_weaponState) {
        case WeaponState::HOLSTERED:
            m_weaponState = WeaponState::DRAWING;
            setWeaponAnimation(m_weaponDrawAnimation, false); // Don't loop draw animation
            m_weaponDrawn = true;
            break;
            
        case WeaponState::DRAWN:
            m_weaponState = WeaponState::HOLSTERING;
            setWeaponAnimation(m_weaponHolsterAnimation, false); // Don't loop holster animation
            break;
            
        case WeaponState::DRAWING:
        case WeaponState::HOLSTERING:
            // Ignore toggle during animation
            break;
    }
}

void CEUIRenderer::renderWeapon(C2CarFile* weapon, double currentTime)
{
    if (!weapon || !m_ui2DCamera || !m_weaponDrawn) {
        return; // Don't render if weapon not drawn or missing
    }

    // Update weapon animation state
    updateWeaponAnimation(weapon, currentTime);
  
  if (!m_weaponDrawn) {
    return;
  }
    
    // For perspective rendering, we'll position weapon in world space via translation matrix
    // This position is just for the transform, actual positioning done in world space
    glm::vec3 position(0.0f, 0.0f, 0.0f);
    
    // Rotate weapon to point away from player and angle upward
    // X: slight up angle, Y: 180° to face away from player, Z: no roll
    glm::vec3 rotationVec(glm::radians(0.0f), glm::radians(180.0f), glm::radians(0.0f));
    
    // Scale weapon appropriately for perspective view
    glm::vec3 scale(1.f);
    
    // Create Transform for the weapon
    Transform weaponTransform(position, rotationVec, scale);

    // Set up weapon-specific rendering state
    setupWeaponRendering();
    
    renderWeaponGeometry(weapon, weaponTransform);
    
    // Restore normal rendering state
    restoreNormalRendering();
}

void CEUIRenderer::renderWeaponGeometry(C2CarFile* weapon, Transform& weaponTransform)
{
    // Get the CAR file's geometry directly
    auto geometry = weapon->getGeometry();
    if (!geometry) {
        std::cerr << "Weapon CAR file has no geometry" << std::endl;
        return;
    }

    // Switch to UI shader for UI rendering (avoids instanced rendering issues)
    geometry->setShader("ui");
    
    // Use perspective projection for weapon to give it depth
    auto position = weaponTransform.GetPos();
    
    // Create perspective projection for weapon depth
    float aspectRatio = (float)m_screenWidth / (float)m_screenHeight;
    glm::mat4 perspectiveProjection = glm::perspective(glm::radians(60.0f), aspectRatio, 0.1f, 100000.0f);
    
    // Create view matrix positioned for first-person weapon view
    glm::vec3 weaponCameraPos(0.0f, 0.0f, 0.1f);  // Close to weapon
    glm::vec3 weaponCameraTarget(0.0f, 0.0f, 0.0f);  // Looking at origin
    glm::vec3 weaponCameraUp(0.0f, 1.0f, 0.0f);
    glm::mat4 view = glm::lookAt(weaponCameraPos, weaponCameraTarget, weaponCameraUp);
    
    // Position weapon in world space for perspective view
    glm::vec3 weaponWorldPos(0.f, 0.f, 0.f);  // Offset for first-person positioning
    glm::mat4 weaponTranslation = glm::translate(glm::mat4(1.0f), weaponWorldPos);
    glm::mat4 model = weaponTranslation * weaponTransform.GetStaticModel();
    
    // Calculate MVP for perspective weapon
    glm::mat4 mvp = perspectiveProjection * view * model;
    
    // Set the perspective MVP matrix manually
    auto shader = geometry->getShader();
    if (shader) {
        shader->use();
        shader->setMat4("MVP", mvp);
    } else {
        std::cerr << "ERROR: Weapon geometry has no shader!" << std::endl;
        return;
    }
    
    // Then call Draw() which will use the standard shader system
    geometry->Draw();
}

void CEUIRenderer::setupWeaponRendering()
{
    // Clear only the depth buffer to ensure weapon renders over everything
    glClear(GL_DEPTH_BUFFER_BIT);
    
    // Keep normal depth testing for weapon's internal depth
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);  // Normal depth testing within weapon
    
    // Enable face culling to hide inside faces
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);  // Cull back faces (inside faces)
    glFrontFace(GL_CCW);  // Counter-clockwise faces are front
    
    // Use a very small depth range near the camera for weapon
    // This keeps weapon in front while preserving internal depth relationships
    glDepthRange(0.0, 0.1);  // Weapon uses first 10% of depth buffer
    
    // Enable blending for proper transparency if needed
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void CEUIRenderer::restoreNormalRendering()
{
    // Restore normal depth testing (already GL_LESS, but good to be explicit)
    glDepthFunc(GL_LESS);     
    glDepthRange(0.0, 1.0);   // Restore full depth range for scene
    
    // Keep face culling enabled for normal scene rendering
    // Most games keep this enabled for performance
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    // Disable blending
    glDisable(GL_BLEND);
}

void CEUIRenderer::configureWeaponAnimations(const std::string& drawAnim, const std::string& holsterAnim)
{
    m_weaponDrawAnimation = drawAnim;
    m_weaponHolsterAnimation = holsterAnim;
}

void CEUIRenderer::setAudioManager(LocalAudioManager* audioManager)
{
    m_audioManager = audioManager;
}

void CEUIRenderer::setWeaponAnimation(const std::string& animationName, bool loop)
{
    // Follow AI pattern: set animation parameters but don't start immediately
    m_currentWeaponAnimation = animationName;
    m_weaponAnimationStartTime = glfwGetTime();
    m_weaponAnimationLastUpdate = m_weaponAnimationStartTime; // Reset to ensure first update triggers
    m_weaponAnimationLoop = loop;
    m_weaponGeometryInitialized = false; // Reset flag to ensure new animation gets initialized
}

std::shared_ptr<CEAnimation> CEUIRenderer::getWeaponAnimation(C2CarFile* weapon, const std::string& animName)
{
    if (animName.empty()) {
        std::cerr << "ERROR: Empty animation name provided" << std::endl;
        return nullptr;
    }
    
    auto animation = weapon->getAnimationByName(animName).lock();
    if (!animation) {
        std::cerr << "ERROR: Animation '" << animName << "' not found in weapon CAR file" << std::endl;
        return nullptr;
    }
    
    return animation;
}

void CEUIRenderer::initializeWeaponGeometry(C2CarFile* weapon)
{
    if (!weapon || m_currentWeaponAnimation.empty()) {
        return;
    }
    
    // Get the current animation
    auto currentAnimation = getWeaponAnimation(weapon, m_currentWeaponAnimation);
    if (!currentAnimation) {
        return;
    }
    
    // Initialize weapon geometry to frame 0 to prevent initial artifacts
    auto geometry = weapon->getGeometry();
    if (geometry) {
        geometry->SetAnimation(currentAnimation, 0); // Set to frame 0
    }
}

void CEUIRenderer::updateWeaponAnimation(C2CarFile* weapon, double currentTime)
{
    // Only update animation when we're actively animating or need to maintain drawn state
    if (m_weaponState != WeaponState::DRAWING && 
        m_weaponState != WeaponState::HOLSTERING && 
        m_weaponState != WeaponState::DRAWN) {
        return;
    }
    
    // Skip if no animation is set
    if (m_currentWeaponAnimation.empty()) {
        std::cerr << "ERROR: No current weapon animation set!" << std::endl;
        return;
    }
    
    // Get the animation (following AI pattern)
    auto currentAnimation = getWeaponAnimation(weapon, m_currentWeaponAnimation);
    if (!currentAnimation) {
        std::cerr << "ERROR: Could not get weapon animation: " << m_currentWeaponAnimation << std::endl;
        return;
    }
    
    // Update the geometry animation (following AI pattern exactly)
    auto geometry = weapon->getGeometry();
    if (!geometry) {
        std::cerr << "ERROR: Weapon has no geometry!" << std::endl;
        return;
    }

    // Initialize weapon geometry to frame 0 on first call to prevent artifacts
    if (!m_weaponGeometryInitialized) {
        geometry->SetAnimation(currentAnimation, 0); // Set to frame 0
        m_weaponGeometryInitialized = true;
    }

    // Handle different states
    bool didUpdate = false;
    if (m_weaponState == WeaponState::DRAWN) {
        // For drawn state, maintain the final frame of the draw animation
        int finalFrame = currentAnimation->m_number_of_frames - 1;
        didUpdate = geometry->SetAnimation(currentAnimation, finalFrame);
    } else {
        // For animating states, use normal animation update with no interpolation for crisp weapon frames
        didUpdate = geometry->SetAnimation(
            currentAnimation,
            currentTime,
            m_weaponAnimationStartTime,
            m_weaponAnimationLastUpdate,
            false, // deferUpdate
            true,  // maxFPS  
            false, // notVisible
            2.0f,  // playbackSpeed
            m_weaponAnimationLoop,  // loop flag
            true   // noInterpolation - weapons need discrete frames
        );
    }
    
    // Check if non-looping animation finished (only during animating states)
    if (!m_weaponAnimationLoop && (m_weaponState == WeaponState::DRAWING || m_weaponState == WeaponState::HOLSTERING)) {
        double animationDuration = (currentAnimation->m_number_of_frames / (double)currentAnimation->m_kps);
        double elapsedTime = currentTime - m_weaponAnimationStartTime;
        
        if (elapsedTime >= animationDuration) {
            // Animation finished - transition to final state
            if (m_weaponState == WeaponState::DRAWING) {
                m_weaponState = WeaponState::DRAWN;
            } else if (m_weaponState == WeaponState::HOLSTERING) {
                m_weaponState = WeaponState::HOLSTERED;
                m_weaponDrawn = false; // Stop rendering
                m_currentWeaponAnimation = ""; // Clear animation
            }
        }
    }
    
    // Handle audio exactly like AI characters do (only during animations, not when drawn)
    if (didUpdate && m_weaponState != WeaponState::DRAWN) {
        auto audioSrc = weapon->getSoundForAnimation(m_currentWeaponAnimation);
        if (audioSrc != nullptr) {
            if (geometry->GetCurrentFrame() == 0) {
                if (!audioSrc->isPlaying()) {
                    audioSrc->setLooped(false);
                    audioSrc->setNoDistance(3.f);
                    m_audioManager->play(audioSrc);
                }
            }
        }
        
        m_weaponAnimationLastUpdate = currentTime;
    }
}

