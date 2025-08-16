//
//  CERemotePlayerController.cpp
//  CarnivoresRenderer
//
//  Created by Tyler Minard on 7/30/24.
//

#include "CERemotePlayerController.hpp"

#include <iostream>
#include <algorithm>

#include "C2CarFile.h"
#include "C2MapRscFile.h"
#include "C2MapFile.h"
#include "camera.h"
#include "transform.h"

#include "IndexedMeshLoader.h"
#include "CEGeometry.h"
#include "CEAnimation.h"
#include "LocalAudioManager.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/constants.hpp>

CERemotePlayerController::CERemotePlayerController(std::shared_ptr<LocalAudioManager> audioManager, std::shared_ptr<C2CarFile> carFile, std::shared_ptr<C2MapFile> map, std::shared_ptr<C2MapRscFile> rsc, std::string initialAnimationName): CEBasePlayerController(map, rsc), m_car(carFile), m_current_animation(initialAnimationName), m_g_audio_manager(audioManager)
{
  m_current_speed = 0.f;
  m_target_speed = 0.f;
  m_animation_started_at = 0.0;
  m_animation_last_update_at = 0.0;
  m_start_turn_time = -1.0;
  
  // TODO: make these configurable
  // Perhaps an attachable physics controller or something
  m_walk_speed = map->getTileLength() * 0.5f;
  m_player_height = 0.f;
  
  // Adjusted acceleration and deceleration values
  m_acceleration = m_walk_speed * 4.0f * 1.25f;  // Reduce to 1.0x walk speed for smoother acceleration
  m_deceleration = m_walk_speed * 1.5f * 1.25f;  // Increase to 0.5x walk speed for less abrupt deceleration

  // Get an exclusive copy of the geometry by rebuilding from the first animation
  std::shared_ptr<CEAnimation> initialAni = carFile->getAnimationByName(m_current_animation).lock();
  if (!initialAni) {
    std::cerr << "Error: car file missing default animation: " << m_current_animation << std::endl << "Falling back to original" << std::endl;
    
    // TODO: We can't use this without animations, right?
    // Consider a different class to spawn non-interactive CAR models
    initialAni = carFile->getFirstAnimation().lock();
    if (!initialAni) {
      throw std::runtime_error("Error: car file missing requested animation and no fallback was available. Consider static class if this is expected.");
    }
  }
  
  std::unique_ptr<IndexedMeshLoader> m_loader = std::make_unique<IndexedMeshLoader>(*initialAni->GetOriginalVertices(), *initialAni->GetFaces());
  
  // Note we create a copy of geometry from the FIRST animation
  // Note we get a lock on the texture, thus there's a dependency here on
  // the original geometry.
  m_geo = std::make_unique<CEGeometry>(m_loader->getVertices(), m_loader->getIndices(), carFile->getGeometry()->getTexture().lock(), "dinosaur");
  
  Transform transform(glm::vec3(0.f), glm::vec3(0, glm::radians(180.f), 0), glm::vec3(1.f));
  std::vector<glm::mat4> model = { transform.GetStaticModel() };
  
  // Only ever ONE instance!
  m_geo->UpdateInstances(model);
  m_geo->ConfigureShaderUniforms(m_map.get(), m_rsc.get());
  
  // Configure sensory capabilities for AI (limited view, no god mode)
  setSensoryCapabilities(60.0f, 80.0f, 20.0f, false);
  
  // Initialize camera look-at direction to match the model's initial facing direction (180 degree Y rotation)
  // The model transform has a 180-degree Y rotation, so it faces negative Z initially
  // Pass the direction vector directly, not a world position
  glm::vec3 initialFacingDirection = glm::vec3(0.0f, 0.0f, -1.0f);
  m_camera.SetLookAt(initialFacingDirection);
}

glm::vec3 CERemotePlayerController::getPosition() const
{
  return this->m_camera.GetPosition();
}

void CERemotePlayerController::setWalkSpeed(float speedFactor)
{
  m_walk_speed = m_map->getTileLength() * speedFactor;
}

