  //
  //  CEPlayer.cpp
  //  CE Character Lab
  //
  //  Created by Minard, Tyler on 6/8/18.
  //  Copyright © 2018 Tyler Minard. All rights reserved.
  //

#include "CEPlayer.hpp"

#include "C2MapFile.h"
#include "TerrainRenderer.h"

CEPlayer::CEPlayer(std::shared_ptr<C2MapFile> map) : m_map(map)
{
  this->m_last_update_seconds = glfwGetTime();
  this->m_direction_sec_velocity = glm::vec3(0.f);
  this->m_acceleration_per_sec = glm::vec3(1.f);
}

void CEPlayer::setLookAt(glm::vec3 direction)
{
  this->m_camera.SetLookAt(direction);
}

void CEPlayer::setVelocity(glm::vec3 velocity)
{
  m_direction_sec_velocity += velocity;
  m_direction_sec_velocity.y = fmaxf(m_direction_sec_velocity.y, TERM_VELOCITY_Y); // TODO: allow exceeding terminal - velocity
}

void CEPlayer::setVelocity(glm::vec3 velocity, glm::vec3 acceleration_per_sec)
{
  this->setVelocity(velocity);
  this->m_acceleration_per_sec = glm::clamp(m_acceleration_per_sec + acceleration_per_sec, -1.f, 1.f); // TODO:: accumulate this with a cap?
}

void CEPlayer::applyVelocity(double time_delta)
{
  m_direction_sec_velocity = m_direction_sec_velocity * m_acceleration_per_sec;

  if (m_direction_sec_velocity == glm::vec3(0.f)) return;
  glm::vec3 current_position = m_camera.GetCurrentPos();

  glm::vec3 offset = current_position + (m_direction_sec_velocity * (float)time_delta);

  this->m_camera.SetPos(offset);
}

void CEPlayer::update()
{
  double current_time = glfwGetTime();
  glm::vec3 current_position = m_camera.GetCurrentPos();

  double time_delta_sec = current_time - m_last_update_seconds;

  float current_height = current_position.y;
  glm::vec3 gravity(0.f);

  if (current_height != m_target_height && m_direction_sec_velocity.y >= TERM_VELOCITY_Y) {
    if (current_height > m_target_height) {

      float delta = current_height - m_target_height;
      if (delta <= this->m_map->getTileLength() * 0.5f) {
        m_camera.SetHeight(m_target_height);
      } else {
        gravity = m_direction_sec_velocity + glm::vec3(0, -(1024.f * (float)time_delta_sec), 0);
        m_direction_sec_velocity.y = fmaxf(gravity.y, TERM_VELOCITY_Y);

        this->applyVelocity(time_delta_sec);

        current_height = m_camera.GetCurrentPos().y;
        if (current_height < m_target_height) {
          m_direction_sec_velocity.y = 0;
          m_camera.SetHeight(m_target_height);
        }
      }
    } else if (current_height < m_target_height) {
        // over time adjust ("standup")
      m_direction_sec_velocity.y = fminf(m_direction_sec_velocity.y + 45.f, 255.f);

      this->applyVelocity(time_delta_sec);

      current_height = m_camera.GetCurrentPos().y;
      if (current_height > m_target_height) {
        m_direction_sec_velocity.y = 0;
        m_camera.SetHeight(m_target_height);
      }
    }
  } else {
    this->applyVelocity(time_delta_sec);
  }

  m_is_grounded = (m_direction_sec_velocity.y == 0);

  this->m_last_update_seconds = current_time;
}

bool CEPlayer::canMoveSide(float amount)
{
  if (!m_is_grounded) return false;
    // TODO: Make this and forward the same; include slope checks
  glm::vec3 pos = m_camera.GetCurrentPos();
  glm::vec3 right = m_camera.GetRight();

    // TODO: Choose a diff approach
  glm::vec3 new_pos = pos + ((right * glm::vec3(1.f, 0, 1.f)) * amount); // todo: get slope delta angles
  glm::vec2 world_pos = this->m_map->getXYAtWorldPosition(glm::vec2(new_pos.x, new_pos.z));
  int nxy = (world_pos.y * this->m_map->getWidth()) + world_pos.x;
  float new_height = this->m_map->getHeightAt(nxy) + HEIGHT;
  new_pos = glm::vec3(new_pos.x, new_height, new_pos.z);

  float l = glm::distance(new_pos, pos);
  float hd = new_height - pos.y;

  if (l > 80.f && hd > 0) return false; // gravity is 1024.0 world units/second
                                        // TODO: slope

  return true;
}

