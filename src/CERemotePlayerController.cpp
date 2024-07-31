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

CERemotePlayerController::CERemotePlayerController(std::shared_ptr<LocalAudioManager> audioManager, std::shared_ptr<C2CarFile> carFile, std::shared_ptr<C2MapFile> map, std::shared_ptr<C2MapRscFile> rsc, std::string initialAnimationName): m_map(map), m_rsc(rsc), m_car(carFile), m_current_animation(initialAnimationName), m_g_audio_manager(audioManager)
{
  m_is_deployed = false;
  m_current_speed = 0.f;
  m_target_speed = 0.f;
  m_animation_started_at = 0.0;
  m_animation_last_update_at = 0.0;
  
  // TODO: make these configurable
  // Perhaps an attachable physics controller or something
  m_walk_speed = 256.f * 10.f;
  m_player_height = 256.f;

  m_acceleration = m_walk_speed * 10.f;
  m_deceleration = m_walk_speed * 4.f;
  
  m_look_at = glm::vec3(0.f);
  
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
  
  Transform transform(glm::vec3(0.f), glm::vec3(0, 0, 0), glm::vec3(1.f));
  std::vector<glm::mat4> model = { transform.GetStaticModel() };
  
  // Only ever ONE instance!
  m_geo->UpdateInstances(model);
  m_geo->ConfigureShaderUniforms(m_map.get(), m_rsc.get());
}

glm::vec3 CERemotePlayerController::getPosition() const
{
  return this->m_camera.GetPosition();
}

glm::vec2 CERemotePlayerController::getWorldPosition() const
{
  glm::vec3 pos = m_camera.GetPosition();

  return glm::vec2(int(floorf(pos.x / m_map->getTileLength())), int(floorf(pos.z / m_map->getTileLength())));
}

void CERemotePlayerController::setNextAnimation(std::string animationName)
{
  // TODO: use a separate "next" animation so we can morph between them
  m_current_animation = animationName;
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
  bool didUpdate = m_geo->SetAnimation(anim, currentTime, m_animation_started_at, m_animation_last_update_at, deferUpdate, maxFPS, notVisible);
  
  if (didUpdate) {
    if (m_geo->GetCurrentFrame() == 0) {
      auto audioSrc = m_car->getSoundForAnimation(m_current_animation);
      if (audioSrc) {
        audioSrc->setPosition(m_camera.GetCurrentPos());
        audioSrc->setLooped(false);
        // TODO: allow character to control audio distance
        audioSrc->setMaxDistance(m_map->getTileLength() * 45.f);
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

void CERemotePlayerController::jump(double currentTime)
{
  // TODO: add jump
}

float calculateYaw(const glm::vec3& direction) {
    // Ignore the Y component and normalize the vector
    glm::vec3 directionXZ = glm::normalize(glm::vec3(direction.x, 0.0f, direction.z));

    // Calculate the angle with respect to the positive Z-axis
    float yaw = atan2(directionXZ.x, directionXZ.z);

    return yaw;
}

float getYawDifferenceInDegrees(const glm::vec3& pos1, const glm::vec3& pos2) {
    // Calculate the direction vectors from the origin to the positions
    glm::vec3 direction1 = glm::normalize(glm::vec3(pos1.x, 0.0f, pos1.z));
    glm::vec3 direction2 = glm::normalize(glm::vec3(pos2.x, 0.0f, pos2.z));

    // Calculate the yaw angles in radians
    float yaw1 = calculateYaw(direction1);
    float yaw2 = calculateYaw(direction2);

    // Compute the difference in yaw angles in radians
    float yawDifference = yaw2 - yaw1;

    // Normalize the difference to the range [-π, π]
    if (yawDifference > glm::pi<float>()) {
        yawDifference -= glm::two_pi<float>();
    } else if (yawDifference < -glm::pi<float>()) {
        yawDifference += glm::two_pi<float>();
    }

    // Convert the yaw difference to degrees
    float yawDifferenceDegrees = glm::degrees(yawDifference);

    // Ensure the result is in the range [0, 360)
    if (yawDifferenceDegrees < 0) {
        yawDifferenceDegrees += 360.0f;
    }

    return yawDifferenceDegrees;
}

void CERemotePlayerController::setPosition(glm::vec3 position)
{
  m_camera.SetPos(position);

  // TODO: add lookAt
  Transform transform(m_camera.GetPosition(), glm::vec3(0.f), glm::vec3(1.f));
  std::vector<glm::mat4> model = { transform.GetStaticModel() };
  
  // Only ever ONE instance!
  m_geo->UpdateInstances(model);
}

void CERemotePlayerController::setElevation(float elevation)
{
    this->m_camera.SetHeight(elevation);
}

void CERemotePlayerController::lookAt(glm::vec3 direction)
{
  m_look_at = direction;
}

Camera* CERemotePlayerController::getCamera()
{
  return &this->m_camera;
}

void CERemotePlayerController::move(double currentTime, double deltaTime, bool forwardPressed, bool backwardPressed, bool rightPressed, bool leftPressed)
{
  // TODO: add movement processing
  
  Transform transform(m_camera.GetPosition(), glm::vec3(0.f), glm::vec3(1.f));
  std::vector<glm::mat4> model = { transform.GetStaticModel() };
  
  // Only ever ONE instance!
  m_geo->UpdateInstances(model);
}