glm::vec2 CERemotePlayerController::getWorldPosition() const
{
  glm::vec3 pos = m_camera.GetPosition();
  
  return glm::vec2(int(floorf(pos.x / m_map->getTileLength())), int(floorf(pos.z / m_map->getTileLength())));
}

void CERemotePlayerController::setNextAnimation(std::string animationName, bool loop)
{
  // TODO: track next animation so we can animation between them cleanly
  m_current_animation = animationName;
  m_animation_started_at = glfwGetTime();
  m_is_looping_anim = loop;
}

bool CERemotePlayerController::isAnimPlaying(double currentTime) {
  if (m_current_animation == "") return false;
  if (m_is_looping_anim) return true;
  
  auto ani = m_car->getAnimationByName(m_current_animation).lock();
  
  return ((currentTime - m_animation_started_at) * 1000.0) < ani->m_total_time;
}

void CERemotePlayerController::update(double currentTime, double deltaTime)
{
  // Base implementation - can be extended for AI-specific updates
  // This method is called by the base controller system
}

void CERemotePlayerController::updateWithObserver(double currentTime, Transform &baseTransform, Camera &observerCamera, glm::vec2 observerWorldPosition)
{
  auto worldPos = getWorldPosition();
  auto dist = glm::distance(worldPos, observerWorldPosition);
  auto anim = m_car->getAnimationByName(m_current_animation);
  
  // Slower updates above 80 tiles
  bool deferUpdate = dist > 80.f;
  // Max the FPS at this range
  bool maxFPS = dist < 20.f;
  // Do not even animate at this range
  bool notVisible = dist > 128.f;
  
  // Smooth terrain pitch angle over time for natural tilting
  static double lastUpdateTime = currentTime;
  float deltaTime = static_cast<float>(currentTime - lastUpdateTime);
  lastUpdateTime = currentTime;
  
  float pitchDifference = m_targetPitchAngle - m_currentPitchAngle;
  float pitchSmoothingRate = m_pitchSmoothingSpeed * deltaTime;
  m_currentPitchAngle += pitchDifference * pitchSmoothingRate;
  
  m_geo->Update(baseTransform, observerCamera);
  
  // Calculate dynamic animation speed based on current movement speed and animation type
  float animationPlaybackSpeed = calculateAnimationPlaybackSpeed();
  
  bool didUpdate = m_geo->SetAnimation(anim, currentTime, m_animation_started_at, m_animation_last_update_at, deferUpdate, maxFPS, notVisible, animationPlaybackSpeed, m_is_looping_anim);
  
  if (didUpdate) {
    auto audioSrc = m_car->getSoundForAnimation(m_current_animation);
    if (audioSrc != nullptr && m_geo->GetCurrentFrame() == 0 && !audioSrc->isPlaying()) {
      audioSrc->setPosition(m_camera.GetPosition());
      audioSrc->setLooped(false);
      audioSrc->setMaxDistance(256*60);
      audioSrc->setGain(2.f);
      audioSrc->setClampDistance(256*6);
      m_g_audio_manager->play(audioSrc);
    }
    m_animation_last_update_at = currentTime;
  }
}

void CERemotePlayerController::Render()
{
  m_geo->DrawInstances();
}

void CERemotePlayerController::StopMovement()
{
  m_current_speed = 0.0;
  m_target_speed = 0.0;
}

void CERemotePlayerController::jump(double currentTime)
{
  // TODO: add jump
}

