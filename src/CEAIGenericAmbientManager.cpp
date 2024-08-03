#include "CEAIGenericAmbientManager.hpp"
#include "CERemotePlayerController.hpp"
#include "C2MapFile.h"
#include "C2MapRscFile.h"
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <unordered_map>
#include <cmath>
#include <iostream>

#include "CEWalkableTerrainPathFinder.hpp"

#include "jps.hpp"

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
m_target_expire_time(0)
{
  m_current_target = glm::vec3(0.0f);
  
//  m_path_waypoints.resize(5);
//  m_path_waypoints.push_back(glm::vec2(256, 256));
//  m_path_waypoints.push_back(glm::vec2(128, 256));
//  m_path_waypoints.push_back(glm::vec2(64, 128));
//  m_path_waypoints.push_back(glm::vec2(64, 64));
//  m_path_waypoints.push_back(glm::vec2(38, 256));
  
  m_path_finder = CEWalkableTerrainPathFinder();
  m_path_finder.map = map;
  m_path_finder.rsc = rsc;
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

void CEAIGenericAmbientManager::chooseNewTarget(glm::vec3 currentPosition, double currentTime) {
  if (!m_path_waypoints.empty()) {
    glm::vec2 nextTarget = m_path_waypoints.back();
    // std::cout << "[" << currentTime << "] " << "Picked next target from queue: x: " << nextTarget.x << " y: " << nextTarget.y << std::endl;
    m_current_target = m_map->getPositionAtCenterTile(nextTarget);
    m_target_expire_time = currentTime + 20.0;
    
    m_path_waypoints.pop_back();
    
    bool isEmptyTarget = (nextTarget.x == 0 && nextTarget.y == 0);
    
    if (!isEmptyTarget) return;
  }
  
  const float tileSize = m_map->getTileLength();
  glm::vec3 targetPosition;
  bool foundSafeTile = false;
  
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
  
  // TODO: replace with A* pathfinding or similar. This is a temporary solution to finalize move and turn to specific point
  std::random_device rd;  // Random device for seeding
  std::mt19937 gen(rd()); // Mersenne Twister generator
  std::uniform_int_distribution<int> distrib(0, (int)directions.size() - 1);
  
  // Pick a random direction
  glm::vec3 direction = directions[distrib(gen)];
  
  targetPosition = currentPosition + direction * 62.0f * tileSize;
  
  // Zero out height
  targetPosition.y = currentPosition.y;
  
  glm::vec2 tileCoords = glm::vec2(int(targetPosition.x / tileSize), int(targetPosition.z / tileSize));
  if (isTileSafe(tileCoords)) {
    // TODO: use incremental pathfinding here (w/ cache)
    // std::cout << "Finding path to [" << tileCoords.x << ", " << tileCoords.y << "]" << std::endl;
    JPS::PathVector path = {};
    auto worldPos = m_player_controller->getWorldPosition();
    bool found = JPS::findPath(path, m_path_finder, worldPos.x, worldPos.y, tileCoords.x, tileCoords.y, 1);
    if (found) {
      for (auto p : path) {
        // std::cout << "\tFOUND PATH: [" << p.x << ", " << p.y << "]" << std::endl;
        m_path_waypoints.push_back(glm::vec2(p.x, p.y));
      }
      return;
    }
    
    //std::cerr << "Failed to find path... restart." << std::endl;
  }
}

void CEAIGenericAmbientManager::Process(double currentTime) {
  double deltaTime = currentTime - m_last_process_time;
  
  glm::vec3 currentPosition = m_player_controller->getPosition();
  glm::vec3 currentForward = m_player_controller->getCamera()->GetForward();
  glm::vec3 forward = glm::normalize(m_current_target - currentPosition);
  
  bool expired = currentTime > m_target_expire_time;
  bool reachedTarget = glm::length(glm::vec2(currentPosition.x, currentPosition.z) - glm::vec2(m_current_target.x, m_current_target.z)) < m_map->getTileLength();
  
  if (reachedTarget || expired) {
    // std::cout << "[ " << currentTime << "] Target expired or reached. Picking new" << std::endl;
    chooseNewTarget(currentPosition, currentTime);
  }
  
  m_player_controller->MoveTo(m_current_target, deltaTime);
  m_player_controller->UpdateLookAtDirection(m_current_target, currentTime);

  const double minInterval = 1.0 / 30.0;
  if (currentTime - m_last_upload_time >= minInterval) {
      m_player_controller->uploadStateToHardware();
      m_last_upload_time = currentTime;
  }
  
  m_last_process_time = currentTime;
}
