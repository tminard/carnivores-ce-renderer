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
#include "Camera.h"
#include "Transform.h"

class CEUIRenderer
{
private:
    std::unique_ptr<Camera> m_ui2DCamera;
    int m_screenWidth, m_screenHeight;
    
    void initializeUI2DCamera();
    void renderCompassGeometry(C2CarFile* compass, Transform& uiTransform);
    void renderTestSquare();
    
public:
    CEUIRenderer(int screenWidth, int screenHeight);
    ~CEUIRenderer();
    
    // Set up orthographic projection for 2D rendering
    void setScreenSize(int width, int height);
    
    // Render compass in lower-left corner  
    void renderCompass(C2CarFile* compass, float rotation);
    void renderCompass(C2CarFile* compass, Camera* gameCamera);
    
    // Set up 2D rendering state
    void begin2DRendering();
    
    // Restore 3D rendering state
    void end2DRendering();
};

#endif /* defined(__CE_Character_Lab__CEUIRenderer__) */
