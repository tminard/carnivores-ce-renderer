#pragma once

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <memory>

/**
 * OpenGL implementation of Bullet Physics debug drawer
 * Allows visualization of physics collision shapes, wireframes, and debug info
 */
class CEBulletDebugDraw : public btIDebugDraw
{
private:
    // Debug modes that can be enabled/disabled
    int m_debugMode;
    
    // OpenGL rendering state
    GLuint m_shaderProgram;
    GLuint m_VAO, m_VBO;
    
    // Helper methods for shader compilation
    GLuint compileShader(GLenum type, const char* source);
    
    // Vertex data for line rendering
    struct DebugVertex {
        glm::vec3 position;
        glm::vec3 color;
    };
    std::vector<DebugVertex> m_vertices;
    
    // Camera matrices for proper 3D rendering
    glm::mat4 m_viewProjectionMatrix;
    glm::vec3 m_cameraPosition;  // For distance culling
    
    void initializeGL();
    void flushLines();

public:
    CEBulletDebugDraw();
    virtual ~CEBulletDebugDraw();
    
    // btIDebugDraw interface implementation
    virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;
    virtual void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) override;
    virtual void reportErrorWarning(const char* warningString) override;
    virtual void draw3dText(const btVector3& location, const char* textString) override;
    virtual void setDebugMode(int debugMode) override;
    virtual int getDebugMode() const override;
    
    // Additional methods for integration with renderer
    void setViewProjectionMatrix(const glm::mat4& viewProjectionMatrix);
    void setCameraPosition(const glm::vec3& cameraPosition);  // For distance culling
    void beginFrame();
    void endFrame();
};