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
    initializeShaders();
}

CEUIRenderer::~CEUIRenderer()
{
    // Smart pointers handle cleanup
}

void CEUIRenderer::initializeShaders()
{
    try {
        std::ifstream f("config.json");
        json data = json::parse(f);
        
        fs::path basePath = fs::path(data["basePath"].get<std::string>());
        fs::path shaderPath = basePath / "shaders";
        
        m_uiShader = std::make_unique<ShaderProgram>(
            (shaderPath / "ui.vs").string(),
            (shaderPath / "ui.fs").string()
        );
        std::cout << "UI shader initialized successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize UI shader: " << e.what() << std::endl;
    }
}

void CEUIRenderer::setScreenSize(int width, int height)
{
    m_screenWidth = width;
    m_screenHeight = height;
    
    // Create orthographic projection matrix for 2D rendering
    // Left, Right, Bottom, Top, Near, Far
    // Flip Y-axis so (0,0) is top-left like typical UI coordinates
    m_projectionMatrix = glm::ortho(0.0f, static_cast<float>(width), 
                                   static_cast<float>(height), 0.0f, 
                                   -10.0f, 100.0f);
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

glm::mat4 CEUIRenderer::createUITransform(const glm::vec2& position, const glm::vec2& scale, float rotation) const
{
    glm::mat4 transform = glm::mat4(1.0f);
    
    // Apply transformations in order: Scale, Rotate, Translate
    transform = glm::translate(transform, glm::vec3(position, 0.0f));
    
    if (rotation != 0.0f) {
        transform = glm::rotate(transform, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    }
    
    transform = glm::scale(transform, glm::vec3(scale, 1.0f));
    
    return transform;
}

void CEUIRenderer::renderCompass(C2CarFile* compass, float rotation)
{
    if (!compass || !m_uiShader) {
        std::cerr << "Failed compass render check: compass=" << (compass ? "valid" : "null") 
                  << " shader=" << (m_uiShader ? "valid" : "null") << std::endl;
        return;
    }
    
    std::cout << "Rendering compass with CAR file" << std::endl;
    
    // Position compass in lower-left corner
    // Size it appropriately for UI display
    float compassSize = 90.0f;  // 80 pixels - reasonable UI size
    float margin = 20.0f;       // Standard UI margin
    
    // Position compass center accounting for its size
    // Lower-left corner with proper offset for center-based positioning
    glm::vec2 position(margin + compassSize/2.0f, 1000.f);
    glm::vec2 scale(1.f, 1.f);
    
    // Create model matrix for compass positioning and rotation
    glm::mat4 model = createUITransform(position, scale, rotation);
    
    // Create MVP matrix (Model-View-Projection)
    glm::mat4 mvp = m_projectionMatrix * model;
    
    std::cout << "Compass position: " << position.x << "," << position.y 
              << " scale: " << scale.x << "," << scale.y << std::endl;
    std::cout << "Screen size: " << m_screenWidth << "x" << m_screenHeight << std::endl;
    
    // Check current viewport
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    std::cout << "Current viewport: " << viewport[0] << "," << viewport[1] 
              << " " << viewport[2] << "x" << viewport[3] << std::endl;
    
    // Set up 2D rendering
    begin2DRendering();
    
    // Use UI shader and set uniforms
    m_uiShader->use();
    
    // Check for OpenGL errors after shader use
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error after shader use: " << error << std::endl;
    }
    
    m_uiShader->setMat4("mvp", mvp);
    m_uiShader->setInt("basic_texture", 0);  // Bind texture to unit 0
    
    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error after setting uniforms: " << error << std::endl;
    }
    
    // Create simple geometry from 3DF data for immediate rendering
    // This is a simplified approach - in production you'd cache this
    
    // Now render the actual compass geometry
    renderCompassGeometry(compass);
    
    // Restore 3D state
    end2DRendering();
}

void CEUIRenderer::renderCompassGeometry(C2CarFile* compass)
{
    // Get the CAR file's geometry directly
    auto geometry = compass->getGeometry();
    if (!geometry) {
        std::cerr << "CAR file has no geometry" << std::endl;
        return;
    }

    std::cout << "Using standard CEGeometry rendering for compass" << std::endl;
    
    // Use the standard CEGeometry Draw() method - it handles everything properly
    geometry->DrawNaked();
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