void CERemotePlayerController::uploadStateToHardware()
{
  auto position = m_camera.GetPosition();
  // GetLookAt() now returns the direction vector directly (not a world position)
  glm::vec3 direction = m_camera.GetLookAt();
  
  // Calculate basic yaw from camera direction (for facing)
  float yaw = atan2(direction.x, direction.z) + glm::radians(180.f);
  
  // Calculate terrain normal at current position for terrain alignment
  glm::vec3 terrainNormal = calculateTerrainNormal(position);
  
  // Calculate roll based on terrain slope in the character's right direction
  glm::vec3 rightDirection = glm::vec3(cos(yaw - glm::radians(90.f)), 0, sin(yaw - glm::radians(90.f)));
  float roll = calculateTerrainRoll(position, rightDirection);
  
  // Calculate pitch based on terrain slope in the character's forward direction  
  glm::vec3 forwardDirection = glm::vec3(sin(yaw), 0, cos(yaw));
  float pitch = calculateTerrainPitch(position, forwardDirection);
  
  // Apply terrain alignment angles (fix both pitch and roll inversion)
  glm::vec3 rotation(-pitch, yaw, -roll); // Negate both pitch and roll to fix inversions
  Transform transform(position, rotation, glm::vec3(1.f));
  
  // Update instance data
  std::vector<glm::mat4> model = { transform.GetStaticModel() };
  m_geo->UpdateInstances(model);
}


void CERemotePlayerController::setPosition(glm::vec3 position)
{
  m_camera.SetPos(position);
}

void CERemotePlayerController::setElevation(float elevation)
{
  this->m_camera.SetHeight(elevation);
}

void CERemotePlayerController::lookAt(glm::vec3 direction)
{
  this->m_camera.SetLookAt(direction);
}

bool CERemotePlayerController::isTurning() {
  return m_start_turn_time > 0.0;
}

float FindVectorAlpha(glm::vec3 vec) {
  float vx = vec.x;
  float vy = vec.z;
    float adx = glm::abs(vx);
    float ady = glm::abs(vy);

    float alpha = glm::pi<float>() / 4.f;
    float dalpha = glm::pi<float>() / 8.f;

    for (int i = 1; i <= 10; i++) {
        glm::vec2 vec(adx, ady);
        glm::vec2 rotVec = glm::vec2(glm::cos(alpha), glm::sin(alpha));

        // Sign of the dot product
        float dotProduct = glm::dot(vec, rotVec);
        alpha = alpha - dalpha * glm::sign(dotProduct);
        dalpha /= 2.f;
    }

    if (vx < 0) {
        if (vy < 0) {
            alpha += glm::pi<float>();
        } else {
            alpha = glm::pi<float>() - alpha;
        }
    } else if (vy < 0) {
        alpha = 2.f * glm::pi<float>() - alpha;
    }

    return alpha;
}

float AngleDifference(float a, float b) {
    a -= b;
    a = glm::abs(a);
    if (a > glm::pi<float>()) {
        a = 2.0f * glm::pi<float>() - a;
    }
    return a;
}

void CERemotePlayerController::UpdateLookAtDirection(glm::vec3 desiredLookAt, double currentTime) {
  // Simplified approach: just set the look-at direction directly like the local player controller
  // The desiredLookAt is actually a world position, so calculate the direction from current position
  glm::vec3 currentPosition = getPosition();
  glm::vec3 direction = glm::normalize(desiredLookAt - currentPosition);
  
  // Set the camera look-at direction directly - pass the DIRECTION VECTOR, not a world position
  // This matches how LocalInputManager calls lookAt(direction)
  lookAt(direction);
}

void CERemotePlayerController::SetFacingDirection(glm::vec3 direction) {
  // Normalize the direction vector
  glm::vec3 normalizedDirection = glm::normalize(direction);
  
  // Set the camera to face this direction immediately - pass the direction vector directly
  lookAt(normalizedDirection);
  
  // Reset any ongoing turn animation
  m_start_turn_time = -1.0;
}


Camera* CERemotePlayerController::getCamera()
{
  return &this->m_camera;
}

const std::string CERemotePlayerController::getCurrentAnimation() const {
  return m_current_animation;
}

