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

#include <random>

bool randomIf(double probability) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::bernoulli_distribution d(probability);
  
  return d(gen);
}

CEAIGenericAmbientManager::CEAIGenericAmbientManager(
                                                     json jsonConfig,
                                                     std::shared_ptr<CERemotePlayerController> playerController,
                                                     std::shared_ptr<C2MapFile> map,
                                                     std::shared_ptr<C2MapRscFile> rsc)
: m_player_controller(playerController),
m_map(map),
m_rsc(rsc),
m_last_process_time(0),
m_target_expire_time(0)
{
  m_current_target = glm::vec3(0.f);
  m_tracked_target = glm::vec3(0.f);
  
  m_path_finder = CEWalkableTerrainPathFinder();
  m_path_finder.map = map;
  m_path_finder.rsc = rsc;
  
  m_path_search_instance = std::make_unique<JPS::Searcher<CEWalkableTerrainPathFinder> >(m_path_finder);
  
  std::string namedAI = jsonConfig.contains("name") ? jsonConfig["name"] : "unnamed";
  float walkSpeed = jsonConfig["character"]["walkSpeed"];
  float heightOffset = jsonConfig["character"].contains("heightOffset") ? (float)jsonConfig["character"]["heightOffset"] : 0.f;

  m_config.WalkAnimName = jsonConfig["animations"]["WALK"];
  m_config.RunAnimName = jsonConfig["animations"].value("RUN", m_config.WalkAnimName);
  m_config.AiName = namedAI;
  m_config.m_idle_odds = jsonConfig["character"].value("idleOdds", 0.3f);
  m_config.m_pf_range = jsonConfig["character"].value("roamRange", 128.f);
  m_config.m_walk_speed = walkSpeed;
  
  if (jsonConfig["animations"].contains("IDLE")) {
    m_config.IdleAnimNames = jsonConfig["animations"]["IDLE"];
  }
  
  m_player_controller->setNextAnimation(m_config.WalkAnimName, true);
  m_player_controller->setWalkSpeed(walkSpeed);
  m_player_controller->setHeightOffset(heightOffset);
}

glm::vec2 CEAIGenericAmbientManager::popNextTarget(double currentTime)
{
  if (!m_path_waypoints.empty()) {
    glm::vec2 nextTarget = m_path_waypoints.back();
    // std::cout << "[" << currentTime << "] " << "Picked next target from queue: x: " << nextTarget.x << " y: " << nextTarget.y << std::endl;
    m_current_target = m_map->getPositionAtCenterTile(nextTarget);
    m_target_expire_time = currentTime + 20.0;
    
    m_path_waypoints.pop_back();
    
    return nextTarget;
  }
  
  return glm::vec2(0);
}

void CEAIGenericAmbientManager::chooseNewTarget(glm::vec3 currentPosition, double currentTime) {
  glm::vec2 nextTarget = popNextTarget(currentTime);
  bool isEmptyTarget = (nextTarget.x == 0 && nextTarget.y == 0);
  
  if (!isEmptyTarget) return;
  
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
  
  int tries = 0;
  float dist = m_config.m_pf_range;
  while (tries < 6) {
    // TODO: replace with A* pathfinding or similar. This is a temporary solution to finalize move and turn to specific point
    std::random_device rd;  // Random device for seeding
    std::mt19937 gen(rd()); // Mersenne Twister generator
    std::uniform_int_distribution<int> distrib(0, (int)directions.size() - 1);
    
    // Pick a random direction
    glm::vec3 direction = directions[distrib(gen)];
    
    targetPosition = currentPosition + direction * dist * tileSize;
    
    // Zero out height
    targetPosition.y = currentPosition.y;
    
    bool found = SetCurrentTarget(targetPosition, currentTime);
    if (found) return;
    
    tries++;
    dist /= 2;
  }
}

void CEAIGenericAmbientManager::updateInflightPathsearch(double currentTime)
{
  if (m_path_search_started_at < 0) return;

  JPS::PathVector path = {};
  auto res = m_path_search_instance->findPathStep(32);

  if (res == JPS_FOUND_PATH) {
    // We found a path. Update planned route
    res = m_path_search_instance->findPathFinish(path, 1);
    if (res == JPS_FOUND_PATH) {
      for (auto p : path) {
        m_path_waypoints.push_back(glm::vec2(p.x, p.y));
      }
      
      popNextTarget(currentTime);
    } else {
      // Some memory issue or something - invalidate target
      m_target_expire_time = currentTime - 1.0;
    }
  
    m_path_search_started_at = -1.0;

    return;
  } else if (res == JPS_NEED_MORE_STEPS) {
    // Keep trying
    return;
  }
  
  // Mark target expired so we pick a new one
  m_target_expire_time = currentTime - 1.0;
}

