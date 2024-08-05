//
//  CERemotePlayerController.cpp
//  CarnivoresRenderer
//
//  Created by Tyler Minard on 7/30/24.
//

#include "CERemotePlayerController.hpp"

#include <iostream>

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

CERemotePlayerController::CERemotePlayerController(std::shared_ptr<LocalAudioManager> audioManager, std::shared_ptr<C2CarFile> carFile, std::shared_ptr<C2MapFile> map, std::shared_ptr<C2MapRscFile> rsc, std::string initialAnimationName): m_map(map), m_rsc(rsc), m_car(carFile), m_current_animation(initialAnimationName), m_g_audio_manager(audioManager)
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

void CERemotePlayerController::update(double currentTime, Transform &baseTransform, Camera &observerCamera, glm::vec2 observerWorldPosition)
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
  
  m_geo->Update(baseTransform, observerCamera);
  bool didUpdate = m_geo->SetAnimation(anim, currentTime, m_animation_started_at, m_animation_last_update_at, deferUpdate, maxFPS, notVisible, 1.f, m_is_looping_anim);
  
  if (didUpdate) {
    if (m_geo->GetCurrentFrame() == 0) {
      auto audioSrc = m_car->getSoundForAnimation(m_current_animation);
      if (audioSrc) {
        audioSrc->setPosition(m_camera.GetPosition());
        audioSrc->setLooped(false);
        audioSrc->setMaxDistance(m_map->getTileLength() * 60.f);
        audioSrc->setClampDistance(6);
        m_g_audio_manager->play(audioSrc);
      }
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
  // Calculate the direction vector from the object position to the lookAt target
  glm::vec3 direction = glm::normalize(m_camera.GetLookAt() - position);
  
  // Calculate the yaw rotation angle needed to face the target
  float yaw = atan2(direction.x, direction.z);
  
  // Set the rotation with the correct yaw angle
  glm::vec3 rotation(0, yaw + glm::radians(180.f), 0);
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

void CERemotePlayerController::UpdateLookAtDirection(glm::vec3 desiredLookAt, double currentTime) {
  glm::vec3 currentPosition = getPosition();
  glm::vec3 currentLookAt = m_camera.GetLookAt();
  
  glm::vec3 currentForward = glm::normalize(currentLookAt - currentPosition);
  glm::vec3 desiredForward = glm::normalize(desiredLookAt - currentPosition);
  
  // Calculate the angle difference between the current and desired forward directions
  float angleDifference = glm::degrees(glm::acos(glm::clamp(glm::dot(currentForward, desiredForward), -1.0f, 1.0f)));
  const float angleThreshold = 5.0f; // Degrees
  bool shouldTurn = angleDifference > angleThreshold;
  
  if (shouldTurn && m_start_turn_time < 0.0) {
    m_start_turn_time = currentTime;
    m_start_turn_lookat = currentLookAt;
  }
  
  // Set the camera to look at the new position
  if (shouldTurn) {
    // Calculate the starting and desired forward vectors
    glm::vec3 startForward = glm::normalize(m_start_turn_lookat - currentPosition);
    
    // Calculate the time elapsed since the turn started
    float timeElapsed = static_cast<float>(currentTime - m_start_turn_time);
    const float rotationDuration = 1.0f; // Duration in seconds to complete the rotation
    
    // Ensure the elapsed time does not exceed the rotation duration
    float t = glm::clamp(timeElapsed / rotationDuration, 0.0f, 1.0f);
    
    // Interpolate between the start and desired forward vectors
    glm::vec3 interpolatedForward = glm::normalize(glm::mix(startForward, desiredForward, t));
    
    // Calculate the new lookAt position
    glm::vec3 updatedLookAt = glm::mix(m_start_turn_lookat, desiredLookAt, t);
    lookAt(updatedLookAt);
  } else {
    lookAt(desiredLookAt);
    m_start_turn_time = -1.0;
    m_start_turn_lookat = desiredLookAt;
  }
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
  glm::vec3 currentLookAt = this->getCamera()->GetLookAt();
  glm::vec3 direction = glm::normalize(currentLookAt - currentPosition);
  float distance = glm::distance(currentPosition, targetPosition);
  
  // Calculate the time delta
  if (deltaTime <= 0.0) {
    return; // Prevent division by zero or negative deltaTime
  }
  
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
  
  // Obtain terrain height at the new position to adjust the y-coordinate
  float groundHeight = m_map->getPlaceGroundHeight(worldPos.x, worldPos.y) + m_player_height;
  float smoothedHeight = (groundHeight * 0.1f) + (m_previousHeight * 0.9f);
  m_previousHeight = smoothedHeight;
  newPosition.y = smoothedHeight;
  
  // Set the new position without changing the lookAt direction
  setPosition(newPosition);
}

void CERemotePlayerController::setHeightOffset(float offset) { 
  m_player_height = offset;
}

void CERemotePlayerController::setTargetSpeed(float targetSpeed) { 
  m_target_speed = m_map->getTileLength() * targetSpeed;
}
