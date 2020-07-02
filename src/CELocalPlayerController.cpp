#include "CELocalPlayerController.hpp"
#include "CEPlayer.hpp"
#include "camera.h"

#include <iostream>
#include <string>

CELocalPlayerController::CELocalPlayerController(std::shared_ptr<CEPlayer> player, float world_width, float world_height, float tile_size) : m_player(player), m_world_width(world_width), m_world_height(world_height), m_tile_size(tile_size) {
}

void CELocalPlayerController::lookAt(glm::vec3 direction)
{
  this->m_camera.SetLookAt(direction);
}

void CELocalPlayerController::update()
{
  // Game loop update
  // TODO: all input commands (move etc) should impact player not camera state. Then, update loop updates camera to track player.
}

void CELocalPlayerController::DBG_printLocationInformation() const
{
  glm::vec3 cur_pos = this->m_camera.GetPosition();
  glm::vec2 cur_world_pos = this->getWorldPosition();

  std::cout << '\n' << "===> CURRENT POSITION IN SPACE <===" <<
          '\n' << "X: " << std::to_string(cur_pos.x) <<
          '\n' << "Y: " << std::to_string(cur_pos.y) <<
          '\n' << "Z: " << std::to_string(cur_pos.z);

  std::cout << '\n' << "===> CURRENT WORLD POSITION <===" <<
          '\n' << "X: " << std::to_string(cur_world_pos.x) <<
          '\n' << "Y: " << std::to_string(cur_world_pos.y);
}

Camera* CELocalPlayerController::getCamera()
{
  return &this->m_camera;
}

glm::vec3 CELocalPlayerController::getPosition() const
{
  return this->m_camera.GetPosition();
}

glm::vec2 CELocalPlayerController::getWorldPosition() const
{
  glm::vec3 pos = m_camera.GetPosition();

  return glm::vec2(int(floorf(pos.x / m_tile_size)), int(floorf(pos.z / m_tile_size)));
}

void CELocalPlayerController::setPosition(glm::vec3 position)
{
  this->m_camera.SetPos(position);
}

void CELocalPlayerController::moveForward()
{
  this->m_camera.MoveForward(150.f);
}
void CELocalPlayerController::moveBackward() {}
void CELocalPlayerController::strafeRight() {}
void CELocalPlayerController::strafeLeft() {}
