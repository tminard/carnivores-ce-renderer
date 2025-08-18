//
//  CEParticleSystem.cpp
//  CE Character Lab
//
//  Particle system implementation for impact effects
//

#include "CEParticleSystem.h"
#include "shader_program.h"
#include "camera.h"
#include <iostream>
#include <random>
#include <cstdlib>
#include <fstream>
#include <filesystem>
#include <glm/gtc/matrix_transform.hpp>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;

CEParticleSystem::CEParticleSystem(size_t maxParticles) 
    : m_maxParticles(maxParticles), m_nextParticleIndex(0), m_VAO(0), m_VBO(0), m_textureID(0)
{
    m_particles.resize(m_maxParticles);
    m_instanceData.reserve(m_maxParticles * 7); // position(3) + size(1) + color(3)
    
    initializeOpenGL();
    createDefaultTexture();
}

CEParticleSystem::~CEParticleSystem()
{
    if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
    if (m_VBO) glDeleteBuffers(1, &m_VBO);
    if (m_textureID) glDeleteTextures(1, &m_textureID);
}

void CEParticleSystem::initializeOpenGL()
{
    // Load particle shaders from files using config.json basePath
    try {
        std::ifstream f("config.json");
        json data = json::parse(f);
        
        fs::path basePath = fs::path(data["basePath"].get<std::string>());
        fs::path shaderPath = basePath / "shaders";
        
        m_shader.reset(new ShaderProgram((shaderPath / "particle.vs").string(), (shaderPath / "particle.fs").string()));
    } catch (const std::exception& e) {
        std::cerr << "Failed to create particle shader: " << e.what() << std::endl;
        return;
    }
    
    // Create quad geometry for particles
    float quadVertices[] = {
        // positions   // texture coords
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, 0.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f,  0.0f, 1.0f
    };
    
    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };
    
    GLuint EBO;
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(m_VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // Vertex attributes for quad
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Instance attributes will be set up during rendering
    glBindVertexArray(0);
}