void CERemotePlayerController::MoveTo(glm::vec3 targetPosition, double deltaTime) {
  // Get the current position
  glm::vec3 currentPosition = this->getPosition();
  float distance = glm::distance(currentPosition, targetPosition);
  
  // Calculate the time delta
  if (deltaTime <= 0.0) {
    return; // Prevent division by zero or negative deltaTime
  }
  
  // Calculate direction toward the target and update camera to face it
  glm::vec3 targetDirection = glm::normalize(targetPosition - currentPosition);
  glm::vec3 targetLookAt = currentPosition + targetDirection;
  
  // Update camera look-at to face the movement direction with smooth turning
  UpdateLookAtDirection(targetLookAt, glfwGetTime());
  
  // Use the target direction for movement (ensures character moves toward target)
  glm::vec3 direction = targetDirection;
  
  float dTime = static_cast<float>(deltaTime);
  const float tileSize = m_map->getTileLength(); // Size of one tile in the map
  
  // Apply acceleration and deceleration to current speed
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
  
  // Calculate the new position based on the current speed and current lookAt direction
  glm::vec3 newPosition = currentPosition + direction * (m_current_speed * dTime);
  glm::vec2 worldPos = glm::vec2(int(floorf(newPosition.x / tileSize)), int(floorf(newPosition.z / tileSize)));
  
  // Ensure the new position is within the map bounds
  if (worldPos.x < 0 || worldPos.x >= m_map->getWidth() || worldPos.y < 0 || worldPos.y >= m_map->getHeight()) {
    return;
  }
  
  // Enhanced terrain height sampling with footprint-based calculation
  glm::vec3 currentWorldPosVec(currentPosition.x, currentPosition.y, currentPosition.z);
  glm::vec3 nextWorldPosVec(newPosition.x, newPosition.y, newPosition.z);
  
  // Use footprint-based height calculation for better ground contact
  float currentWorldHeight = calculateFootprintHeight(currentWorldPosVec, direction);
  float nextWorldHeight = calculateFootprintHeight(nextWorldPosVec, direction);
  
  // Use predictive height sampling for smoother movement over varying terrain
  float predictiveHeight = nextWorldHeight;
  if (glm::length(direction) > 0.0f) {
      predictiveHeight = getPredictiveHeight(nextWorldPosVec, direction, m_current_speed);
  }
  
  // Calculate slope angle for adaptive smoothing
  float slopeAngle = computeSlope(worldPos.x, worldPos.y);
  
  // Apply adaptive low-pass filter for smoother height transitions
  float targetHeight = predictiveHeight + m_player_height;
  float heightDifference = targetHeight - m_previousHeight;
  
  // Calculate adaptive smoothing factor based on terrain conditions and movement
  float adaptiveFactor = getAdaptiveSmoothingFactor(heightDifference, m_current_speed, slopeAngle);
  
  float smoothedHeight = (targetHeight * adaptiveFactor) + (m_previousHeight * (1.0f - adaptiveFactor));
  m_previousHeight = smoothedHeight;
  
  newPosition.y = smoothedHeight;
  
  // Calculate and smooth terrain pitch for visual angle adjustment
  m_targetPitchAngle = calculateTerrainPitch(newPosition, direction);
  
  // Set the new position without changing the lookAt direction
  setPosition(newPosition);
}

void CERemotePlayerController::setHeightOffset(float offset) { 
  m_player_height = offset;
}

void CERemotePlayerController::setTargetSpeed(float targetSpeed) { 
  m_target_speed = m_map->getTileLength() * targetSpeed;
}

float CERemotePlayerController::computeSlope(float x, float z) {
    int x0 = static_cast<int>(floor(x));
    int z0 = static_cast<int>(floor(z));
    int x1 = x0 + 1;
    int z1 = z0 + 1;

    // Clamp to map bounds
    int width = m_map->getWidth();
    int height = m_map->getHeight();
    x0 = std::max(0, std::min(x0, width - 1));
    x1 = std::max(0, std::min(x1, width - 1));
    z0 = std::max(0, std::min(z0, height - 1));
    z1 = std::max(0, std::min(z1, height - 1));

    float Q11 = m_map->getHeightAt((width * z0) + x0);
    float Q21 = m_map->getHeightAt((width * z0) + x1);
    float Q12 = m_map->getHeightAt((width * z1) + x0);
    float Q22 = m_map->getHeightAt((width * z1) + x1);

    // Calculate the gradient in the x and z directions
    float tileSize = m_map->getTileLength();
    float dx = ((Q21 - Q11) + (Q22 - Q12)) / (2.0f * tileSize);
    float dz = ((Q12 - Q11) + (Q22 - Q21)) / (2.0f * tileSize);

    // Calculate the magnitude of the gradient (slope)
    float gradientMagnitude = sqrt(dx * dx + dz * dz);

    // Calculate the slope angle in degrees
    float slopeAngle = atan(gradientMagnitude) * (180.0f / 3.14159f);

    return slopeAngle;
}