void CEAIGenericAmbientManager::Process(double currentTime) {
  // Update pathfinding
  double deltaTime = currentTime - m_last_process_time;
  
  // Update in-flight pathfinding first if needed
  updateInflightPathsearch(currentTime);
  
  // Check route following and target state
  glm::vec3 currentPosition = m_player_controller->getPosition();
  glm::vec3 currentForward = m_player_controller->getCamera()->GetForward();
  glm::vec3 forward = glm::normalize(m_current_target - currentPosition);
  glm::vec2 target = m_map->getWorldTilePosition(m_current_target);
  
  bool invalidTarget = (target.x == 0 && target.y == 0);
  bool expired = currentTime > m_target_expire_time;
  bool reachedTarget = glm::length(glm::vec2(currentPosition.x, currentPosition.z) - glm::vec2(m_current_target.x, m_current_target.z)) < m_map->getTileLength();
  
  if (reachedTarget || expired || invalidTarget) {
    chooseNewTarget(currentPosition, currentTime);
    target = m_map->getWorldTilePosition(m_current_target);
    invalidTarget = (target.x == 0 && target.y == 0);
  }

  // Apply behavior
  auto currentAnimation = m_player_controller->getCurrentAnimation();

  // Apply movement
  if (m_mood == FEAR) {
    // Set run animation if needed
    if (m_player_controller->getCurrentAnimation() != m_config.RunAnimName) {
      m_player_controller->setNextAnimation(m_config.RunAnimName, true);
    }
    // Run
    m_player_controller->setTargetSpeed(m_config.m_walk_speed * 1.75f);
    // choose a safe target AWAY from threat source direction (physical body, sound, smell, etc)
  } else if (m_mood == ANGRY) {
    // Set run animation
    if (m_player_controller->getCurrentAnimation() != m_config.RunAnimName) {
      m_player_controller->setNextAnimation(m_config.RunAnimName, true);
    }
    // Speed
    m_player_controller->setTargetSpeed(m_config.m_walk_speed * 1.75f);
    // Decide on attack pattern if needed
    // Move towards the target with attack pattern
  } else if (m_mood == CURIOUS) {
    bool hasIdleAnim = !m_config.IdleAnimNames.empty();
    bool isIdle = hasIdleAnim && isIdleAnimation(currentAnimation);
    if (hasIdleAnim && !isIdle && currentTime - m_last_idle_time > 13.0) {
      if (randomIf(m_config.m_idle_odds)) {
        m_player_controller->setNextAnimation(chooseIdleAnimation(), false);
      }
      m_last_idle_time = currentTime;
    } else if (isIdle && !m_player_controller->isAnimPlaying(currentTime)) {
      m_player_controller->setNextAnimation(m_config.WalkAnimName, true);
    } else if (!isIdle && currentAnimation != m_config.WalkAnimName) {
      m_player_controller->setNextAnimation(m_config.WalkAnimName, true);
    }

    if (isIdle) {
      m_player_controller->setTargetSpeed(0.f);
      m_player_controller->StopMovement();
    } else if (m_player_controller->isTurning()) {
      // TODO: base on turning angle
      m_player_controller->setTargetSpeed(m_config.m_walk_speed * 0.65f);
    } else if (m_player_controller->getCurrentAnimation() == m_config.WalkAnimName) {
      m_player_controller->setTargetSpeed(m_config.m_walk_speed);
    }
  }

  if (!invalidTarget) {
    m_player_controller->MoveTo(m_current_target, deltaTime);
  } else if (invalidTarget) {
    m_player_controller->StopMovement();
    std::cout << m_config.AiName << " cannot move; invalid target. Redeploying." << std::endl;
    m_player_controller->setPosition(m_map->getRandomLanding());
  }
  
  m_player_controller->UpdateLookAtDirection(m_current_target, currentTime);
  
  const double minInterval = 1.0 / 30.0;
  if (currentTime - m_last_upload_time >= minInterval) {
    m_player_controller->uploadStateToHardware();
    m_last_upload_time = currentTime;
  }
  
  m_last_process_time = currentTime;
}

std::string CEAIGenericAmbientManager::chooseIdleAnimation() {
  if (m_config.IdleAnimNames.empty()) {
    throw std::runtime_error("Tried to chooseIdleAnimation but none are defined!");
  }

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distrib(0, (int)m_config.IdleAnimNames.size() - 1);

  int randomIndex = distrib(gen);
  
  return m_config.IdleAnimNames.at(randomIndex);
}


bool CEAIGenericAmbientManager::isIdleAnimation(std::string animationName) {
  // TODO: this is a linear search. Likely fine as usually this is 1-3 items max
  return std::find(m_config.IdleAnimNames.begin(), m_config.IdleAnimNames.end(), animationName) != m_config.IdleAnimNames.end();
}

void CEAIGenericAmbientManager::Reset(double currentTime)
{
  if (m_mood == ANGRY) {
    // Completely forget what we were after
    m_path_waypoints.clear();
    chooseNewTarget(m_player_controller->getPosition(), currentTime);
  }
  
  m_mood = CURIOUS;
}

void CEAIGenericAmbientManager::ReportNotableEvent(glm::vec3 position, std::string eventType, double currentTime) {
  // TODO: add to an attention queue with x ms delay in processing and finite space
  if (eventType == "PLAYER_SPOTTED") {
    m_mood = ANGRY;
    SetCurrentTarget(position, currentTime);
  }
}

bool CEAIGenericAmbientManager::SetCurrentTarget(glm::vec3 targetPosition, double currentTime) {
  float tileSize = m_map->getTileLength();
  glm::vec2 tileCoords = glm::vec2(int(targetPosition.x / tileSize), int(targetPosition.z / tileSize));
  
  if (m_tracked_target == tileCoords) return true;
  
  auto worldPos = m_player_controller->getWorldPosition();

  JPS::PathVector path = {};
  // WARNING: init will abort any active search
  auto res = m_path_search_instance->findPathInit(JPS::Pos(worldPos.x, worldPos.y), JPS::Pos(tileCoords.x, tileCoords.y));
  
  bool found = false;
  if (res == JPS_FOUND_PATH) {
    // Greedy algo already found a path
    res = m_path_search_instance->findPathFinish(path, 1);
    if (res == JPS_FOUND_PATH) {
      for (auto p : path) {
        m_path_waypoints.push_back(glm::vec2(p.x, p.y));
      }

      popNextTarget(currentTime);
      found = true;
    }
    m_path_search_started_at = -1.0;
  } else if (res == JPS_NEED_MORE_STEPS) {
    // Otherwise, defer to future frames
    m_path_search_started_at = currentTime;
    found = true;
  }
  
  m_tracked_target = tileCoords;
  
  return found;
}
