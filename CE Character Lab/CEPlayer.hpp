#pragma once

// A `CEPlayer` represents a human player state. This includes location, position, viewing angle, health, etc.

#include <glm/vec3.hpp>


class CEPlayer
{
private:
//  const float WALK_SPEED = 8.f;
//  const float RUN_SPEED = 16.f;
//  const float NOCLIP_SPEED = 150.f;
//  const float HEIGHT = 128.f;

  glm::vec3 m_position;

public:
  glm::vec3 getPosition();
  void setPosition(glm::vec3 position);
};
