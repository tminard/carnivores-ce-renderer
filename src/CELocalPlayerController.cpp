#include "CELocalPlayerController.hpp"
#include "CEPlayer.hpp"
#include "camera.h"
#include "C2MapFile.h"

#include <iostream>
#include <string>

CELocalPlayerController::CELocalPlayerController(std::shared_ptr<CEPlayer> player, float world_width, float world_height, float tile_size, std::shared_ptr<C2MapFile> map) : m_player(player), m_world_width(world_width), m_world_height(world_height), m_tile_size(tile_size), m_map(map) {
  
  m_walk_speed = m_tile_size * 3.5f;  // This seems reasonable for walk speed
  m_player_height = m_tile_size * 0.85f;

  m_current_speed = 0.0f;
  m_target_speed = 0.0f;

  // Adjusted acceleration and deceleration values
  m_acceleration = m_walk_speed * 4.0f;  // Reduce to 1.0x walk speed for smoother acceleration
  m_deceleration = m_walk_speed * 1.5f;  // Increase to 0.5x walk speed for less abrupt deceleration
  
  m_bobble_speed = 0.1f;
  m_bobble_amount = m_player_height * 0.1f;
  m_bobble_time = 0.f;
}

void CELocalPlayerController::lookAt(glm::vec3 direction)
{
  this->m_camera.SetLookAt(direction);
}

void CELocalPlayerController::update(double deltaTime)
{
  float dTime = static_cast<float>(deltaTime);
  if (m_target_speed < m_current_speed) {
      m_current_speed -= m_deceleration * dTime;
      if (m_current_speed < m_target_speed) {
          m_current_speed = m_target_speed;
      }
  }
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

void CELocalPlayerController::setElevation(float elevation)
{
    this->m_camera.SetHeight(elevation);
}

void CELocalPlayerController::moveForward(double deltaTime)
{
  float dTime = static_cast<float>(deltaTime);
  
  m_target_speed = m_walk_speed;
  if (m_current_speed < m_target_speed) {
      m_current_speed += m_acceleration * dTime;
      if (m_current_speed > m_target_speed) m_current_speed = m_target_speed;
  } else if (m_current_speed > m_target_speed) {
      m_current_speed -= m_deceleration * dTime;
      if (m_current_speed < m_target_speed) m_current_speed = m_target_speed;
  }
  
  glm::vec3 forward = m_camera.GetForward() * (m_current_speed * dTime);
  glm::vec3 pos = m_camera.GetPosition() + forward;
  
  auto worldPos = glm::vec2(int(floorf(pos.x / m_tile_size)), int(floorf(pos.z / m_tile_size)));
  
  pos.y = m_map->getHeightAt((m_map->getWidth() * worldPos.y) + worldPos.x) + m_player_height;

  this->m_camera.SetPos(pos);
}

void CELocalPlayerController::moveBackward(double deltaTime) {
  float dTime = static_cast<float>(deltaTime);
  
  // Reduced acceleration for moving backward
  m_target_speed = m_walk_speed * 0.5f; // Adjust as needed
  if (m_current_speed < m_target_speed) {
      m_current_speed += m_acceleration * dTime * 0.5f; // Reduced acceleration
      if (m_current_speed > m_target_speed) m_current_speed = m_target_speed;
  }
  
  glm::vec3 forward = m_camera.GetForward() * (m_current_speed * dTime) * -1.f;
  glm::vec3 pos = m_camera.GetPosition() + forward;
  
  auto worldPos = glm::vec2(int(floorf(pos.x / m_tile_size)), int(floorf(pos.z / m_tile_size)));
  
  pos.y = m_map->getHeightAt((m_map->getWidth() * worldPos.y) + worldPos.x) + m_player_height;

  this->m_camera.SetPos(pos);
}

void CELocalPlayerController::strafeRight(double deltaTime) {
  float dTime = static_cast<float>(deltaTime);
  
  // Reduced acceleration for strafing
  m_target_speed = m_walk_speed * 0.7f; // Adjust as needed
  if (m_current_speed < m_target_speed) {
      m_current_speed += m_acceleration * dTime * 0.7f; // Reduced acceleration
      if (m_current_speed > m_target_speed) m_current_speed = m_target_speed;
  }
  
  glm::vec3 forward = m_camera.GetForward();
  glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f))); // Calculate the right vector
  right *= (m_current_speed * dTime); // Scale by speed and time delta

  glm::vec3 pos = m_camera.GetPosition() + right;
  auto worldPos = glm::vec2(int(floorf(pos.x / m_tile_size)), int(floorf(pos.z / m_tile_size)));
  
  pos.y = m_map->getHeightAt((m_map->getWidth() * worldPos.y) + worldPos.x) + m_player_height;

  this->m_camera.SetPos(pos);
}