bool CEPlayer::canMoveForward(float amount)
{
  if (!m_is_grounded) return false;

  glm::vec3 pos = m_camera.GetCurrentPos();
  glm::vec3 forward = m_camera.GetForward();

    // TODO: Choose a diff approach
  glm::vec3 new_pos = pos + ((forward * glm::vec3(1.f, 0, 1.f)) * amount); // todo: get slope delta angles
  glm::vec2 world_pos = this->m_map->getXYAtWorldPosition(glm::vec2(new_pos.x, new_pos.z));
  int nxy = (world_pos.y * this->m_map->getWidth()) + world_pos.x;
  float new_height = this->m_map->getHeightAt(nxy) + HEIGHT;
  new_pos = glm::vec3(new_pos.x, new_height, new_pos.z);

  float l = glm::distance(new_pos, pos);
  float hd = new_height - pos.y;

    // TODO: slope
  if (l > 90.f && hd > 0) return false; // gravity is 1024.0 world units/second

  return true;
}

void CEPlayer::moveBackward()
{
  float amt = RUN_SPEED * -0.8f;

  if (canMoveForward(amt)) {
    this->m_camera.MoveForward(amt);

    glm::vec3 pos = m_camera.GetCurrentPos();
    glm::vec2 world_pos = this->m_map->getXYAtWorldPosition(glm::vec2(pos.x, pos.z));
    int nxy = (world_pos.y * this->m_map->getWidth()) + world_pos.x;
    float new_height = this->m_map->getHeightAt(nxy) + HEIGHT;

      // TODO: use physics engine for this? If target height is below our height, then let "gravity" pull us down
      // Otherwise, "step up" to the height (move into new position preserving existing height, then "adjust" up to new height over a smooth time period.
    m_target_height = new_height;
  }
}

void CEPlayer::strafeRight()
{
  float amt = (RUN_SPEED * 0.8f) * -1.f;

  if (canMoveSide(amt)) {
    this->m_camera.MoveRight(amt);

    glm::vec3 pos = m_camera.GetCurrentPos();
    glm::vec2 world_pos = this->m_map->getXYAtWorldPosition(glm::vec2(pos.x, pos.z));
    int nxy = (world_pos.y * this->m_map->getWidth()) + world_pos.x;
    float new_height = this->m_map->getHeightAt(nxy) + HEIGHT;

      // TODO: use physics engine for this? If target height is below our height, then let "gravity" pull us down
      // Otherwise, "step up" to the height (move into new position preserving existing height, then "adjust" up to new height over a smooth time period.
    m_target_height = new_height;
  }
}

void CEPlayer::strafeLeft()
{
  float amt = RUN_SPEED * 0.8f;

  if (canMoveSide(amt)) {
    this->m_camera.MoveRight(amt);

    glm::vec3 pos = m_camera.GetCurrentPos();
    glm::vec2 world_pos = this->m_map->getXYAtWorldPosition(glm::vec2(pos.x, pos.z));
    int nxy = (world_pos.y * this->m_map->getWidth()) + world_pos.x;
    float new_height = this->m_map->getHeightAt(nxy) + HEIGHT;

      // TODO: use physics engine for this? If target height is below our height, then let "gravity" pull us down
      // Otherwise, "step up" to the height (move into new position preserving existing height, then "adjust" up to new height over a smooth time period.
    m_target_height = new_height;
  }
}

bool CEPlayer::canJump()
{
  if (m_direction_sec_velocity.y == 0) return true;

  return false;
}

void CEPlayer::moveForward()
{
  float amt = RUN_SPEED;

  if (canMoveForward(amt)) {
    this->m_camera.MoveForward(amt);
//    glm::vec3 fw = this->m_camera.GetForward();
//    //this->setVelocity(glm::vec3 velocity, glm::vec3 acceleration_per_sec);
//    this->setVelocity(fw+amt, fw*-1.f);

    glm::vec3 pos = m_camera.GetCurrentPos();
    glm::vec2 world_pos = this->m_map->getXYAtWorldPosition(glm::vec2(pos.x, pos.z));
    int nxy = (world_pos.y * this->m_map->getWidth()) + world_pos.x;
    float new_height = this->m_map->getHeightAt(nxy) + HEIGHT;

    // TODO: use physics engine for this? If target height is below our height, then let "gravity" pull us down
    // Otherwise, "step up" to the height (move into new position preserving existing height, then "adjust" up to new height over a smooth time period.
    m_target_height = new_height;
  }
}

glm::vec2 CEPlayer::getWorldPosition()
{
  int x, y;
  glm::vec3 pos = this->getPosition();

  x = int(floorf(pos.x / m_map->getTileLength()));
  y = int(floorf(pos.z / m_map->getTileLength()));
  return glm::vec2(x, y);
}

void CEPlayer::setWorldPosition(int x, int y)
{
  int xy = (y * this->m_map->getWidth()) + x;

  if (x >= this->m_map->getHeight() || y >= this->m_map->getWidth()) {
    throw std::runtime_error("Invalid location specified");
  }
  float tile_length = this->m_map->getTileLength();
  float real_height = this->m_map->getHeightAt(xy);

  float real_x = (tile_length * x) + (tile_length / 2);
  float real_y = real_height + HEIGHT;
  float real_z = (tile_length * y) + (tile_length / 2);

  m_target_height = real_y;

  glm::vec3 position(real_x, real_y + 1024.f, real_z);

  this->m_camera.SetPos(position);
}
