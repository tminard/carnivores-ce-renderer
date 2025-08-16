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

class LocalAudioManager;
class CEAnimation;

class CEUIRenderer
{
public:
    enum class WeaponState { HOLSTERED, DRAWING, DRAWN, HOLSTERING };

private:
    std::unique_ptr<Camera> m_ui2DCamera;
    int m_screenWidth, m_screenHeight;
    bool m_weaponDrawn;
    
    // Weapon animation system (following AI pattern)
    WeaponState m_weaponState;
    std::string m_currentWeaponAnimation;  // Current animation name
    double m_weaponAnimationStartTime;     // When animation was set to start
    double m_weaponAnimationLastUpdate;   // Last time SetAnimation was called
    bool m_weaponAnimationLoop;           // Whether to loop the animation
    bool m_weaponGeometryInitialized;     // Track if weapon geometry has been initialized
    std::string m_weaponDrawAnimation;    // Draw animation name from config
    std::string m_weaponHolsterAnimation; // Holster animation name from config
    LocalAudioManager* m_audioManager;
    
    void initializeUI2DCamera();
    void renderCompassGeometry(C2CarFile* compass, Transform& uiTransform);
    void renderWeaponGeometry(C2CarFile* weapon, Transform& weaponTransform);
    void setupWeaponRendering();
    void restoreNormalRendering();
    void renderTestSquare();
    
    // Weapon animation helpers (following AI pattern)
    void setWeaponAnimation(const std::string& animationName, bool loop);
    void updateWeaponAnimation(C2CarFile* weapon, double currentTime);
    void initializeWeaponGeometry(C2CarFile* weapon);
    std::shared_ptr<CEAnimation> getWeaponAnimation(C2CarFile* weapon, const std::string& animName);
    
public:
    CEUIRenderer(int screenWidth, int screenHeight);
    ~CEUIRenderer();
    
    // Set up orthographic projection for 2D rendering
    void setScreenSize(int width, int height);
    
    // Render compass in lower-left corner  
    void renderCompass(C2CarFile* compass, float rotation);
    void renderCompass(C2CarFile* compass, Camera* gameCamera);
    
    // Weapon system
    void toggleWeapon(); // Draw/holster weapon
    void renderWeapon(C2CarFile* weapon, double currentTime);
    bool isWeaponDrawn() const { return m_weaponDrawn; }
    WeaponState getWeaponState() const { return m_weaponState; }
    
    // Configuration
    void configureWeaponAnimations(const std::string& drawAnim, const std::string& holsterAnim);
    void setAudioManager(LocalAudioManager* audioManager);
    
    // Set up 2D rendering state
    void begin2DRendering();
    
    // Restore 3D rendering state
    void end2DRendering();
};

#endif /* defined(__CE_Character_Lab__CEUIRenderer__) */
