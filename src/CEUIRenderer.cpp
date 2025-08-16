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
#include <glad/glad.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <filesystem>


namespace fs = std::filesystem;
using json = nlohmann::json;

CEUIRenderer::CEUIRenderer(int screenWidth, int screenHeight)
    : m_screenWidth(screenWidth), m_screenHeight(screenHeight)
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
    glm::mat4 orthoProjection = glm::ortho(0.0f, (float)m_screenWidth, 0.0f, (float)m_screenHeight, -1000.0f, 1000.0f);
    
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
