#pragma once

#include <stdio.h>
#include <memory>

#include <glm/vec3.hpp>
#include "CEObservable.hpp"

class CEPlayer;

class CELocalPlayerController : CEObservable
{
private:
  std::shared_ptr<CEPlayer> m_player;

  float m_world_width;
  float m_world_height;
  float m_tile_size;

public:
  CELocalPlayerController(std::shared_ptr<CEPlayer> player, float world_width, float world_height, float tile_size);

  glm::vec3 getPosition() const;
  glm::vec2 getWorldPosition() const;

  void setPosition(glm::vec3 position);

  void lookAt(glm::vec3 direction);

  void update();

  void moveForward();
  void moveBackward();
  void strafeRight();
  void strafeLeft();

  void DBG_printLocationInformation() const;

  Camera* getCamera();
};
