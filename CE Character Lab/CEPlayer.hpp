#pragma once

#include "CEObservable.hpp"
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

#include <memory>

class C2MapFile;

class CEPlayer :
public CEObservable
{
private:
  const float WALK_SPEED = 8.f;
  const float RUN_SPEED = 16.f;
  const float NOCLIP_SPEED = 150.f;
  const float HEIGHT = 128.f;
  const float TERM_VELOCITY_Y = -2048.f;

  double m_last_update_seconds;

  float m_target_height;
  glm::vec3 m_direction_sec_velocity;

  std::shared_ptr<C2MapFile> m_map;
  bool canMoveForward(float amount);
  bool canMoveSide(float amount);
  void applyVelocity(double time_delta);
public:
  CEPlayer(std::shared_ptr<C2MapFile> map);
  glm::vec2 getWorldPosition();
  void setWorldPosition(int x, int y);

  void setLookAt(glm::vec3 direction);

  /*
   * pass in direction; speed is implied as /sec
   */
  void setVelocity(glm::vec3 velocity);

  bool canJump();

  void moveForward();
  void moveBackward();
  void strafeRight();
  void strafeLeft();

  void update();
};