float CERemotePlayerController::getPredictiveHeight(const glm::vec3& currentPos, const glm::vec3& movementDir, float speed) {
    // Calculate predictive position ahead of movement
    float tileSize = m_map->getTileLength();
    float predictiveDistanceWorld = m_predictiveDistance * tileSize;
    glm::vec3 predictivePos = currentPos + (movementDir * predictiveDistanceWorld);
    
    // Sample multiple points ahead for smoother prediction
    float currentHeight = m_map->getHeightAtWorldPosition(currentPos);
    float nearHeight = m_map->getHeightAtWorldPosition(currentPos + (movementDir * predictiveDistanceWorld * 0.5f));
    float farHeight = m_map->getHeightAtWorldPosition(predictivePos);
    
    // Weighted average based on movement speed
    float speedFactor = std::min(speed / m_walk_speed, 1.0f);
    
    // Blend heights: more weight on near prediction when moving slowly, far prediction when moving fast
    float blendedHeight = currentHeight * (1.0f - speedFactor * 0.35f) + 
                         nearHeight * (speedFactor * 0.25f) + 
                         farHeight * (speedFactor * 0.1f);
    
    return blendedHeight;
}

float CERemotePlayerController::getAdaptiveSmoothingFactor(float heightDifference, float speed, float slopeAngle) {
    // Base smoothing factor for AI
    float smoothingFactor = m_baseSmoothingFactor;
    
    // Adjust based on height difference - larger changes need more smoothing
    float tileSize = m_map->getTileLength();
    float heightFactor = std::min(std::abs(heightDifference) / (m_player_height * 2.0f + tileSize * 0.5f), 1.0f);
    
    // Adjust based on movement speed - faster movement needs less smoothing for responsiveness
    float speedFactor = std::min(speed / m_walk_speed, 1.0f);
    
    // Adjust based on slope - steeper terrain needs more smoothing
    float slopeFactor = std::min(slopeAngle / maxSlopeAngle, 1.0f);
    
    // Combine factors: more smoothing for large height changes and steep slopes,
    // less smoothing for fast movement
    smoothingFactor = m_baseSmoothingFactor + 
                      (heightFactor * 0.08f) +     // Increase smoothing for large height changes
                      (slopeFactor * 0.04f) -      // Increase smoothing for steep slopes  
                      (speedFactor * 0.03f);       // Decrease smoothing for fast movement
    
    // Clamp to reasonable bounds
    return std::max(m_minSmoothingFactor, std::min(m_maxSmoothingFactor, smoothingFactor));
}

float CERemotePlayerController::calculateTerrainPitch(const glm::vec3& position, const glm::vec3& forward) {
    // Use more stable sampling with larger offset
    float tileSize = m_map->getTileLength();
    float offset = tileSize * 0.25f; // Match terrain normal sampling
    
    // Sample multiple points for more stable pitch calculation
    glm::vec3 backPosition = position - forward * offset;
    glm::vec3 frontPosition = position + forward * offset;
    
    float backHeight = m_map->getHeightAtWorldPosition(backPosition);
    float frontHeight = m_map->getHeightAtWorldPosition(frontPosition);
    
    // Calculate the pitch angle based on height difference
    float heightDiff = frontHeight - backHeight;
    float distance = 2.0f * offset; // Total distance between samples
    
    // Calculate pitch angle in radians (positive = uphill, negative = downhill)
    float pitchAngle = atan2(heightDiff, distance);
    
    // Clamp to reasonable bounds - creatures can handle steeper forward/backward slopes
    // Max 35 degrees up/down (more tolerant for pitch)
    return std::max(-glm::radians(35.0f), std::min(glm::radians(35.0f), pitchAngle));
}

