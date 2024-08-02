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
    m_panic_look_target = glm::vec3(0.f);
    m_previous_position = glm::vec3(0.0f);
  m_last_stuck_time = 0.0;
}

bool CEAIGenericAmbientManager::isTilePassable(glm::vec2 tile, const glm::vec3& currentForward, const glm::vec3& potentialForward) {
    bool isWater = m_map->hasWaterAt(tile.x, tile.y);
    return !isWater;
}

bool CEAIGenericAmbientManager::isTileSafe(glm::vec2 tile) {
    // Check if the tile itself and its surrounding tiles are passable and not adjacent to water
    std::vector<glm::vec2> surroundingTiles = {
        tile,
        glm::vec2(tile.x + 3, tile.y),   // East
        glm::vec2(tile.x - 3, tile.y),   // West
        glm::vec2(tile.x, tile.y + 3),   // North
        glm::vec2(tile.x, tile.y - 3),   // South
        glm::vec2(tile.x + 3, tile.y + 3), // Northeast
        glm::vec2(tile.x - 3, tile.y + 3), // Northwest
        glm::vec2(tile.x + 3, tile.y - 3), // Southeast
        glm::vec2(tile.x - 3, tile.y - 3)  // Southwest
    };

    for (const auto& t : surroundingTiles) {
      if (t.x < 0 || t.x >= m_map->getWidth() || t.y < 0 || t.y >= m_map->getHeight()) {
        return false;
      }
      
        if (m_map->hasWaterAt(t.x, t.y)) {
            return false;
        }
    }
    return true;
}

void CEAIGenericAmbientManager::chooseNewTarget(glm::vec3 currentPosition, bool getUnstuck, double timeDelta) {
    const float tileSize = 256.0f;
    glm::vec3 targetPosition;
    bool foundSafeTile = false;
    double currentTime = glfwGetTime();

    // Predefined directions for cardinal and intercardinal directions
    std::vector<glm::vec3> directions = {
        glm::vec3(1, 0, 0),   // East
        glm::vec3(1, 0, 1),   // Northeast
        glm::vec3(0, 0, 1),   // North
        glm::vec3(-1, 0, 1),  // Northwest
        glm::vec3(-1, 0, 0),  // West
        glm::vec3(-1, 0, -1), // Southwest
        glm::vec3(0, 0, -1),  // South
        glm::vec3(1, 0, -1)   // Southeast
    };

    if (getUnstuck) {
        // Try to find a safe tile by cycling through predefined directions
        for (const auto& direction : directions) {
            // Calculate the potential target position 1 tile away in the chosen direction
            glm::vec3 potentialTarget = currentPosition + glm::normalize(direction) * tileSize;

            // Check if the tile at the potential target and its surroundings are safe
            glm::vec2 tileCoords = glm::vec2(potentialTarget.x / tileSize, potentialTarget.z / tileSize);
            if (isTileSafe(tileCoords)) {
                // Set the found safe tile as the target position
                targetPosition = potentialTarget;
                foundSafeTile = true;
                break;
            }
        }

        if (foundSafeTile) {
            // Set the new target 3 tiles in the direction of the safe tile
            m_current_target = targetPosition + glm::normalize(m_current_target - currentPosition) * 6.0f * tileSize;
            m_target_expire_time = currentTime + 12.0;
        } else {
            // If no safe tile is found, fallback to the current position
            m_current_target = currentPosition;
            m_target_expire_time = currentTime + 12.0;
        }
    } else {
        // Regular target selection
        for (int attempt = 0; attempt < 10; ++attempt) {
            glm::vec3 direction = glm::vec3(
                static_cast<float>(rand()) / RAND_MAX - 0.5f,
                0.0f,
                static_cast<float>(rand()) / RAND_MAX - 0.5f
            );
            direction = glm::normalize(direction);

            targetPosition = currentPosition + direction * 12.0f * tileSize;

            glm::vec2 tileCoords = glm::vec2(targetPosition.x / tileSize, targetPosition.z / tileSize);
            if (isTileSafe(tileCoords)) {
                m_current_target = targetPosition;
                m_target_expire_time = currentTime + 10.0;
                break;
            }
        }
    }
}


float CEAIGenericAmbientManager::calculateEnergyCost(glm::vec3 currentPos, glm::vec3 targetPos) {
    glm::vec2 horizontalDistanceVec = glm::vec2(targetPos.x - currentPos.x, targetPos.z - currentPos.z);
    float horizontalDistance = glm::length(horizontalDistanceVec);
    float elevationChange = targetPos.y - currentPos.y;
    return horizontalDistance + 2.0f * fabs(elevationChange);
}

void CEAIGenericAmbientManager::updateLookAtDirection(glm::vec3 desiredLookAt, float deltaTime) {
  // TODO: make this fancier. For now this works.
  m_player_controller->lookAt(desiredLookAt);
}


void CEAIGenericAmbientManager::Process(double currentTime) {
    double deltaTime = currentTime - m_last_process_time;

    glm::vec3 currentPosition = m_player_controller->getPosition();
    glm::vec3 currentForward = m_player_controller->getCamera()->GetForward();
    glm::vec3 forward = glm::normalize(m_current_target - currentPosition);
  
  bool expired = currentTime > m_target_expire_time;
  bool reachedTarget = glm::length(currentPosition - m_current_target) < m_map->getTileLength();
  bool isStuck = m_stuck_timer > 5.f;

    if ((reachedTarget || expired) && !isStuck) {
        chooseNewTarget(currentPosition, false, deltaTime);
    }
  
  glm::vec3 target = m_current_target;

    if (glm::length(currentPosition - m_previous_position) < 32.0f && !reachedTarget && !expired) {
        m_stuck_timer += deltaTime;
        if (isStuck) {
          if (m_last_stuck_time <= 0) {
            m_last_stuck_time = currentTime;
          }
          float radius = 256.0f * 8.f; // Circle radius
          float rotationSpeed = 2.0f; // Control the speed of rotation (adjust as needed)
          float angle = fmod((currentTime - m_last_stuck_time) * rotationSpeed, 2.f * glm::pi<float>());

          // Calculate the panic look target
          m_panic_look_target = currentPosition + glm::vec3(
              radius * cos(angle),
              0.f,
              radius * sin(angle)
          );
          target = m_panic_look_target;
        }
    } else {
        m_stuck_timer = 0;
        m_last_stuck_time = -1;
    }

    updateLookAtDirection(target, static_cast<float>(deltaTime));

    if (isTilePassable(glm::vec2(currentPosition.x / 256.f, currentPosition.z / 256.f), currentForward, forward)) {
        m_player_controller->MoveTo(m_current_target, deltaTime);
    } else if (!m_stuck_timer) {
        chooseNewTarget(currentPosition, false, deltaTime);
    } else if (isStuck) {
      m_player_controller->MoveTo(target, deltaTime);
    }
  
  // Check if the AI is on an unsafe tile
  if (!isTileSafe(m_player_controller->getWorldPosition())) {
      if (!m_invertedLookDirection && !isStuck) {
          // Invert the look direction
          glm::vec3 invertedLookAt = currentPosition - currentForward;
          m_player_controller->lookAt(invertedLookAt);
          m_invertedLookDirection = true; // Set the flag
      }
  } else {
      // Reset the flag once the AI moves to a safe tile
      m_invertedLookDirection = false;
  }
  
  m_previous_position = currentPosition;
  m_last_process_time = currentTime;
  
  m_player_controller->uploadStateToHardware();
}
