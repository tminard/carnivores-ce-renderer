//
//  CEUIRenderer.h
//  CE Character Lab
//
//  Created by Claude Code for 2D UI rendering system
//

#ifndef __CE_Character_Lab__CEUIRenderer__
#define __CE_Character_Lab__CEUIRenderer__

#include <memory>
#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "shader_program.h"
#include "C2CarFile.h"

class CEUIRenderer
{
private:
    std::unique_ptr<ShaderProgram> m_uiShader;
    glm::mat4 m_projectionMatrix;
    int m_screenWidth, m_screenHeight;
    
    void initializeShaders();
    void renderCompassGeometry(C2CarFile* compass);
    void renderTestSquare();
    
public:
    CEUIRenderer(int screenWidth, int screenHeight);
    ~CEUIRenderer();
    
    // Set up orthographic projection for 2D rendering
    void setScreenSize(int width, int height);
    
    // Render compass in lower-left corner  
    void renderCompass(C2CarFile* compass, float rotation);
    
    // Set up 2D rendering state
    void begin2DRendering();
    
    // Restore 3D rendering state
    void end2DRendering();
    
    // Create transformation matrix for UI element positioning
    glm::mat4 createUITransform(const glm::vec2& position, const glm::vec2& scale, float rotation) const;
};

#endif /* defined(__CE_Character_Lab__CEUIRenderer__) */
