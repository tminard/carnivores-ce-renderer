#include "CELocalPlayerController.hpp"
#include "camera.h"
#include "C2MapFile.h"
#include "C2MapRscFile.h"
#include "CEWorldModel.h"

#include <iostream>
#include <string>

CELocalPlayerController::CELocalPlayerController(float world_width, float world_height, float tile_size, std::shared_ptr<C2MapFile> map, std::shared_ptr<C2MapRscFile> rsc) : CEBasePlayerController(map, rsc), m_world_width(world_width), m_world_height(world_height), m_tile_size(tile_size) {
  
  m_walk_speed = m_tile_size * 2.8f;
  m_player_height = m_tile_size * 1.25f;

  m_current_speed = 0.0f;
  m_target_speed = 0.0f;

  // Adjusted acceleration and deceleration values
  m_acceleration = m_walk_speed * 4.0f * 1.25f;  // Reduce to 1.0x walk speed for smoother acceleration
  m_deceleration = m_walk_speed * 1.5f * 1.25f;  // Increase to 0.5x walk speed for less abrupt deceleration
  
  m_bobble_speed = 0.1f * 1.25f;
  m_bobble_amount = m_player_height * 0.1f;
  m_bobble_time = 0.f;
  
  // Configure sensory capabilities for local player (god view enabled)
  setSensoryCapabilities(1000.0f, 1000.0f, 1000.0f, true);
}

void CELocalPlayerController::lookAt(glm::vec3 direction)
{
  if (m_dead) return;
  this->m_camera.SetLookAt(direction);
}

