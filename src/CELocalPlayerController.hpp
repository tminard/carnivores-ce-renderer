#pragma once

#include <stdio.h>
#include <memory>

#include <glm/vec3.hpp>
#include "CEObservable.hpp"

class C2MapFile;
class C2MapRscFile;

class CELocalPlayerController : CEObservable
{
private:
  std::shared_ptr<C2MapFile> m_map;
  std::shared_ptr<C2MapRscFile> m_rsc;

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

  float computeSlope(float x, float z);
  glm::vec3 computeSlidingDirection(float x, float z);
  
  const float maxSlopeAngle = 45.0f;
  float m_previousHeight = 0.0f;
  
  bool m_is_jumping = false;
  float m_vertical_speed = 0.0f;
  const float m_jump_speed = 12.f;
  const float m_gravity = 48.f;
  double m_last_jump_time = 0.0;
  const double m_jump_cooldown = 0.15; // Cooldown period in seconds
  double m_died_at = 0.0;
  bool m_dead = false;
  glm::vec3 m_body_at;

public:
  CELocalPlayerController(float world_width, float world_height, float tile_size, std::shared_ptr<C2MapFile> map, std::shared_ptr<C2MapRscFile> rsc);

  glm::vec3 getPosition() const;
  glm::vec2 getWorldPosition() const;

  void setPosition(glm::vec3 position);
  void setElevation(float elevation);
  
  void kill(double killedAt);
  void panAroundBody(double currentTime);

  void lookAt(glm::vec3 direction);

  void update(double timeDelta, double currentTime);

  void moveForward(double timeDelta);
  void moveBackward(double timeDelta);
  void strafeRight(double timeDelta);
  void strafeLeft(double timeDelta);
  
  void move(double currentTime, double deltaTime, bool forwardPressed, bool backwardPressed, bool rightPressed, bool leftPressed);
  
  void jump(double currentTime);

  void DBG_printLocationInformation() const;
  
  bool isAlive(double currentTime);

  Camera* getCamera();
};
