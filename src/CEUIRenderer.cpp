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
    
    std::cout << "Rendering compass with CAR file using 2D camera and Update/Draw pattern" << std::endl;
    
    // Position compass in lower-left corner using screen coordinates
    float compassSize = 80.0f;   // Size in screen pixels
    float margin = 20.0f;        // Margin in screen pixels
    
    // Try positioning near camera center first for testing
    glm::vec3 position(m_screenWidth/2.0f, m_screenHeight/2.0f, 0.0f);
    
    // Original lower-left positioning (commented for debugging):
    // glm::vec3 position(margin + compassSize/2.0f, m_screenHeight - margin - compassSize/2.0f, 0.0f);
    glm::vec3 rotationVec(0.0f, 0.0f, rotation);
    glm::vec3 scale(1.f, 1.f, 1.f);  // Heavy scaling to reduce CAR geometry to UI size
    
    // Create Transform for the UI element
    Transform uiTransform(position, rotationVec, scale);
    
    std::cout << "DEBUG: Screen height: " << m_screenHeight << std::endl;
    std::cout << "DEBUG: Calculated position: (" << position.x << ", " << position.y << ")" << std::endl;
    std::cout << "DEBUG: Scale vector: (" << scale.x << ", " << scale.y << ")" << std::endl;
    
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

    std::cout << "Using CEGeometry Update/Draw pattern for compass with 2D camera" << std::endl;
    
    // Debug: Check geometry state
    std::cout << "DEBUG: Geometry has " << geometry->GetVertices().size() << " vertices" << std::endl;
    std::cout << "DEBUG: Geometry has " << geometry->GetIndexCount() << " indices" << std::endl;
    std::cout << "DEBUG: VAO ID: " << geometry->GetVAO() << std::endl;
    
    // Switch to UI shader for UI rendering (avoids instanced rendering issues)
    geometry->setShader("ui");
    
    // Don't use Camera's Update - we'll set our own orthographic MVP
    auto position = uiTransform.GetPos();
    
    // Create orthographic projection matrix for UI
    // Left, Right, Bottom, Top, Near, Far
    glm::mat4 orthoProjection = glm::ortho(0.0f, (float)m_screenWidth, 0.0f, (float)m_screenHeight, -100.0f, 100.0f);
    
    // Create view matrix (identity for orthographic UI)
    glm::mat4 view = glm::mat4(1.0f);
    
    // Get model matrix from transform
    glm::mat4 model = uiTransform.GetStaticModel();
    
    // Calculate MVP manually
    glm::mat4 mvp = orthoProjection * view * model;
    
    std::cout << "DEBUG: Object pos: (" << position->x << ", " << position->y << ", " << position->z << ")" << std::endl;
    std::cout << "DEBUG: Using orthographic projection for UI" << std::endl;
    
    // Set the orthographic MVP matrix manually
    auto shader = geometry->getShader();
    if (shader) {
        shader->use();
        shader->setMat4("MVP", mvp);
        std::cout << "DEBUG: UI shader is ready with orthographic MVP" << std::endl;
    } else {
        std::cerr << "ERROR: Geometry has no shader!" << std::endl;
        return;
    }
    
    // Then call Draw() which will use the standard shader system
    std::cout << "DEBUG: About to call geometry->Draw()" << std::endl;
    geometry->Draw();
    std::cout << "DEBUG: geometry->Draw() completed" << std::endl;
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