glm::vec3 CERemotePlayerController::calculateTerrainNormal(const glm::vec3& position) {
    float tileSize = m_map->getTileLength();
    // Use a larger, more stable offset for better normal calculation
    float offset = tileSize * 0.25f; // Increased from 0.1f for more stable gradients
    
    // Method 1: Cross-product method using 4 corner points (more accurate)
    // Sample heights at 4 corners to form two triangles
    glm::vec3 p1 = position + glm::vec3(-offset, 0, -offset); // Bottom-left
    glm::vec3 p2 = position + glm::vec3(offset, 0, -offset);  // Bottom-right
    glm::vec3 p3 = position + glm::vec3(-offset, 0, offset);  // Top-left
    glm::vec3 p4 = position + glm::vec3(offset, 0, offset);   // Top-right
    
    // Get heights at these positions
    p1.y = m_map->getHeightAtWorldPosition(p1);
    p2.y = m_map->getHeightAtWorldPosition(p2);
    p3.y = m_map->getHeightAtWorldPosition(p3);
    p4.y = m_map->getHeightAtWorldPosition(p4);
    
    // Calculate two vectors for cross product
    // Triangle 1: p1 -> p2 -> p3
    glm::vec3 v1 = p2 - p1; // Bottom edge
    glm::vec3 v2 = p3 - p1; // Left edge
    
    // Triangle 2: p2 -> p4 -> p3  
    glm::vec3 v3 = p4 - p2; // Right edge
    glm::vec3 v4 = p3 - p2; // Diagonal
    
    // Calculate normals for both triangles
    glm::vec3 normal1 = glm::normalize(glm::cross(v1, v2));
    glm::vec3 normal2 = glm::normalize(glm::cross(v3, v4));
    
    // Average the two normals for more stability
    glm::vec3 averageNormal = glm::normalize((normal1 + normal2) * 0.5f);
    
    // Ensure the normal points upward (positive Y component)
    if (averageNormal.y < 0.0f) {
        averageNormal = -averageNormal;
    }
    
    return averageNormal;
}

float CERemotePlayerController::calculateTerrainRoll(const glm::vec3& position, const glm::vec3& rightDirection) {
    // Use more stable sampling with larger offset
    float tileSize = m_map->getTileLength();
    float offset = tileSize * 0.25f; // Match terrain normal sampling
    
    // Sample multiple points for more stable roll calculation
    glm::vec3 leftPosition = position - rightDirection * offset;
    glm::vec3 rightPosition = position + rightDirection * offset;
    
    float leftHeight = m_map->getHeightAtWorldPosition(leftPosition);
    float rightHeight = m_map->getHeightAtWorldPosition(rightPosition);
    
    // Calculate the roll angle based on height difference
    float heightDiff = rightHeight - leftHeight;
    float distance = 2.0f * offset; // Total distance between samples
    
    // Calculate roll angle in radians (positive = right side higher)
    float rollAngle = atan2(heightDiff, distance);
    
    // Clamp to restrictive bounds - creatures can't lean too far sideways
    // Max 15 degrees left/right (much more restrictive for roll)
    return std::max(-glm::radians(5.0f), std::min(glm::radians(5.0f), rollAngle));
}