void CELocalPlayerController::strafeLeft(double deltaTime) {
  float dTime = static_cast<float>(deltaTime);
  // Reduced acceleration for strafing
  m_target_speed = m_walk_speed * 0.7f; // Adjust as needed
  if (m_current_speed < m_target_speed) {
      m_current_speed += m_acceleration * dTime * 0.7f; // Reduced acceleration
      if (m_current_speed > m_target_speed) m_current_speed = m_target_speed;
  }
  
  glm::vec3 forward = m_camera.GetForward();
  glm::vec3 left = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), forward)); // Calculate the left vector
  left *= (m_current_speed * dTime); // Scale by speed and time delta

  glm::vec3 pos = m_camera.GetPosition() + left;
  auto worldPos = glm::vec2(int(floorf(pos.x / m_tile_size)), int(floorf(pos.z / m_tile_size)));
  
  pos.y = m_map->getHeightAt((m_map->getWidth() * worldPos.y) + worldPos.x) + m_player_height;

  this->m_camera.SetPos(pos);
}

float CELocalPlayerController::interpolateHeight(float x, float z) {
  int x0 = static_cast<int>(floor(x));
  int z0 = static_cast<int>(floor(z));
  int x1 = x0 + 1;
  int z1 = z0 + 1;

  float Q11 = m_map->getHeightAt((m_map->getWidth() * z0) + x0);
  float Q21 = m_map->getHeightAt((m_map->getWidth() * z0) + x1);
  float Q12 = m_map->getHeightAt((m_map->getWidth() * z1) + x0);
  float Q22 = m_map->getHeightAt((m_map->getWidth() * z1) + x1);

  float xRatio = x - x0;
  float zRatio = z - z0;

  float R1 = Q11 * (1 - xRatio) + Q21 * xRatio;
  float R2 = Q12 * (1 - xRatio) + Q22 * xRatio;

  float interpolatedHeight = R1 * (1 - zRatio) + R2 * zRatio;

  return interpolatedHeight;
}

float CELocalPlayerController::computeSlope(float x, float z) {
    int x0 = static_cast<int>(floor(x));
    int z0 = static_cast<int>(floor(z));
    int x1 = x0 + 1;
    int z1 = z0 + 1;

    float Q11 = m_map->getHeightAt((m_map->getWidth() * z0) + x0);
    float Q21 = m_map->getHeightAt((m_map->getWidth() * z0) + x1);
    float Q12 = m_map->getHeightAt((m_map->getWidth() * z1) + x0);
    float Q22 = m_map->getHeightAt((m_map->getWidth() * z1) + x1);

    // Calculate the gradient in the x and z directions
    float dx = ((Q21 - Q11) + (Q22 - Q12)) / (2.0f * m_tile_size);
    float dz = ((Q12 - Q11) + (Q22 - Q21)) / (2.0f * m_tile_size);

    // Calculate the magnitude of the gradient (slope)
    float gradientMagnitude = sqrt(dx * dx + dz * dz);

    // Calculate the slope angle in degrees
    float slopeAngle = atan(gradientMagnitude) * (180.0f / 3.14159f);

    return slopeAngle;
}

