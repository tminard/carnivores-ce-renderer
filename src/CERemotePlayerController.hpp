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

#include "CEGeometry.h"

class C2MapFile;
class C2CarFile;
class C2MapRscFile;
class LocalAudioManager;
struct Camera;
struct Transform;

/**
 Class for remote controlled entities. This includes local AI control or avatars represented by remote human players.
 Using this class, local entities can be observed via a camera that can be used to render perspective.
 For local controllers like AI, commands will be directly passed in to direct the player. For network, a network controller
 will provide commands.
 For example, the local player controller is owned by an input manager, which directs keyboard input to the controller.
 
 Finally, controller has a reference to an avatar which will be rendered on the map.
 */
class CERemotePlayerController : CEObservable
{
private:
  std::shared_ptr<C2MapFile> m_map;
  std::shared_ptr<C2MapRscFile> m_rsc;
  std::shared_ptr<LocalAudioManager> m_g_audio_manager;
  
  // Shared reference to CAR. Defines audio, texture, animations, etc
  std::shared_ptr<C2CarFile> m_car;
  
  // We need a unique copy of geometry for local avatar animations
  std::unique_ptr<CEGeometry> m_geo;
  
  std::string m_current_animation;
  // Currently always 0, but eventually marks the start of an ani
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
  
  bool m_is_jumping = false;
  float m_vertical_speed = 0.0f;
  const float m_jump_speed = 12.f;
  const float m_gravity = 48.f;
  double m_last_jump_time = 0.0;
  const double m_jump_cooldown = 0.25; // Cooldown period in seconds
  
  bool m_is_deployed;
  
  void uploadStateToHardware();

public:
  CERemotePlayerController(std::shared_ptr<LocalAudioManager> audioManager, std::shared_ptr<C2CarFile> carFile, std::shared_ptr<C2MapFile> map, std::shared_ptr<C2MapRscFile> rsc, std::string initialAnimationName);

  glm::vec3 getPosition() const;
  glm::vec2 getWorldPosition() const;

  void setPosition(glm::vec3 position);
  void setElevation(float elevation);
  void lookAt(glm::vec3 direction);

  void update(double currentTime, Transform &baseTransform, Camera &observerCamera, glm::vec2 observerWorldPosition);
  
  void move(double currentTime, double deltaTime, bool forwardPressed, bool backwardPressed, bool rightPressed, bool leftPressed);
  
  void jump(double currentTime);
  
  void Render();

  Camera* getCamera();
};