float CERemotePlayerController::calculateFootprintHeight(const glm::vec3& centerPosition, const glm::vec3& facingDirection) {
    float tileSize = m_map->getTileLength();
    
    // Define character footprint size (adjust these based on character size)
    float footprintLength = tileSize * 0.4f; // Front to back distance
    float footprintWidth = tileSize * 0.25f;  // Left to right distance
    
    // Calculate front, back, left, right positions based on facing direction
    glm::vec3 frontOffset = facingDirection * (footprintLength * 0.5f);
    glm::vec3 rightDirection = glm::normalize(glm::cross(facingDirection, glm::vec3(0, 1, 0)));
    glm::vec3 rightOffset = rightDirection * (footprintWidth * 0.5f);
    
    // Sample heights at the four corners of the footprint (where feet would be)
    float frontLeftHeight = m_map->getHeightAtWorldPosition(centerPosition + frontOffset - rightOffset);
    float frontRightHeight = m_map->getHeightAtWorldPosition(centerPosition + frontOffset + rightOffset);
    float backLeftHeight = m_map->getHeightAtWorldPosition(centerPosition - frontOffset - rightOffset);
    float backRightHeight = m_map->getHeightAtWorldPosition(centerPosition - frontOffset + rightOffset);
    
    // Use the average height of the footprint corners
    // This ensures the character "stands" on the average terrain height of its footprint
    float avgHeight = (frontLeftHeight + frontRightHeight + backLeftHeight + backRightHeight) / 4.0f;
    
    return avgHeight;
}

glm::vec3 CERemotePlayerController::extractEulerAngles(const glm::mat3& rotMatrix) {
    // Extract Euler angles from rotation matrix (YXZ order)
    float pitch = asin(-rotMatrix[2][1]);
    float yaw, roll;
    
    if (cos(pitch) > 0.001f) {
        yaw = atan2(rotMatrix[2][0], rotMatrix[2][2]);
        roll = atan2(rotMatrix[0][1], rotMatrix[1][1]);
    } else {
        yaw = atan2(-rotMatrix[0][2], rotMatrix[0][0]);
        roll = 0.0f;
    }
    
    return glm::vec3(pitch, yaw, roll);
}

bool CERemotePlayerController::isMovementAnimation(const std::string& animationName) {
    // Check if the animation name contains movement-related keywords
    std::string lowerAnimName = animationName;
    std::transform(lowerAnimName.begin(), lowerAnimName.end(), lowerAnimName.begin(), ::tolower);
    
    return (lowerAnimName.find("walk") != std::string::npos ||
            lowerAnimName.find("run") != std::string::npos ||
            lowerAnimName.find("jog") != std::string::npos ||
            lowerAnimName.find("trot") != std::string::npos ||
            lowerAnimName.find("move") != std::string::npos);
}

float CERemotePlayerController::calculateAnimationPlaybackSpeed() {
    // Default playback speed for non-movement animations
    float playbackSpeed = 1.0f;
    
    // Only adjust speed for movement animations
    if (isMovementAnimation(m_current_animation)) {
        // Calculate speed ratio: current movement speed / base walk speed
        float speedRatio = (m_walk_speed > 0) ? (m_current_speed / m_walk_speed) : 0.0f;
        
        // Determine if this is a run animation by checking name and speed
        std::string lowerAnimName = m_current_animation;
        std::transform(lowerAnimName.begin(), lowerAnimName.end(), lowerAnimName.begin(), ::tolower);
        bool isRunAnimation = (lowerAnimName.find("run") != std::string::npos);
        
        if (isRunAnimation) {
            // For run animations, scale based on how fast we're actually moving
            // Expected run speed is typically 1.75x walk speed
            float expectedRunRatio = 1.75f;
            playbackSpeed = m_baseRunAnimSpeed * (speedRatio / expectedRunRatio);
        } else {
            // For walk animations, direct speed ratio
            playbackSpeed = m_baseWalkAnimSpeed * speedRatio;
        }
        
        // Clamp to reasonable bounds (0.3x to 2.0x normal speed)
        playbackSpeed = std::max(0.3f, std::min(2.0f, playbackSpeed));
        
        // If barely moving, slow the animation significantly
        if (m_current_speed < (m_walk_speed * 0.1f)) {
            playbackSpeed *= 0.5f;
        }
    }
    
    return playbackSpeed;
}
