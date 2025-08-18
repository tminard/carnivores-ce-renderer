#include "CELocalPlayerController.hpp"
#include "CEBulletPlayerController.h"
#include "CEPhysicsWorld.h"
#include "camera.h"
#include "C2MapFile.h"
#include "C2MapRscFile.h"
#include "CEWorldModel.h"

#include <iostream>
#include <string>
#include <cmath>

CELocalPlayerController::CELocalPlayerController(float world_width, float world_height, float tile_size, std::shared_ptr<C2MapFile> map, std::shared_ptr<C2MapRscFile> rsc) : CEBasePlayerController(map, rsc), m_world_width(world_width), m_world_height(world_height), m_tile_size(tile_size) {
  
  m_walk_speed = 60.0f;  // 5.1 m/s running speed - only speed we use (scaled to 1 unit = 8.5cm)
  m_player_height = 12.0f;  // Lower eye level height for better camera feel

  m_current_speed = 0.0f;
  m_target_speed = 0.0f;

  // Adjusted acceleration and deceleration values
  m_acceleration = m_walk_speed * 4.0f * 1.25f;  // Reduce to 1.0x walk speed for smoother acceleration
  m_deceleration = m_walk_speed * 1.5f * 1.25f;  // Increase to 0.5x walk speed for less abrupt deceleration
  
  m_bobble_speed = 0.1f * 1.25f;
  m_bobble_amount = m_player_height * 0.1f;
  m_bobble_time = 0.f;
  
  // Initialize camera smoothing to safe values
  m_smoothedCameraPosition = glm::vec3(0.0f, 0.0f, 0.0f);
  m_targetCameraPosition = glm::vec3(0.0f, 0.0f, 0.0f);
  
  // Configure sensory capabilities for local player (god view enabled)
  setSensoryCapabilities(1000.0f, 1000.0f, 1000.0f, true);
}

CELocalPlayerController::~CELocalPlayerController() = default;

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
  
  // Sync physics body position when physics is enabled
  if (m_usePhysics && m_physicsController) {
    m_physicsController->setPosition(position);
  }
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
    float radius = 0.625f; // Scaled down 16x (was 10.0f)
    glm::vec3 bodyPosition = getPosition();
    float x = bodyPosition.x + radius * cos(angle);
    float z = bodyPosition.z + radius * sin(angle);
    float y = bodyPosition.y + 0.3125f; // Scaled down 16x (was 5.0f)

    glm::vec3 cameraPosition = glm::vec3(x, y, z);
    setPosition(cameraPosition);
  
    glm::vec3 direction = glm::normalize(m_body_at - cameraPosition);
    m_camera.SetLookAt(direction);
}