void CEParticleSystem::createDefaultTexture()
{
    // Create a simple white circle texture for particles
    const int size = 32;
    std::vector<unsigned char> textureData(size * size * 4);
    
    float center = size * 0.5f;
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            float dx = x - center;
            float dy = y - center;
            float distance = sqrt(dx * dx + dy * dy);
            float alpha = 1.0f - (distance / center);
            alpha = std::max(0.0f, alpha);
            
            int index = (y * size + x) * 4;
            textureData[index + 0] = 255; // R
            textureData[index + 1] = 255; // G
            textureData[index + 2] = 255; // B
            textureData[index + 3] = (unsigned char)(alpha * 255); // A
        }
    }
    
    glGenTextures(1, &m_textureID);
    glBindTexture(GL_TEXTURE_2D, m_textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

size_t CEParticleSystem::findDeadParticle()
{
    // Start from last used particle for better cache performance
    for (size_t i = m_nextParticleIndex; i < m_maxParticles; i++) {
        if (!m_particles[i].isAlive()) {
            m_nextParticleIndex = i;
            return i;
        }
    }
    
    // Search from beginning
    for (size_t i = 0; i < m_nextParticleIndex; i++) {
        if (!m_particles[i].isAlive()) {
            m_nextParticleIndex = i;
            return i;
        }
    }
    
    // Override oldest particle if all are alive
    m_nextParticleIndex = (m_nextParticleIndex + 1) % m_maxParticles;
    return m_nextParticleIndex;
}

void CEParticleSystem::emitGroundImpact(const glm::vec3& position, const glm::vec3& normal, int count)
{
    for (int i = 0; i < count; i++) {
        size_t index = findDeadParticle();
        CEParticle& particle = m_particles[index];
        
        // Larger spread for more dramatic effect
        float spread = 1.2f;
        particle.position = position + glm::vec3(
            (rand() % 100 / 100.0f - 0.5f) * spread,
            0.05f,
            (rand() % 100 / 100.0f - 0.5f) * spread
        );
        
        // Much higher velocity for explosive effect
        glm::vec3 randomDir = glm::vec3(
            (rand() % 100 / 100.0f - 0.5f),
            (rand() % 100 / 100.0f),
            (rand() % 100 / 100.0f - 0.5f)
        );
        particle.velocity = (normal + randomDir * 0.8f) * (10.0f + rand() % 100 / 15.0f); // 10-16 speed (much faster)
        
        // Darker dirt colors
        float colorVariation = 0.4f + (rand() % 40) / 100.0f; // 0.4 - 0.8 (darker)
        particle.color = glm::vec4(0.5f * colorVariation, 0.3f * colorVariation, 0.15f * colorVariation, 1.0f);
        
        particle.life = 1.0f;
        particle.maxLife = 1.5f + (rand() % 150) / 100.0f; // 1.5-3 seconds
        particle.size = 0.075f + (rand() % 100) / 400.0f; // 0.075 - 0.325 (half size)
        particle.gravity = -15.0f - (rand() % 100) / 20.0f; // Much stronger gravity (-15 to -20)
    }
}

void CEParticleSystem::emitDustCloud(const glm::vec3& position, int count)
{
    for (int i = 0; i < count; i++) {
        size_t index = findDeadParticle();
        CEParticle& particle = m_particles[index];
        
        // Larger initial spread for bigger dust cloud
        particle.position = position + glm::vec3(
            (rand() % 100 / 100.0f - 0.5f) * 0.8f,
            (rand() % 100 / 100.0f) * 0.4f,
            (rand() % 100 / 100.0f - 0.5f) * 0.8f
        );
        
        // Higher upward velocity for more dramatic smoke plume
        particle.velocity = glm::vec3(
            (rand() % 100 / 100.0f - 0.5f) * 3.0f,
            4.0f + (rand() % 100) / 33.0f, // 4.0-7.0 upward speed (faster)
            (rand() % 100 / 100.0f - 0.5f) * 3.0f
        );
        
        // Gray/black smoke color - much darker
        float colorVariation = 0.2f + (rand() % 30) / 100.0f; // 0.2 - 0.5 (very dark)
        float grayValue = 0.3f + (rand() % 40) / 100.0f; // 0.3 - 0.7 gray range
        particle.color = glm::vec4(grayValue * colorVariation, grayValue * colorVariation, grayValue * colorVariation, 0.8f); // Gray smoke
        
        particle.life = 1.0f;
        particle.maxLife = 3.0f + (rand() % 200) / 100.0f; // 3-5 seconds for lingering smoke
        particle.size = 0.2f + (rand() % 100) / 200.0f; // 0.2 - 0.7 (half original size)
        particle.gravity = -2.0f; // Heavier gravity for faster settling
    }
}

void CEParticleSystem::emitDebris(const glm::vec3& position, const glm::vec3& impactDirection, int count)
{
    for (int i = 0; i < count; i++) {
        size_t index = findDeadParticle();
        CEParticle& particle = m_particles[index];
        
        particle.position = position;
        
        // Much more explosive debris velocity
        glm::vec3 baseVelocity = -impactDirection * (12.0f + rand() % 100 / 16.0f); // 12-18 speed (much faster)
        glm::vec3 randomness = glm::vec3(
            (rand() % 100 / 100.0f - 0.5f) * 5.0f,
            (rand() % 100 / 100.0f) * 4.0f,
            (rand() % 100 / 100.0f - 0.5f) * 5.0f
        );
        particle.velocity = baseVelocity + randomness;
        
        // Darker debris colors
        float colorVariation = 0.5f + (rand() % 30) / 100.0f; // 0.5 - 0.8 (darker)
        if (rand() % 3 == 0) {
            // Some sparks - bright yellow/orange but darker
            particle.color = glm::vec4(0.8f * colorVariation, 0.6f * colorVariation, 0.2f * colorVariation, 1.0f);
        } else {
            // Stone/concrete debris - much darker
            particle.color = glm::vec4(0.4f * colorVariation, 0.35f * colorVariation, 0.25f * colorVariation, 1.0f);
        }
        
        particle.life = 1.0f;
        particle.maxLife = 2.0f + (rand() % 150) / 100.0f; // 2-3.5 seconds
        particle.size = 0.04f + (rand() % 100) / 800.0f; // 0.04-0.165 (half size)
        particle.gravity = -20.0f; // Much heavier gravity for dramatic fast arcs
    }
}

void CEParticleSystem::update(float deltaTime)
{
    for (auto& particle : m_particles) {
        if (particle.isAlive()) {
            particle.update(deltaTime);
        }
    }
}

void CEParticleSystem::render(Camera* camera)
{
    if (!camera || !m_shader) return;
    
    // Prepare instance data
    m_instanceData.clear();
    for (const auto& particle : m_particles) {
        if (particle.isAlive()) {
            // Position (3 floats)
            m_instanceData.push_back(particle.position.x);
            m_instanceData.push_back(particle.position.y);
            m_instanceData.push_back(particle.position.z);
            
            // Size (1 float)
            m_instanceData.push_back(particle.size);
            
            // Color (3 floats) - alpha handled by life
            m_instanceData.push_back(particle.color.r);
            m_instanceData.push_back(particle.color.g);
            m_instanceData.push_back(particle.color.b);
        }
    }
    
    if (m_instanceData.empty()) return;
    
    // Set up rendering state
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE); // Don't write to depth buffer
    
    m_shader->use();
    m_shader->setMat4("view", camera->getViewMatrix());
    m_shader->setMat4("projection", camera->getProjectionMatrix());
    m_shader->setFloat("alphaMultiplier", 1.0f);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_textureID);
    m_shader->setInt("particleTexture", 0);
    
    glBindVertexArray(m_VAO);
    
    // Upload instance data
    GLuint instanceVBO;
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, m_instanceData.size() * sizeof(float), m_instanceData.data(), GL_DYNAMIC_DRAW);
    
    // Set up instance attributes
    size_t stride = 7 * sizeof(float);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)0); // Position
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);
    
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float))); // Size
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);
    
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, stride, (void*)(4 * sizeof(float))); // Color
    glEnableVertexAttribArray(4);
    glVertexAttribDivisor(4, 1);
    
    // Render instances
    int instanceCount = m_instanceData.size() / 7;
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, instanceCount);
    
    // Cleanup
    glDeleteBuffers(1, &instanceVBO);
    glVertexAttribDivisor(2, 0);
    glVertexAttribDivisor(3, 0);
    glVertexAttribDivisor(4, 0);
    glBindVertexArray(0);
    
    // Restore rendering state
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void CEParticleSystem::clear()
{
    for (auto& particle : m_particles) {
        particle.life = 0.0f;
    }
}

size_t CEParticleSystem::getActiveParticleCount() const
{
    size_t count = 0;
    for (const auto& particle : m_particles) {
        if (particle.isAlive()) count++;
    }
    return count;
}