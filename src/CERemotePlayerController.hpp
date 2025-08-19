//
//  CERemotePlayerController.hpp
//  CarnivoresRenderer
//
//  Created by Tyler Minard on 7/30/24.
//

#pragma once

#include <stdio.h>
#include <memory>

#include <glm/vec3.hpp>
#include "CEObservable.hpp"
#include "CEBasePlayerController.hpp"

#include "CEGeometry.h"

class C2MapFile;
class C2CarFile;
class C2MapRscFile;
class LocalAudioManager;
struct Camera;
struct Transform;
class CEGeometry;

/**
 Class for remote controlled entities. This includes local AI control or avatars represented by remote human players.
 Using this class, local entities can be observed via a camera that can be used to render perspective.
 For local controllers like AI, commands will be directly passed in to direct the player. For network, a network controller
 will provide commands.
 For example, the local player controller is owned by an input manager, which directs keyboard input to the controller.
 
 Finally, controller has a reference to an avatar which will be rendered on the map.
 */
class CERemotePlayerController : public CEBasePlayerController, public CEObservable
{
private:
  std::shared_ptr<LocalAudioManager> m_g_audio_manager;
  
  // Shared reference to CAR. Defines audio, texture, animations, etc
  std::shared_ptr<C2CarFile> m_car;
  
  // We need a unique copy of geometry for local avatar animations
  std::unique_ptr<CEGeometry> m_geo;
  
  std::string m_current_animation;

  bool m_is_looping_anim = false;
  double m_animation_started_at;
  double m_animation_last_update_at;
  
  float m_walk_speed;
  float m_player_height;
  
  float m_current_speed;
  float m_target_speed;
  float m_acceleration;
  float m_deceleration;
  
  const float maxSlopeAngle = 45.0f;
  float m_previousHeight = 0.0f;
  
  // Enhanced height sampling parameters (similar to player)
  float m_predictiveDistance = 1.2f; // Distance ahead to sample (in tiles)
  
  // Adaptive smoothing parameters for AI
  float m_baseSmoothingFactor = 0.12f; // Slightly less aggressive than player
  float m_minSmoothingFactor = 0.04f;
  float m_maxSmoothingFactor = 0.22f;
  
  // Terrain angle adjustment
  float m_currentPitchAngle = 0.0f;
  float m_targetPitchAngle = 0.0f;
  float m_pitchSmoothingSpeed = 3.0f;
  
  // Animation speed synchronization
  float m_baseWalkAnimSpeed = 1.0f;   // Base speed for walk animation
  float m_baseRunAnimSpeed = 1.0f;    // Base speed for run animation
  
  // Death/frozen state
  bool m_isFrozen = false;
  bool m_shouldLockAtEnd = false; // Flag to lock at final frame when animation finishes
  
  
  bool m_is_jumping = false;
  float m_vertical_speed = 0.0f;
  const float m_jump_speed = 12.f;
  const float m_gravity = 48.f;
  double m_last_jump_time = 0.0;
  double m_start_turn_time = -1.0;
  glm::vec3 m_start_turn_lookat = glm::vec3(0.f);
  const double m_jump_cooldown = 0.25; // Cooldown period in seconds

private:
  // Enhanced height sampling methods (similar to player)
  float computeSlope(float x, float z);
  float getPredictiveHeight(const glm::vec3& currentPos, const glm::vec3& movementDir, float speed);
  float getAdaptiveSmoothingFactor(float heightDifference, float speed, float slopeAngle);
  float calculateTerrainPitch(const glm::vec3& position, const glm::vec3& forward);
  float calculateAnimationPlaybackSpeed();
  bool isMovementAnimation(const std::string& animationName);
  
  // Terrain alignment methods
  glm::vec3 calculateTerrainNormal(const glm::vec3& position);
  float calculateTerrainRoll(const glm::vec3& position, const glm::vec3& rightDirection);
  float calculateFootprintHeight(const glm::vec3& centerPosition, const glm::vec3& facingDirection);
  glm::vec3 extractEulerAngles(const glm::mat3& rotMatrix);
  
  
public:
  void uploadStateToHardware();
  CERemotePlayerController(std::shared_ptr<LocalAudioManager> audioManager, std::shared_ptr<C2CarFile> carFile, std::shared_ptr<C2MapFile> map, std::shared_ptr<C2MapRscFile> rsc, std::string initialAnimationName);

  // CEBasePlayerController interface implementation
  glm::vec3 getPosition() const override;
  glm::vec2 getWorldPosition() const override;
  void setPosition(glm::vec3 position) override;
  void setElevation(float elevation) override;
  void lookAt(glm::vec3 direction) override;
  Camera* getCamera() override;
  void update(double currentTime, double deltaTime) override;
  std::string getControllerType() const override { return "remote_ai"; }
  bool isRemoteController() const override { return true; }
  
  // Remote controller specific methods
  void setNextAnimation(std::string animationName, bool loop = true);
  const std::string getCurrentAnimation() const;
  bool isAnimPlaying(double currentTime);
  
  void setWalkSpeed(float speedFactor);
  void setHeightOffset(float offset);
  void setTargetSpeed(float targetSpeed);

  void updateWithObserver(double currentTime, Transform &baseTransform, Camera &observerCamera, glm::vec2 observerWorldPosition);
  
  void StopMovement();
  void MoveTo(glm::vec3 targetPosition, double currentTime);
  void UpdateLookAtDirection(glm::vec3 desiredLookAt, double currentTime);
  void SetFacingDirection(glm::vec3 direction);
  
  bool isTurning();
  
  void jump(double currentTime);
  
  // Death state management
  void freezeAnimation() { m_isFrozen = true; }
  void unfreezeAnimation() { m_isFrozen = false; }
  bool isFrozen() const { return m_isFrozen; }
  void lockAtFinalFrame(Transform& transform, Camera& camera); // Lock animation at final frame
  
  // Animation control helpers
  void setAnimationAndFreeze(const std::string& animationName); // Play once then freeze at final frame
  void holdCurrentFrame(); // Stop animation progression but keep current frame
  bool hasAnimationFinished(double currentTime); // Check if current non-looping animation has finished
  
  CEGeometry* getGeometry() const;
  
  void Render();
};
