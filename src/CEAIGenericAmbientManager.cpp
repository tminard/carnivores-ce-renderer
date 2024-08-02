#include "CEAIGenericAmbientManager.hpp"
#include "CERemotePlayerController.hpp"
#include "C2MapFile.h"
#include "C2MapRscFile.h"
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <unordered_map>
#include <cmath>
#include <iostream>

CEAIGenericAmbientManager::CEAIGenericAmbientManager(
    AIGenericAmbientManagerConfig config,
    std::shared_ptr<CERemotePlayerController> playerController,
    std::shared_ptr<C2MapFile> map,
    std::shared_ptr<C2MapRscFile> rsc)
    : m_config(config),
      m_player_controller(playerController),
      m_map(map),
      m_rsc(rsc),
      m_last_process_time(0),
      m_target_expire_time(0),
      m_stuck_timer(0)
{
    m_current_target = glm::vec3(0.0f);
    m_previous_position = glm::vec3(0.0f);
}

bool CEAIGenericAmbientManager::isTilePassable(glm::vec2 tile, const glm::vec3& currentForward, const glm::vec3& potentialForward) {
    bool isWater = m_map->hasWaterAt(tile.x, tile.y);
    return !isWater;
}

void CEAIGenericAmbientManager::chooseNewTarget(glm::vec3 currentPosition, bool getUnstuck, double timeDelta) {
  double currentTime = glfwGetTime();
    const float tileSize = 256.0f;
    glm::vec3 direction = glm::vec3(
        static_cast<float>(rand()) / RAND_MAX - 0.5f,
        0.0f,
        static_cast<float>(rand()) / RAND_MAX - 0.5f
    );
    direction = glm::normalize(direction);

    m_current_target = currentPosition + (direction * 128.0f * tileSize);

    if (!getUnstuck) {
        m_target_expire_time = currentTime + 45.0;
    }
}

float CEAIGenericAmbientManager::calculateEnergyCost(glm::vec3 currentPos, glm::vec3 targetPos) {
    glm::vec2 horizontalDistanceVec = glm::vec2(targetPos.x - currentPos.x, targetPos.z - currentPos.z);
    float horizontalDistance = glm::length(horizontalDistanceVec);
    float elevationChange = targetPos.y - currentPos.y;
    return horizontalDistance + 2.0f * fabs(elevationChange);
}

void CEAIGenericAmbientManager::updateLookAtDirection(glm::vec3 desiredLookAt, float deltaTime) {
    // Get the current position and lookAt direction
    glm::vec3 currentPosition = m_player_controller->getPosition();
    glm::vec3 currentLookAt = m_player_controller->getCamera()->GetLookAt();

    // Compute the current and desired forward vectors
    glm::vec3 currentForward = glm::normalize(currentLookAt - currentPosition);
    glm::vec3 desiredForward = glm::normalize(desiredLookAt - currentPosition);

    // If the current and desired forward vectors are different, interpolate between them
    if (glm::length(currentForward - desiredForward) > 0.001f) {
        // Define the speed of rotation in degrees per second
        const float rotationSpeed = 45.0f; // Adjust this value for faster/slower rotation

        // Calculate the maximum allowed rotation amount for this frame in radians
        float maxRotationAmount = glm::radians(rotationSpeed * deltaTime);

        // Calculate the dot product to find the cosine of the angle
        float dotProduct = glm::clamp(glm::dot(currentForward, desiredForward), -1.0f, 1.0f);

        // Calculate the angle difference between the current and desired directions
        float angleDifference = glm::acos(dotProduct);

        // If the angle difference is small enough, snap to the desired direction
        if (angleDifference < 0.01f) {
            m_player_controller->lookAt(desiredLookAt);
        } else {
            // Calculate the interpolation factor based on the maximum rotation amount
            float t = glm::min(maxRotationAmount / angleDifference, 1.0f);

            // Interpolate between the current and desired forward directions
            glm::vec3 newForward = glm::normalize(glm::mix(currentForward, desiredForward, t));

            // Set the new lookAt direction
            glm::vec3 newLookAt = currentPosition + newForward;
            m_player_controller->lookAt(newLookAt);
        }
    } else {
        // If the vectors are almost the same, directly set the lookAt to the desired direction
        m_player_controller->lookAt(desiredLookAt);
    }
}


void CEAIGenericAmbientManager::Process(double currentTime) {
    double deltaTime = currentTime - m_last_process_time;

    glm::vec3 currentPosition = m_player_controller->getPosition();
    glm::vec3 currentForward = m_player_controller->getCamera()->GetForward();
    glm::vec3 forward = glm::normalize(m_current_target - currentPosition);
  
  bool expired = currentTime > m_target_expire_time;
  bool reachedTarget = glm::length(currentPosition - m_current_target) < m_map->getTileLength();

    if (reachedTarget || expired) {
      std::cout << "Expired: " << expired << "; at target: " << reachedTarget << std::endl;
        chooseNewTarget(currentPosition, false, deltaTime);
    }

//    if (glm::length(currentPosition - m_previous_position) < 128.0f) {
//        m_stuck_timer += deltaTime;
//      std::cout << "Stuck mofo" << m_stuck_timer << std::endl;
//        if (m_stuck_timer > 4.0) {
//          std::cout << "pick new dest" << std::endl;
//            chooseNewTarget(currentPosition, true, deltaTime);
//            m_stuck_timer = 0;
//        }
//    } else {
//        m_stuck_timer = 0;
//    }

    updateLookAtDirection(m_current_target, static_cast<float>(deltaTime));

    m_previous_position = currentPosition;
    m_last_process_time = currentTime;

    if (isTilePassable(glm::vec2(currentPosition.x / 256.f, currentPosition.z / 256.f), currentForward, forward)) {
        m_player_controller->MoveTo(m_current_target, deltaTime);
    } else {
      std::cout << "Not passable - get new target" << std::endl;
        chooseNewTarget(currentPosition, false, deltaTime);
    }
  
  m_player_controller->uploadStateToHardware();
}