void CELocalPlayerController::move(double currentTime, double deltaTime, bool forwardPressed, bool backwardPressed, bool rightPressed, bool leftPressed) {
  move(currentTime, deltaTime, forwardPressed, backwardPressed, rightPressed, leftPressed, false);
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
    // Physics system handles jumping directly through the move() method with jump input
    if (!m_usePhysics || !m_physicsController) {
        std::cerr << "Warning: Physics-based movement not enabled. Use move() with jumpPressed parameter instead." << std::endl;
        return;
    }
    
    // Jump input should be handled via the move() method with jumpPressed parameter
    // This method is kept for backward compatibility but doesn't do anything in physics mode
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

void CELocalPlayerController::enablePhysics(CEPhysicsWorld* physicsWorld)
{
  if (!physicsWorld) return;
  
  // Create physics controller at current position
  glm::vec3 currentPos = getPosition();
  m_physicsController = std::make_unique<CEBulletPlayerController>(physicsWorld, currentPos);
  
  // Configure physics controller to match current settings
  m_physicsController->setWalkSpeed(m_walk_speed);
  m_physicsController->setJumpForce(m_jump_speed * 80.0f); // Convert to physics impulse
  m_physicsController->setFlying(m_flyingMode);
  
  m_usePhysics = true;
}

void CELocalPlayerController::disablePhysics()
{
  if (m_physicsController) {
    // Store current physics position before disabling
    glm::vec3 physicsPos = m_physicsController->getPosition();
    setPosition(physicsPos);
    
    m_physicsController.reset();
  }
  m_usePhysics = false;
}

void CELocalPlayerController::setFlyingMode(bool flying)
{
  m_flyingMode = flying;
  
  if (m_physicsController) {
    m_physicsController->setFlying(flying);
  }
}

void CELocalPlayerController::move(double currentTime, double deltaTime, bool forwardPressed, bool backwardPressed, bool rightPressed, bool leftPressed, bool jumpPressed) {
  if (m_dead) return;
  
  // Physics-based movement is now the primary system
  if (!m_usePhysics || !m_physicsController) {
    // Physics not enabled - fall back to safe camera position
    std::cerr << "Warning: Physics-based movement not enabled. Call enablePhysics() first." << std::endl;
    
    // Set a safe default camera position if not initialized
    if (glm::length(m_smoothedCameraPosition) < 0.1f) {
      m_smoothedCameraPosition = m_camera.GetPosition();
    }
    return;
  }
  
  // Pass input directly - physics controller will handle camera transformation
  CEBulletPlayerController::MovementInput input;
  input.forward = forwardPressed;
  input.backward = backwardPressed;
  input.left = leftPressed;
  input.right = rightPressed;
  input.jump = jumpPressed;
  input.flying = m_flyingMode;
  
  // Update physics controller with camera forward direction
  m_physicsController->update(input, currentTime, static_cast<float>(deltaTime), m_camera.GetForward());
  
  // Get physics position and calculate movement
  glm::vec3 physicsPos = m_physicsController->getPosition();
  auto movementState = m_physicsController->getMovementState();
  
  // Calculate camera bobble based on movement speed and ground contact
  glm::vec3 bobbleOffset = glm::vec3(0);
  if (movementState.isGrounded && movementState.speed > 1.0f && !m_flyingMode) {
    // Calculate bobble intensity based on movement speed
    float speedRatio = glm::clamp(movementState.speed / m_walk_speed, 0.0f, 1.0f);
    m_currentBobbleIntensity = glm::mix(m_currentBobbleIntensity, speedRatio, static_cast<float>(deltaTime) * 8.0f);
    
    // Update bobble time based on movement speed
    m_physicsBobbleTime += static_cast<float>(deltaTime) * movementState.speed * 0.02f;
    
    // Calculate natural bobble using sine waves for realistic walking motion
    float bobbleFreq = 12.0f; // Steps per second feel
    float verticalBob = std::sin(m_physicsBobbleTime * bobbleFreq) * m_currentBobbleIntensity * 1.6f; // Doubled for more noticeable effect
    float horizontalBob = std::sin(m_physicsBobbleTime * bobbleFreq * 0.5f) * m_currentBobbleIntensity * 0.8f; // Doubled for more noticeable effect
    
    bobbleOffset = glm::vec3(horizontalBob, verticalBob, 0);
  } else {
    // Smooth out bobble when not moving or in air
    m_currentBobbleIntensity = glm::mix(m_currentBobbleIntensity, 0.0f, static_cast<float>(deltaTime) * 5.0f);
  }
  
  // Apply camera position with proper eye height and bobble
  glm::vec3 eyeHeight = glm::vec3(0, m_player_height, 0); // m_player_height is already eye level
  m_targetCameraPosition = physicsPos + eyeHeight + bobbleOffset;
  
  // Smooth camera position to reduce terrain jitter while keeping physics accurate
  // Check if this is the first frame or position is invalid
  if (glm::length(m_smoothedCameraPosition) < 0.1f || 
      std::isnan(m_smoothedCameraPosition.x) || std::isnan(m_smoothedCameraPosition.y) || std::isnan(m_smoothedCameraPosition.z) ||
      std::abs(m_smoothedCameraPosition.x) > 10000.0f || std::abs(m_smoothedCameraPosition.y) > 10000.0f || std::abs(m_smoothedCameraPosition.z) > 10000.0f) {
    // Initialize or reset to safe position
    m_smoothedCameraPosition = m_targetCameraPosition;
  } else {
    // Smooth the camera position over time
    m_smoothedCameraPosition = glm::mix(m_smoothedCameraPosition, m_targetCameraPosition, 
                                       static_cast<float>(deltaTime) * (1.0f / m_cameraSmoothing));
  }
  
  m_camera.SetPos(m_smoothedCameraPosition);
}

