#include "CEAIGenericAmbientManager.hpp"

#include "CERemotePlayerController.hpp"
#include "C2MapFile.h"
#include "C2MapRscFile.h"

#include <random>
#include <cmath>
#include <iostream>

CEAIGenericAmbientManager::CEAIGenericAmbientManager(AIGenericAmbientManagerConfig config, std::shared_ptr<CERemotePlayerController> playerController, std::shared_ptr<C2MapFile> map, std::shared_ptr<C2MapRscFile> rsc): m_config(config), m_player_controller(playerController), m_map(map), m_rsc(rsc)
{
  m_last_process_time = 0;
  m_target_expire_time = 0;
  m_current_target = glm::vec3(-1.f);
}

const float move_distance = 164.f; // Movement per delta time
// TODO: only allow movement in direction we are facing
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
  // check if movement here is allowed
  if (abs((actualPos.y - curPos.y)) < 128.f) {
    return true;
  }
  
  // TODO: check for water
  
  return false;
}

void CEAIGenericAmbientManager::Process(double currentTime)
{
  double delta = currentTime - m_last_process_time;
  glm::vec3 currentPosition = m_player_controller->getPosition();
  
  float distToTarget = glm::distance(currentPosition, m_current_target);
  
  // Determine target
  bool curTargetExpired = m_target_expire_time < currentTime;
  // TODO: deal with getting stuck
  bool needNewTarget = curTargetExpired || m_current_target.x < 0.f || distToTarget < 128.f;
  if (needNewTarget) {
    float range = (m_map->getTileLength() * 12.f);
    std::random_device rd; // Seed
    std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<float> dist(-range, range);

    // Pick a random x and y position within n tiles in either direction
    float randomOffsetX = dist(gen);
    float randomOffsetY = dist(gen);
    
    m_current_target.x = currentPosition.x + randomOffsetX;
    m_current_target.y = currentPosition.z;
    m_current_target.z = currentPosition.y + randomOffsetY;
    
    m_target_expire_time = currentTime + 30;
  }
  
  // Move toward target, avoiding blockers and impassable terrain
  // Find best direction to move towards the target
  float bestValue = -std::numeric_limits<float>::max();
  glm::vec3 bestDirection = currentPosition;

  for (const glm::vec2& dir : directions) {
      glm::vec3 potentialPosition = currentPosition + glm::vec3(dir.x, 0, dir.y) * move_distance * static_cast<float>(delta);
      // get tile at that position
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
      }
  }
  
  // Make sure we are looking in the direction we are walking
  // If not, we need to rotate (via slide animation?) for N frames until we are,
  // or move in our forward direction while altering our rotation each frame

  // Move towards the best direction found
  if (bestValue != -std::numeric_limits<float>::max()) {
    // TODO: call move and delete all movement time processing from this class!
      bestDirection.y = m_map->getPositionAtCenterTile(m_player_controller->getWorldPosition()).y;
    m_player_controller->lookAt(bestDirection);
    m_player_controller->setPosition(bestDirection);
  }
    
  // Update animation state if needed
  m_player_controller->setNextAnimation(m_config.WalkAnimName);
  
  m_last_process_time = currentTime;
}
