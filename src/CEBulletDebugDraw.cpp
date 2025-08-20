#include "CEBulletDebugDraw.h"
#include <iostream>

// Simple vertex shader for line rendering
const char* lineVertexShader = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

uniform mat4 viewProjection;

out vec3 vertexColor;

void main()
{
    gl_Position = viewProjection * vec4(aPos, 1.0);
    vertexColor = aColor;
}
)";

// Simple fragment shader for line rendering
const char* lineFragmentShader = R"(
#version 330 core
in vec3 vertexColor;
out vec4 FragColor;

void main()
{
    FragColor = vec4(vertexColor, 1.0);
}
)";

CEBulletDebugDraw::CEBulletDebugDraw()
    : m_debugMode(btIDebugDraw::DBG_DrawWireframe | btIDebugDraw::DBG_DrawContactPoints)
    , m_shaderProgram(0)
    , m_VAO(0)
    , m_VBO(0)
    , m_viewProjectionMatrix(1.0f)
    , m_cameraPosition(0.0f)
{
    initializeGL();
}

CEBulletDebugDraw::~CEBulletDebugDraw()
{
    if (m_VBO) glDeleteBuffers(1, &m_VBO);
    if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
    if (m_shaderProgram) glDeleteProgram(m_shaderProgram);
}

GLuint CEBulletDebugDraw::compileShader(GLenum type, const char* source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    
    // Check for compilation errors
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

void CEBulletDebugDraw::initializeGL()
{
    // Create and compile shaders
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, lineVertexShader);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, lineFragmentShader);
    
    if (vertexShader == 0 || fragmentShader == 0) {
        std::cerr << "Failed to compile debug line shaders" << std::endl;
        return;
    }
    
    // Create shader program
    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, vertexShader);
    glAttachShader(m_shaderProgram, fragmentShader);
    glLinkProgram(m_shaderProgram);
    
    // Check for linking errors
    GLint success;
    glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(m_shaderProgram, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    
    // Clean up shaders (they're now linked into the program)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    // Create VAO and VBO for line rendering
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    
    // Position attribute (location 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Color attribute (location 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)offsetof(DebugVertex, color));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
    
    std::cout << "🔧 Bullet Physics debug drawer initialized" << std::endl;
}

void CEBulletDebugDraw::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
    // Convert Bullet vectors to GLM
    glm::vec3 fromPos(from.x(), from.y(), from.z());
    glm::vec3 toPos(to.x(), to.y(), to.z());
    glm::vec3 lineColor(color.x(), color.y(), color.z());
    
    // Add vertices for the line
    m_vertices.push_back({fromPos, lineColor});
    m_vertices.push_back({toPos, lineColor});
}

void CEBulletDebugDraw::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color)
{
    // Draw contact point as a small cross
    glm::vec3 point(PointOnB.x(), PointOnB.y(), PointOnB.z());
    glm::vec3 normal(normalOnB.x(), normalOnB.y(), normalOnB.z());
    glm::vec3 contactColor(color.x(), color.y(), color.z());
    
    float size = 0.1f; // Size of contact point visualization
    
    // Draw normal line
    glm::vec3 normalEnd = point + normal * size * 2.0f;
    m_vertices.push_back({point, contactColor});
    m_vertices.push_back({normalEnd, contactColor});
    
    // Draw small cross at contact point
    glm::vec3 right = glm::normalize(glm::cross(normal, glm::vec3(0, 1, 0))) * size;
    glm::vec3 up = glm::normalize(glm::cross(normal, right)) * size;
    
    m_vertices.push_back({point - right, contactColor});
    m_vertices.push_back({point + right, contactColor});
    m_vertices.push_back({point - up, contactColor});
    m_vertices.push_back({point + up, contactColor});
}

void CEBulletDebugDraw::reportErrorWarning(const char* warningString)
{
    std::cout << "🚨 Bullet Physics Warning: " << warningString << std::endl;
}

void CEBulletDebugDraw::draw3dText(const btVector3& location, const char* textString)
{
    // Text rendering not implemented - would require additional text rendering setup
    // Could be added later if needed
}

void CEBulletDebugDraw::setDebugMode(int debugMode)
{
    m_debugMode = debugMode;
}

int CEBulletDebugDraw::getDebugMode() const
{
    return m_debugMode;
}

void CEBulletDebugDraw::setViewProjectionMatrix(const glm::mat4& viewProjectionMatrix)
{
    m_viewProjectionMatrix = viewProjectionMatrix;
}

void CEBulletDebugDraw::setCameraPosition(const glm::vec3& cameraPosition)
{
    m_cameraPosition = cameraPosition;
}

void CEBulletDebugDraw::beginFrame()
{
    // Clear vertex buffer for new frame
    m_vertices.clear();
}

void CEBulletDebugDraw::endFrame()
{
    // Render all accumulated lines
    flushLines();
}

void CEBulletDebugDraw::flushLines()
{
    if (m_vertices.empty() || m_shaderProgram == 0) {
        return;
    }
    
    // Performance monitoring: log vertex count occasionally
    static int frameCount = 0;
    if (++frameCount % 60 == 0) { // Log every 60 frames (once per second at 60fps)
        std::cout << "🔧 Debug rendering: " << m_vertices.size() << " vertices (" << (m_vertices.size()/2) << " lines)" << std::endl;
    }
    
    // Enable line rendering
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL); // Allow debug lines to show through geometry
    
    // Use our line shader
    glUseProgram(m_shaderProgram);
    
    // Set view-projection matrix uniform
    GLint viewProjectionLoc = glGetUniformLocation(m_shaderProgram, "viewProjection");
    if (viewProjectionLoc != -1) {
        glUniformMatrix4fv(viewProjectionLoc, 1, GL_FALSE, &m_viewProjectionMatrix[0][0]);
    }
    
    // Update VBO with new vertex data
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(DebugVertex), m_vertices.data(), GL_DYNAMIC_DRAW);
    
    // Draw lines
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_vertices.size()));
    
    glBindVertexArray(0);
    
    // Clear vertices for next frame
    m_vertices.clear();
}
