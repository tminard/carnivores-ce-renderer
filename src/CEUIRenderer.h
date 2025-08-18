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
#include "camera.h"
#include "transform.h"

class LocalAudioManager;
class CEAnimation;
class CEBulletProjectileManager;
class Camera;

class CEUIRenderer
{
public:
    enum class WeaponState { HOLSTERED, DRAWING, DRAWN, HOLSTERING, FIRING, RELOADING };

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
    std::string m_weaponFireAnimation;    // Fire animation name from config
    std::string m_weaponReloadAnimation;  // Reload animation name from config
    
    // Ammo tracking system
    int m_weaponMaxRounds;        // Maximum ammo capacity
    int m_weaponCurrentRounds;    // Current ammo remaining
    
    LocalAudioManager* m_audioManager;
    CEBulletProjectileManager* m_projectileManager;
    Camera* m_gameCamera;
    
    // Projectile configuration
    float m_muzzleVelocity;
    glm::vec3 m_muzzleOffset;
    float m_projectileDamage;
    float m_recoilStrength;
    float m_swayStrength;
    
    // Sway state variables
    double m_swayTime;
    glm::vec2 m_swayOffset;
    
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
    void fireWeapon(); // Fire weapon if drawn
    void reloadWeapon(); // Reload weapon if drawn
    void renderWeapon(C2CarFile* weapon, double currentTime);
    bool isWeaponDrawn() const { return m_weaponDrawn; }
    WeaponState getWeaponState() const { return m_weaponState; }
    
    // Configuration
    void configureWeaponAnimations(const std::string& drawAnim, const std::string& holsterAnim, const std::string& fireAnim, const std::string& reloadAnim);
    void setAudioManager(LocalAudioManager* audioManager);
    void setProjectileManager(CEBulletProjectileManager* projectileManager);
    void setGameCamera(Camera* camera);
    void configureProjectiles(float muzzleVelocity, const glm::vec3& muzzleOffset, float damage);
    void configureAmmo(int maxRounds);
    void configureRecoil(float recoilStrength);
    void configureSway(float swayStrength);
    void updateSway(double currentTime, float playerVelocityMagnitude);
    
    // Ammo information getters
    int getCurrentAmmo() const { return m_weaponCurrentRounds; }
    int getMaxAmmo() const { return m_weaponMaxRounds; }
    
    // Sway information getter
    glm::vec2 getCurrentSwayOffset() const { return m_swayOffset; }
    
    // Set up 2D rendering state
    void begin2DRendering();
    
    // Restore 3D rendering state
    void end2DRendering();
    
    // Text rendering for debug information
    void renderText(const std::string& text, float x, float y, float scale = 1.0f, glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f));
    
private:
    // Text rendering system
    void initializeTextRendering();
    void createFontTexture();
    void renderCharacterQuad(char c, float x, float y, float scale);
    void renderCharacter(char c, float x, float y, float scale, glm::vec3 color);
    
    GLuint m_textVAO;
    GLuint m_textVBO;
    GLuint m_fontTexture;
    std::unique_ptr<ShaderProgram> m_textShader;
    bool m_textInitialized;
    
    // Font metrics
    static const int FONT_ATLAS_WIDTH = 128;
    static const int FONT_ATLAS_HEIGHT = 128;
    static const int CHAR_WIDTH = 8;
    static const int CHAR_HEIGHT = 16;
};

#endif /* defined(__CE_Character_Lab__CEUIRenderer__) */