void CELocalPlayerController::move(double currentTime, double deltaTime, bool forwardPressed, bool backwardPressed, bool rightPressed, bool leftPressed) {
  float dTime = static_cast<float>(deltaTime);
  glm::vec3 movement(0.0f);

  // Calculate movement vectors based on input
  if (forwardPressed) {
      movement += m_camera.GetForward();
      m_target_speed = m_walk_speed;
  }
  if (backwardPressed) {
      movement -= m_camera.GetForward();
      m_target_speed = m_walk_speed * 0.5f; // Adjust as needed
  }
  if (rightPressed) {
      movement += glm::normalize(glm::cross(m_camera.GetForward(), glm::vec3(0.0f, 1.0f, 0.0f)));
      m_target_speed = m_walk_speed * 0.85f; // Adjust as needed
  }
  if (leftPressed) {
      movement -= glm::normalize(glm::cross(m_camera.GetForward(), glm::vec3(0.0f, 1.0f, 0.0f)));
      m_target_speed = m_walk_speed * 0.85f; // Adjust as needed
  }

  // Normalize the movement vector if diagonal movement is happening
  if (glm::length(movement) > 0.0f) {
      movement = glm::normalize(movement);
  }

  // Apply acceleration and deceleration
  if (m_target_speed > m_current_speed) {
      m_current_speed += m_acceleration * dTime;
      if (m_current_speed > m_target_speed) {
          m_current_speed = m_target_speed;
      }
  } else if (m_target_speed < m_current_speed) {
      m_current_speed -= m_deceleration * dTime;
      if (m_current_speed < m_target_speed) {
          m_current_speed = m_target_speed;
      }
  }

  // Calculate the new position
  auto currentPos = m_camera.GetPosition();
  glm::vec2 currentWorldPos = glm::vec2(int(floorf(currentPos.x / m_tile_size)), int(floorf(currentPos.z / m_tile_size)));

  glm::vec3 pos = m_camera.GetPosition() + movement * (m_current_speed * dTime);
  glm::vec2 worldPos = glm::vec2(int(floorf(pos.x / m_tile_size)), int(floorf(pos.z / m_tile_size)));
  bool outOfBounds = worldPos.x < 0 || worldPos.x > m_map->getWidth() || worldPos.y < 0 || worldPos.y > m_map->getHeight();
  
  // If we are currently out of bounds then do something about it
  if (outOfBounds) {
    pos = m_map->getRandomLanding();
    setPosition(pos);
    currentPos = pos;
    worldPos = glm::vec2(int(floorf(pos.x / m_tile_size)), int(floorf(pos.z / m_tile_size)));
    currentWorldPos = glm::vec2(int(floorf(currentPos.x / m_tile_size)), int(floorf(currentPos.z / m_tile_size)));
  }
  
  float currentWorldHeight = m_map->getPlaceGroundHeight(currentWorldPos.x, currentWorldPos.y);// interpolateHeight(currentWorldPos.x, currentWorldPos.y);
  float nextWorldHeight = m_map->getPlaceGroundHeight(worldPos.x, worldPos.y);
  
  // Apply head bobble effect
  float bobbleOffset = 0.f;
  if (m_current_speed > 0.0f) {
      bobbleOffset = m_bobble_amount * sin(m_bobble_speed * m_bobble_time);
  }
  
  // Apply low-pass filter for smoother height transitions
    float smoothedHeight = (nextWorldHeight * 0.1f) + (m_previousHeight * 0.9f);
    m_previousHeight = smoothedHeight;
  
  // Check if movement is allowed or out of bounds
  float slopeAngle = computeSlope(worldPos.x, worldPos.y);
  float maxClimbHeight = m_player_height / 4.0f;
  
  if (m_is_jumping) {
    maxClimbHeight = m_player_height * 2.f;
  }

  if ((slopeAngle <= maxSlopeAngle ||
      nextWorldHeight < currentWorldHeight ||
      abs(nextWorldHeight - currentWorldHeight) < maxClimbHeight) && !outOfBounds) {
    
    if (!m_is_jumping) {
      pos.y = smoothedHeight + m_player_height + bobbleOffset;
    }

      this->m_camera.SetPos(pos);
  } else {
      currentPos.y = currentWorldHeight + m_player_height + bobbleOffset;

      this->m_camera.SetPos(currentPos);
  }

  // Reset target speed if no movement keys are pressed
  if (!forwardPressed && !backwardPressed && !rightPressed && !leftPressed) {
      m_target_speed = 0.0f;
  }
  
  // Update bobble time if the player is moving
  if (forwardPressed || backwardPressed || rightPressed || leftPressed) {
      m_bobble_time += 100.f * dTime;
  } else {
      m_bobble_time = 0.0f;
  }
  
  // Handle jumping
  if (m_is_jumping) {
      m_vertical_speed -= m_gravity * dTime;
      pos = m_camera.GetPosition();
      pos.y += (m_vertical_speed * m_tile_size) * dTime;

      // Check for landing
    worldPos = glm::vec2(int(floorf(pos.x / m_tile_size)), int(floorf(pos.z / m_tile_size)));
      float groundHeight = interpolateHeight(worldPos.x, worldPos.y) + m_player_height;
      if (pos.y <= groundHeight) {
          pos.y = groundHeight;
          m_is_jumping = false;
          m_vertical_speed = 0.0f;
          m_last_jump_time = currentTime;
      }
      m_camera.SetPos(pos);
  }
}

glm::vec3 CELocalPlayerController::computeSlidingDirection(float x, float z) {
    int x0 = static_cast<int>(floor(x));
    int z0 = static_cast<int>(floor(z));
    int x1 = x0 + 1;
    int z1 = z0 + 1;

    float Q11 = m_map->getHeightAt((m_map->getWidth() * z0) + x0);
    float Q21 = m_map->getHeightAt((m_map->getWidth() * z0) + x1);
    float Q12 = m_map->getHeightAt((m_map->getWidth() * z1) + x0);
    float Q22 = m_map->getHeightAt((m_map->getWidth() * z1) + x1);

    // Calculate the gradient in the x and z directions
    float dx = ((Q21 - Q11) + (Q22 - Q12)) / (2.0f * m_tile_size);
    float dz = ((Q12 - Q11) + (Q22 - Q21)) / (2.0f * m_tile_size);

    glm::vec3 slidingDirection(-dx, 0.0f, -dz);
    return glm::normalize(slidingDirection);
}

void CELocalPlayerController::jump(double currentTime) {
  if (!m_is_jumping && (currentTime - m_last_jump_time) >= m_jump_cooldown) {
      m_is_jumping = true;
      m_vertical_speed = m_jump_speed;
  }
}
