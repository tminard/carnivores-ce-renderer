#include "CEAIGenericAmbientManager.hpp"
#include "CERemotePlayerController.hpp"
#include "C2MapFile.h"
#include "C2MapRscFile.h"

#include <random>
#include <cmath>
#include <iostream>
#include <glm/gtc/quaternion.hpp> // Include for quaternion operations
#include <glm/gtx/quaternion.hpp> // Include for quaternion slerp and rotations

CEAIGenericAmbientManager::CEAIGenericAmbientManager(
    AIGenericAmbientManagerConfig config,
    std::shared_ptr<CERemotePlayerController> playerController,
    std::shared_ptr<C2MapFile> map,
    std::shared_ptr<C2MapRscFile> rsc)
    : m_config(config), m_player_controller(playerController), m_map(map), m_rsc(rsc), m_last_process_time(0), m_target_expire_time(0), m_current_target(-1.0f)
{
}

const float move_distance = 228.0f; // Movement per delta time
const std::vector<glm::vec2> directions = {
    {1, 0},   // Right
    {1, 1},   // Up-right
    {0, 1},   // Up
    {-1, 1},  // Up-left
    {-1, 0},  // Left
    {-1, -1}, // Down-left
    {0, -1},  // Down
    {1, -1}   // Down-right
};

bool CEAIGenericAmbientManager::isTilePassable(glm::vec2 tile)
{
    glm::vec3 actualPos = m_map->getPositionAtCenterTile(tile);
    glm::vec3 curPos = m_player_controller->getPosition();

    // Check if movement here is allowed based on height difference
    if (std::abs(actualPos.y - curPos.y) < 128.0f) {
        return true;
    }

    // TODO: Add water and other impassable checks

    return false;
}

void CEAIGenericAmbientManager::Process(double currentTime)
{
    double delta = currentTime - m_last_process_time;
    glm::vec3 currentPosition = m_player_controller->getPosition();

    float distToTarget = glm::distance(currentPosition, m_current_target);

    // Determine if we need a new target
    bool curTargetExpired = m_target_expire_time < currentTime;
    bool needNewTarget = curTargetExpired || m_current_target.x < 0.0f || distToTarget < 128.0f;

    if (needNewTarget) {
        float range = m_map->getTileLength() * 12.0f;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dist(-range, range);

        float randomOffsetX = dist(gen);
        float randomOffsetZ = dist(gen);

        glm::vec3 targetPosition = currentPosition + glm::vec3(randomOffsetX, 0.0f, randomOffsetZ);
        targetPosition.y = m_map->getPositionAtCenterTile(m_map->getWorldTilePosition(targetPosition)).y;

        m_current_target = targetPosition;
        m_target_expire_time = currentTime + 30;
    }

    // Determine the best direction to move towards the target
    float bestValue = -std::numeric_limits<float>::max();
    bool blocked = true;
    glm::vec3 bestDirection = currentPosition;

    for (const glm::vec2 &dir : directions) {
        glm::vec3 potentialPosition = currentPosition + glm::vec3(dir.x, 0, dir.y) * move_distance * static_cast<float>(delta);
        glm::vec2 potentialPositionTile = m_map->getWorldTilePosition(potentialPosition);

        // Check if the potential position is passable
        bool isPassable = isTilePassable(potentialPositionTile);

        // Calculate the distance to the target from the potential position
        float potentialDistToTarget = glm::distance(potentialPosition, m_current_target);

        // Determine a value that represents how good this direction is
        float value = -potentialDistToTarget; // Closer to target is better, so use negative distance

        // If the position is passable and has a better value, select it
        if (isPassable && value > bestValue) {
            bestValue = value;
            bestDirection = potentialPosition;
            blocked = false;
        }
    }

    if (blocked) {
        // Reverse direction (try the opposite direction)
        m_current_target = currentPosition - glm::normalize(m_current_target - currentPosition) * move_distance;
        bestDirection = m_current_target;
    }

    // Look at the direction we are moving with smooth interpolation
    glm::vec3 moveDirection = bestDirection - currentPosition;

    if (glm::length(moveDirection) > 0.0f) {
        glm::vec3 currentForward = m_player_controller->getCamera()->GetForward();
        glm::vec3 desiredForward = glm::normalize(moveDirection);

        // Compute the axis of rotation
        glm::vec3 rotationAxis = glm::cross(currentForward, desiredForward);

        if (glm::length(rotationAxis) > 0.0f) {
            float dotProduct = glm::dot(currentForward, desiredForward);
            float angle = std::acos(glm::clamp(dotProduct, -1.0f, 1.0f)); // Clamp to avoid domain errors

            // Create quaternion from axis and angle
            glm::quat rotationQuat = glm::angleAxis(angle, glm::normalize(rotationAxis));

            // Interpolate between the current and target orientations
            float rotationSpeed = 0.1f; // Adjust the rotation speed as needed
            glm::quat currentQuat = glm::quat_cast(glm::lookAt(currentPosition, currentPosition + currentForward, glm::vec3(0.0f, 1.0f, 0.0f)));
            glm::quat newQuat = glm::slerp(currentQuat, rotationQuat, static_cast<float>(delta * rotationSpeed));

            // Update lookAt with new forward vector
            glm::vec3 newForward = glm::normalize(newQuat * currentForward);
            m_player_controller->lookAt(currentPosition + newForward);
        }
    }

    // Move towards the best direction found
    if (!blocked) {
        bestDirection.y = m_map->getPositionAtCenterTile(m_map->getWorldTilePosition(bestDirection)).y;
        m_player_controller->setPosition(bestDirection);
    }

    // Update animation state if needed
    m_player_controller->setNextAnimation(m_config.WalkAnimName);

    m_last_process_time = currentTime;
}