void CELocalPlayerController::update(double currentTime, double deltaTime)
{
  if (m_dead) {
    if (currentTime - m_died_at > 8.0) {
      m_dead = false;
      setPosition(m_map->getRandomLanding());
      glm::vec3 direction = glm::normalize(m_body_at - m_camera.GetPosition());
      m_camera.SetLookAt(direction);
    } else {
      panAroundBody(currentTime);
      return;
    }
  }

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

void CELocalPlayerController::panAroundBody(double currentTime) {
    float angle = (currentTime - m_died_at) * 0.5f;
    float radius = 10.0f;
    glm::vec3 bodyPosition = getPosition();
    float x = bodyPosition.x + radius * cos(angle);
    float z = bodyPosition.z + radius * sin(angle);
    float y = bodyPosition.y + 5.0f;

    glm::vec3 cameraPosition = glm::vec3(x, y, z);
    setPosition(cameraPosition);
  
    glm::vec3 direction = glm::normalize(m_body_at - cameraPosition);
    m_camera.SetLookAt(direction);
}

void CELocalPlayerController::move(double currentTime, double deltaTime, bool forwardPressed, bool backwardPressed, bool rightPressed, bool leftPressed) {
  if (m_dead) return;

  float dTime = static_cast<float>(deltaTime);
  glm::vec3 movement(0.0f);

  // Calculate movement vectors based on input
  if (forwardPressed) {
      movement += m_camera.GetForward();
      m_target_speed = m_walk_speed;
  }
  if (backwardPressed) {
      movement -= m_camera.GetForward();
      m_target_speed = m_walk_speed * 0.5f;
  }
  if (rightPressed) {
      movement += glm::normalize(glm::cross(m_camera.GetForward(), glm::vec3(0.0f, 1.0f, 0.0f)));
      m_target_speed = m_walk_speed * 0.85f;
  }
  if (leftPressed) {
      movement -= glm::normalize(glm::cross(m_camera.GetForward(), glm::vec3(0.0f, 1.0f, 0.0f)));
      m_target_speed = m_walk_speed * 0.85f;
  }

  // Normalize the movement vector if diagonal movement is happening
  if (glm::length(movement) > 0.0f) {
      movement = glm::normalize(movement);
  }
  
  // Reset target speed if no movement keys are pressed
  if (!forwardPressed && !backwardPressed && !rightPressed && !leftPressed) {
      m_target_speed = 0.0f;
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
  bool outOfBounds = worldPos.x < 0 || worldPos.x > m_map->getHeight() || worldPos.y < 0 || worldPos.y > m_map->getWidth();

  // If we are currently out of bounds then do something about it
  if (outOfBounds) {
    pos = m_map->getRandomLanding();
    setPosition(pos);
    currentPos = pos;
    worldPos = glm::vec2(int(floorf(pos.x / m_tile_size)), int(floorf(pos.z / m_tile_size)));
    currentWorldPos = glm::vec2(int(floorf(currentPos.x / m_tile_size)), int(floorf(currentPos.z / m_tile_size)));
  }
  
  // Use enhanced interpolated height sampling with predictive sampling
  glm::vec3 currentWorldPosVec(currentPos.x, currentPos.y, currentPos.z);
  glm::vec3 nextWorldPosVec(pos.x, pos.y, pos.z);
  
  float currentWorldHeight = m_map->getHeightAtWorldPosition(currentWorldPosVec);
  float nextWorldHeight = m_map->getHeightAtWorldPosition(nextWorldPosVec);
  
  // Use predictive height sampling for smoother movement over varying terrain
  float predictiveHeight = nextWorldHeight;
  if (glm::length(movement) > 0.0f) {
      predictiveHeight = getPredictiveHeight(nextWorldPosVec, movement, m_current_speed, dTime);
  }
  
  // Apply head bobble effect
  float bobbleOffset = 0.f;
  if (m_current_speed > 0.0f) {
      bobbleOffset = m_bobble_amount * sin(m_bobble_speed * m_bobble_time);
  }
  
  // Calculate slope angle once for both smoothing and movement validation
  float slopeAngle = computeSlope(worldPos.x, worldPos.y);
  
  // Apply adaptive low-pass filter for smoother height transitions
    float targetHeight = predictiveHeight;
    float heightDifference = targetHeight - m_previousHeight;
    
    // Calculate adaptive smoothing factor based on terrain conditions and movement
    float adaptiveFactor = getAdaptiveSmoothingFactor(heightDifference, m_current_speed, slopeAngle);
    
    float smoothedHeight = (targetHeight * adaptiveFactor) + (m_previousHeight * (1.0f - adaptiveFactor));
    m_previousHeight = smoothedHeight;
  
  // Check if movement is allowed or out of bounds
  float maxClimbHeight = m_player_height / 4.0f;
  
  if (m_is_jumping) {
    maxClimbHeight = m_player_height * 2.f;
  }

  if ((slopeAngle <= maxSlopeAngle ||
      predictiveHeight < currentWorldHeight ||
      abs(predictiveHeight - currentWorldHeight) < maxClimbHeight) && !outOfBounds) {
    
    if (!m_is_jumping) {
      pos.y = smoothedHeight + m_player_height + bobbleOffset;
    }

      this->m_camera.SetPos(pos);
  } else {
      currentPos.y = currentWorldHeight + m_player_height + bobbleOffset;

      this->m_camera.SetPos(currentPos);
  }
  
  // Update bobble time if the player is moving
  if (forwardPressed || backwardPressed || rightPressed || leftPressed || m_current_speed > 0.0) {
      m_bobble_time += 100.f * dTime;
  } else {
      m_bobble_time = 0.0f;
  }
  
  // Handle jumping
  if (m_is_jumping) {
      m_vertical_speed -= m_gravity * dTime;
      pos = m_camera.GetPosition();
      pos.y += (m_vertical_speed * m_tile_size) * dTime;

      // Check for landing using interpolated height
      glm::vec3 jumpWorldPos(pos.x, pos.y, pos.z);
      float groundHeight = m_map->getHeightAtWorldPosition(jumpWorldPos) + m_player_height;
      if (pos.y <= groundHeight - 24.f) {
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

float CELocalPlayerController::getPredictiveHeight(const glm::vec3& currentPos, const glm::vec3& movementDir, float speed, float deltaTime) {
    // Calculate predictive position ahead of movement
    float predictiveDistanceWorld = m_predictiveDistance * m_tile_size;
    glm::vec3 predictivePos = currentPos + (movementDir * predictiveDistanceWorld);
    
    // Sample multiple points ahead for smoother prediction
    float currentHeight = m_map->getHeightAtWorldPosition(currentPos);
    float nearHeight = m_map->getHeightAtWorldPosition(currentPos + (movementDir * predictiveDistanceWorld * 0.5f));
    float farHeight = m_map->getHeightAtWorldPosition(predictivePos);
    
    // Weighted average based on movement speed and direction
    float speedFactor = std::min(speed / m_walk_speed, 1.0f);
    
    // Blend heights: more weight on near prediction when moving slowly, far prediction when moving fast
    float blendedHeight = currentHeight * (1.0f - speedFactor * 0.4f) + 
                         nearHeight * (speedFactor * 0.3f) + 
                         farHeight * (speedFactor * 0.1f);
    
    return blendedHeight;
}

float CELocalPlayerController::getAdaptiveSmoothingFactor(float heightDifference, float speed, float slopeAngle) {
    // Base smoothing factor
    float smoothingFactor = m_baseSmoothingFactor;
    
    // Adjust based on height difference - larger changes need more smoothing
    float heightFactor = std::min(std::abs(heightDifference) / (m_player_height * 2.0f), 1.0f);
    
    // Adjust based on movement speed - faster movement needs less smoothing for responsiveness
    float speedFactor = std::min(speed / m_walk_speed, 1.0f);
    
    // Adjust based on slope - steeper terrain needs more smoothing
    float slopeFactor = std::min(slopeAngle / maxSlopeAngle, 1.0f);
    
    // Combine factors: more smoothing for large height changes and steep slopes,
    // less smoothing for fast movement
    smoothingFactor = m_baseSmoothingFactor + 
                      (heightFactor * 0.1f) +     // Increase smoothing for large height changes
                      (slopeFactor * 0.05f) -     // Increase smoothing for steep slopes  
                      (speedFactor * 0.05f);      // Decrease smoothing for fast movement
    
    // Clamp to reasonable bounds
    return std::max(m_minSmoothingFactor, std::min(m_maxSmoothingFactor, smoothingFactor));
}

void CELocalPlayerController::jump(double currentTime) {
    if (!m_is_jumping && (currentTime - m_last_jump_time) >= m_jump_cooldown) {
        m_is_jumping = true;

        // Influence of forward speed on jump
        glm::vec3 forward = m_camera.GetForward();
        float forwardSpeedFactor = glm::dot(forward, glm::normalize(forward));
        
        // Calculate horizontal speed contribution to the jump
        float horizontalSpeedContribution = glm::length(forward) * forwardSpeedFactor;
        
        // Reduce the vertical speed slightly if moving forward
        float adjustedVerticalSpeed = m_jump_speed - (horizontalSpeedContribution * 0.2f);

        // Set the vertical speed with the reduced factor
        m_vertical_speed = adjustedVerticalSpeed;
        
        // Add a small forward boost if the player is moving forward
        if (horizontalSpeedContribution > 0) {
            m_camera.SetPos(m_camera.GetPosition() + forward * horizontalSpeedContribution * 0.5f);
        }
    }
}

bool CELocalPlayerController::isAlive(double currentTime)
{
  return (currentTime - m_died_at > 10.0);
}

void CELocalPlayerController::kill(double killedAt) { 
  m_died_at = killedAt;
  m_body_at = getPosition();
  m_dead = true;
  m_target_speed = 0.0;
  m_current_speed = 0.0;
}

