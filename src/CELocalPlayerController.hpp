#pragma once

#include <stdio.h>
#include <memory>

#include <glm/vec3.hpp>
#include "CEObservable.hpp"

class CEPlayer;
class C2MapFile;

class CELocalPlayerController : CEObservable
{
private:
  std::shared_ptr<CEPlayer> m_player;
    std::shared_ptr<C2MapFile> m_map;

  float m_world_width;
  float m_world_height;
  float m_tile_size;
  
  float m_walk_speed;
  float m_player_height;
  
  float m_current_speed;
  float m_target_speed;
  float m_acceleration;
  float m_deceleration;
  
  float m_bobble_speed;
  float m_bobble_amount;
  float m_bobble_time;
  
  float interpolateHeight(float x, float z);
  float computeSlope(float x, float z);
  glm::vec3 computeSlidingDirection(float x, float z);
  
  const float maxSlopeAngle = 45.0f;
  float m_previousHeight = 0.0f;
  
  bool m_is_jumping = false;
  float m_vertical_speed = 0.0f;
  const float m_jump_speed = 12.f;
  const float m_gravity = 48.f;
  double m_last_jump_time = 0.0;
  const double m_jump_cooldown = 0.25; // Cooldown period in seconds

public:
  CELocalPlayerController(std::shared_ptr<CEPlayer> player, float world_width, float world_height, float tile_size, std::shared_ptr<C2MapFile> map);

  glm::vec3 getPosition() const;
  glm::vec2 getWorldPosition() const;

  void setPosition(glm::vec3 position);
  void setElevation(float elevation);

  void lookAt(glm::vec3 direction);

  void update(double timeDelta);

  void moveForward(double timeDelta);
  void moveBackward(double timeDelta);
  void strafeRight(double timeDelta);
  void strafeLeft(double timeDelta);
  
  void move(double currentTime, double deltaTime, bool forwardPressed, bool backwardPressed, bool rightPressed, bool leftPressed);
  
  void jump(double currentTime);

  void DBG_printLocationInformation() const;

  Camera* getCamera();
};
