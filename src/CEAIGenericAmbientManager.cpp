#include "CEAIGenericAmbientManager.hpp"
#include "CERemotePlayerController.hpp"
#include "CELocalPlayerController.hpp"
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

glm::vec3 rotateVector(const glm::vec3& vector, float angleRad) {
  float cosAngle = std::cos(angleRad);
  float sinAngle = std::sin(angleRad);
  glm::vec3 rotatedVector;
  rotatedVector.x = vector.x * cosAngle - vector.z * sinAngle;
  rotatedVector.z = vector.x * sinAngle + vector.z * cosAngle;
  rotatedVector.y = vector.y; // Keep the y component the same as we are rotating in the XZ plane.
  return rotatedVector;
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
  
  m_debug = jsonConfig.value("DEBUG", false);
  
  if (jsonConfig["animations"].contains("IDLE")) {
    m_config.IdleAnimNames = jsonConfig["animations"]["IDLE"];
  }
  
  if (jsonConfig.contains("behavior") && jsonConfig["behavior"].is_object()) {
    m_view_range = jsonConfig["behavior"].value("vision", DEFAULT_VIEW_RANGE);
    m_is_dangerous = jsonConfig["behavior"].value("damageOnContact", DEFAULT_IS_DANGER);
    m_min_attack_chance = jsonConfig["behavior"].value("minAttackChance", DEFAULT_MIN_ATTACK);
    m_max_attack_chance = jsonConfig["behavior"].value("maxAttackChance", DEFAULT_MAX_ATTACK);
  }
  
  m_player_controller->setNextAnimation(m_config.WalkAnimName, true);
  m_player_controller->setWalkSpeed(walkSpeed);
  m_player_controller->setHeightOffset(heightOffset);
}

glm::vec2 CEAIGenericAmbientManager::popNextTarget(double currentTime)
{
  if (!m_path_waypoints.empty()) {
    glm::vec2 nextTarget = m_path_waypoints.back();
    m_current_target = m_map->getPositionAtCenterTile(nextTarget);
    m_target_expire_time = currentTime + 20.0;
    
    m_path_waypoints.pop_back();
    
    return nextTarget;
  }
  
  return glm::vec2(0);
}

void CEAIGenericAmbientManager::chooseNewTarget(glm::vec3 currentPosition, double currentTime) {
  if (m_debug) std::cout << currentTime << " " << m_config.AiName << " DEBUG: " << currentTime << ":" << "CEAIGenericAmbientManager::chooseNewTarget" << ": called." << std::endl;
  glm::vec2 nextTarget = popNextTarget(currentTime);
  bool isEmptyTarget = (nextTarget.x == 0 && nextTarget.y == 0);
  
  if (!isEmptyTarget) {
    if (m_debug) std::cout << currentTime << " " << m_config.AiName << " DEBUG: " << ":" << "CEAIGenericAmbientManager::chooseNewTarget" << ": target queue not empty. Using next location. Queue: " << m_path_waypoints.size() << std::endl;
    return;
  }
  
  const float tileSize = m_map->getTileLength();
  glm::vec3 targetPosition;
  bool foundSafeTile = false;
  
  // No planned waypoint available - pick a suitable direction instead
  if (m_debug) std::cout << currentTime << " " << m_config.AiName << " DEBUG: " << ":" << "CEAIGenericAmbientManager::chooseNewTarget" << ": Queue empty. Deciding on next route. Mood: " << m_mood << std::endl;
  
  int tries = 0;
  float dist = m_config.m_pf_range;
  std::random_device rd;  // Random device for seeding
  std::mt19937 gen(rd()); // Mersenne Twister generator
  std::uniform_int_distribution<int> distrib(0, (int)directions.size() - 1);

  while (tries < 12 && dist > 1) {
    if (m_mood == CURIOUS) {
      // Pick a random direction
      glm::vec3 direction = directions[distrib(gen)];
      targetPosition = currentPosition + direction * dist * tileSize;
      targetPosition.y = currentPosition.y;
      
      bool found = SetCurrentTarget(targetPosition, currentTime);
      if (m_debug) std::cout << currentTime << " " << m_config.AiName << " DEBUG: " << ":" << "CEAIGenericAmbientManager::chooseNewTarget" << ": Mood curious. Next rando spot selected? Found: " << found << std::endl;

      if (found) return;

      dist /= 2;
    } else if (m_tracked_target.x > 0.f && m_tracked_target.y > 0.f) {
      // Try to get "close enough" by moving a few tiles towards target
      // Find a point in direction of target
      // Note we do a one-way short distance JPS search to avoid resetting any in-flight
      glm::vec2 worldPos = m_player_controller->getWorldPosition();
      glm::vec2 direction;
      
      float factor = m_mood == ANGRY ? 1.0 : -1.0; // move towards OR away
      if (m_debug) std::cout << currentTime << " " << m_config.AiName << " DEBUG: " << ":" << "CEAIGenericAmbientManager::chooseNewTarget" << ": Tracked target available. Attempting to move: " << factor << std::endl;
      
      if (factor > 0.0) {
        direction = glm::normalize(m_tracked_target - worldPos);
      } else {
        direction = glm::normalize(worldPos - m_tracked_target);
      }

      // Try a fixed distance
      glm::vec2 targetPos = worldPos + direction * (6.f);

      // Run an inline search
      // TODO: improve performance of this!
      JPS::PathVector path = {};
      bool found = JPS::findPath(path, m_path_finder, worldPos.x, worldPos.y, targetPos.x, targetPos.y, 1);
       if (found) {
         if (m_debug) std::cout << currentTime << " " << m_config.AiName << " DEBUG: " << ":" << "CEAIGenericAmbientManager::chooseNewTarget" << ": Found path to tracked target. Queuing: " << path.size() << std::endl;

         for (auto p : path) {
           m_path_waypoints.push_back(glm::vec2(p.x, p.y));
         }
     
         popNextTarget(currentTime);
       } else {
         if (m_debug) std::cout << currentTime << " " << m_config.AiName << " DEBUG: " << ":" << "CEAIGenericAmbientManager::chooseNewTarget" << ": FAILED to find tracked target path... Queue: " << m_path_waypoints.size() << std::endl;
         
         if (m_path_waypoints.empty()) {
           if (m_debug) std::cout << currentTime << " " << m_config.AiName << " DEBUG: " << ":" << "CEAIGenericAmbientManager::chooseNewTarget" << ": Queue empty! No where to go. Run to landing." << std::endl;

           glm::vec2 safePos = m_map->getRandomLanding();
           m_current_target = glm::vec3(safePos.x * m_map->getTileLength(), 0, safePos.y * m_map->getTileLength());
           m_target_expire_time = currentTime + 1.0;
         }
       }

      // Just try once
      return;
    }

    tries++;
  }
}

void CEAIGenericAmbientManager::updateInflightPathsearch(double currentTime)
{
  if (m_path_search_started_at < 0) return;
  
  if (m_debug) std::cout << currentTime << " " << m_config.AiName << " DEBUG: " << " - updateInflightPathsearch() invoked" << std::endl;

  JPS::PathVector path = {};
  auto res = m_path_search_instance->findPathStep(12);

  if (res == JPS_FOUND_PATH) {
    if (m_debug) std::cout << currentTime << " " << m_config.AiName << " DEBUG: " << " - updateInflightPathsearch() JPS_FOUND_PATH" << std::endl;

    // We found a path. Update planned route
    res = m_path_search_instance->findPathFinish(path, 1);
    if (res == JPS_FOUND_PATH) {
      if (m_debug) std::cout << currentTime << " " << m_config.AiName << " DEBUG: " << " - updateInflightPathsearch() JPS_FOUND_PATH received. Mood: " << m_mood << "; points: " << path.size() << std::endl;

      // If I'm angry or afriad then clear eveything and focus on this
      if (m_mood == ANGRY) {
        m_path_waypoints.clear();
      }

      for (auto p : path) {
        m_path_waypoints.push_back(glm::vec2(p.x, p.y));
      }
      
      popNextTarget(currentTime);
    } else {
      // Some memory issue or something - invalidate target
      if (m_debug) std::cout << currentTime << " " << m_config.AiName << " DEBUG: " << " - updateInflightPathsearch() FAILED to find path or some error returned. Keeping existing planned route." << std::endl;
    }
  
    m_path_search_started_at = -1.0;

    return;
  } else if (res == JPS_NEED_MORE_STEPS) {
    // Keep trying
    if (m_debug) std::cout << currentTime << " " << m_config.AiName << " DEBUG: " << " - updateInflightPathsearch() JPS_NEED_MORE_STEPS. Deferring..." << std::endl;

    return;
  }
  
  // Mark target expired so we pick a new one
  if (m_debug) std::cout << currentTime << " " << m_config.AiName << " DEBUG: " << " - updateInflightPathsearch() No path to target found? Keeping existing in case we find target" << std::endl;
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
  
  bool lostTarget = (m_mood == ANGRY && currentTime - m_danger_last_spotted_at > DEFAULT_LOST_TARGET_GIVEUP_TIME);
  bool angryAndNoRoute = (m_mood == ANGRY && m_path_waypoints.empty());
  bool invalidTarget = (target.x == 0 && target.y == 0);
  bool expired = currentTime > m_target_expire_time;
  bool reachedTarget = glm::length(glm::vec2(currentPosition.x, currentPosition.z) - glm::vec2(m_current_target.x, m_current_target.z)) < (m_map->getTileLength() / 2.f);

  if (lostTarget || angryAndNoRoute) {
    Reset(currentTime);
    target = m_map->getWorldTilePosition(m_current_target);
    invalidTarget = (target.x == 0 && target.y == 0);
  } else if (reachedTarget || expired || invalidTarget) {
    chooseNewTarget(currentPosition, currentTime);
    target = m_map->getWorldTilePosition(m_current_target);
    invalidTarget = (target.x == 0 && target.y == 0);
  }

  // Apply behavior
  auto currentAnimation = m_player_controller->getCurrentAnimation();

  // Apply movement
  if (m_mood == ANGRY) {
    // Set run animation
    if (m_player_controller->getCurrentAnimation() != m_config.RunAnimName) {
      m_player_controller->setNextAnimation(m_config.RunAnimName, true);
    }
    // Speed
    m_player_controller->setTargetSpeed(m_config.m_walk_speed * 1.75f);
    // Decide on attack pattern if needed
    // Move towards the target with attack pattern
    // Decide when to give up on our feelings and return to curious or fear state
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
    if (m_debug) std::cout << currentTime << " " << m_config.AiName << " DEBUG: " << " cannot move; invalid target. Redeploying." << std::endl;
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
  if (m_debug) std::cout << currentTime << " " << m_config.AiName << " DEBUG: " << " - CEAIGenericAmbientManager::Reset() - Reset called. Mood: " << m_mood << std::endl;

  if (m_mood == ANGRY) {
    // Completely forget what we were after
    m_path_waypoints.clear();
    chooseNewTarget(m_player_controller->getPosition(), currentTime);
  }
  
  m_danger_last_spotted_at = 0.0;
  
  m_mood = CURIOUS;
}

void CEAIGenericAmbientManager::ReportNotableEvent(glm::vec3 position, std::string eventType, double currentTime) {
  if (eventType == "PLAYER_ELIMINATED") {
    if (m_debug) std::cout << currentTime << " " << m_config.AiName << " DEBUG: " << " - ReportNotableEvent() - I killed a player. Rejoice and reset." << std::endl;

    Reset(currentTime);
    return;
  }

  float dist = glm::distance(position, m_player_controller->getPosition()) / m_map->getTileLength();
//  if (eventType == "PLAYER_SPOTTED" && dist > m_view_range / 2.f && randomIf(0.1)) {
//    if (m_debug) std::cout << currentTime << " " << m_config.AiName << " DEBUG: " << " - ReportNotableEvent() - player spotted. Decided to ignore it. Dist: " << dist << std::endl;
//    return;
//  };

  // TODO: add to an attention queue with x ms delay in processing and finite space
  if (eventType == "PLAYER_SPOTTED" && (m_mood == CURIOUS)) {
    if (m_debug) std::cout << currentTime << " " << m_config.AiName << " DEBUG: " << " - ReportNotableEvent() - player spotted. Mode is curious. Deciding what to do.." << std::endl;
    //Decide if we are afraid or angry about this
    if (m_max_attack_chance > 0.f || m_min_attack_chance > 0.f) {
      if (dist > m_view_range) {
        m_mood_decision = ESCAPE;
      } else {
        float attackChance = CalculateAttackChance(dist, m_view_range, m_min_attack_chance, m_max_attack_chance);
        m_mood_decision = randomIf(attackChance) ? ATTACK : ESCAPE;
      }
      m_last_attack_decision_at = currentTime;
    } else {
      m_mood_decision = ESCAPE;
    }
    
    m_mood = ANGRY;
    if (m_debug) std::cout << currentTime << " " << m_config.AiName << " DEBUG: " << " - ReportNotableEvent() - DECIDED: " << m_mood << std::endl;
    if (m_mood_decision == ATTACK) {
      SetCurrentTarget(position, currentTime);
    } else if (currentTime - m_last_safe_target_calculation > 12.0) {
      SetCurrentTarget(findSafeTarget(position), currentTime);
      m_last_safe_target_calculation = currentTime;
    }
  }
  
  if (eventType == "PLAYER_SPOTTED" && m_mood == ANGRY)
  {
    if (currentTime - m_danger_last_spotted_at < 2.0 && m_mood_decision == ESCAPE) return;
    if (m_debug) std::cout << currentTime << " " << m_config.AiName << " DEBUG: " << " - ReportNotableEvent() Already angry. Updating angry target." << std::endl;
    // Ensure we stay angry and focused but do not reset any paths
    m_mood = ANGRY;
    m_danger_last_spotted_at = currentTime;
    AIAttackDecision curDec = m_mood_decision;
    //Decide if we are afraid or angry about this
    if ((m_max_attack_chance > 0.f || m_min_attack_chance > 0.f) && curDec == ESCAPE && currentTime - m_last_attack_decision_at > 3.f) {
      if (dist > m_view_range) {
        m_mood_decision = ESCAPE;
      } else {
        float attackChance = CalculateAttackChance(dist, m_view_range, m_min_attack_chance, m_max_attack_chance);
        m_mood_decision = randomIf(attackChance) ? ATTACK : ESCAPE;
        // If we were in escape mode but decide to attack then clear the current plan
        if (m_mood_decision == ATTACK) m_path_waypoints.clear();
      }
      m_last_attack_decision_at = currentTime;
    }

    if (m_mood_decision == ATTACK) {
      SetCurrentTarget(position, currentTime);
    } else if (currentTime - m_last_safe_target_calculation > 3.0) {
      SetCurrentTarget(findSafeTarget(position), currentTime);
      m_last_safe_target_calculation = currentTime;
    }
  }
}

bool CEAIGenericAmbientManager::IsDangerous()
{
  return m_is_dangerous;
}

glm::vec3 CEAIGenericAmbientManager::findSafeTarget(glm::vec3 direction)
{
  if (m_debug) std::cout << glfwGetTime() << " " << m_config.AiName << " DEBUG: " << " [" << m_mood << "] findSafeTarget invoked." << std::endl;

  int tries = 0;
  glm::vec2 curPos = m_player_controller->getWorldPosition();
  glm::vec3 pos = glm::vec3(0.f);
  glm::vec2 check;

  // Calculate the vector towards the provided position (direction)
  glm::vec3 towards = glm::normalize(direction - m_player_controller->getPosition());

  // Sort directions based on their dot product with the negative towards vector (opposite)
  std::vector<glm::vec3> sortedDirections = directions;
  std::sort(sortedDirections.begin(), sortedDirections.end(), [&](const glm::vec3& a, const glm::vec3& b) {
    return glm::dot(-towards, a) > glm::dot(-towards, b);
  });

  for (const auto& dir : sortedDirections) {
    check = curPos + (glm::vec2(dir.x, dir.z) * 32.f);
    JPS::PathVector path = {};
    bool found = JPS::findPath(path, m_path_finder, curPos.x, curPos.y, check.x, check.y, 1);
    if (found && !path.empty()) {
      if (m_debug) std::cout << glfwGetTime() << " " << m_config.AiName << " DEBUG: " << " [" << m_mood << "] findSafeTarget found safe target" << std::endl;

      pos.x = path.back().x * m_map->getTileLength();
      pos.y = 0.f;
      pos.z = path.back().y * m_map->getTileLength();
      
      if (m_debug) std::cout << glfwGetTime() << " " << m_config.AiName << " DEBUG: " << " [" << m_mood << "] findSafeTarget SAFE: " << pos.x << ", " << pos.y << ", " << pos.z << std::endl;

      return pos;
    }
    tries++;
  }

  if (m_debug) std::cout << glfwGetTime() << " " << m_config.AiName << " DEBUG: " << " [" << m_mood << "] findSafeTarget. No target found." << std::endl;

  return direction; // Return the original position if no safe target is found
}


bool CEAIGenericAmbientManager::SetCurrentTarget(glm::vec3 targetPosition, double currentTime) {
  if (m_debug) std::cout << currentTime << " " << m_config.AiName << " DEBUG: " << " [" << m_mood << "] SetCurrentTarget invoked." << std::endl;
  float tileSize = m_map->getTileLength();

  glm::vec2 tileCoords = glm::vec2(int(targetPosition.x / tileSize), int(targetPosition.z / tileSize));
  auto distMoved = glm::distance(m_tracked_target, tileCoords);
  
  if (distMoved < 2) {
    if (m_debug) std::cout << currentTime << " " << m_config.AiName << " DEBUG: " << " - CEAIGenericAmbientManager::SetCurrentTarget - Already tracked target matched given target. Returning TRUE" << std::endl;
    return true;
  } else {
    if (m_debug) std::cout << currentTime << " " << m_config.AiName << " DEBUG: " << " - CEAIGenericAmbientManager::SetCurrentTarget: current tracked target differs from new" << std::endl;
  }
  
  auto worldPos = m_player_controller->getWorldPosition();

  JPS::PathVector path = {};
  // WARNING: init will abort any active search
  if (m_debug) std::cout << currentTime << " " << m_config.AiName << " DEBUG: " << "- SetCurrentTarget() - Aborting any inflight search to find new target" << std::endl;
  auto res = m_path_search_instance->findPathInit(JPS::Pos(worldPos.x, worldPos.y), JPS::Pos(tileCoords.x, tileCoords.y));
  
  bool found = false;
  if (res == JPS_FOUND_PATH) {
    // Greedy algo already found a path
    res = m_path_search_instance->findPathFinish(path, 1);
    if (res == JPS_FOUND_PATH) {
      if (m_debug) std::cout << currentTime << " " << m_config.AiName << " DEBUG: " << "- SetCurrentTarget(): GREEDY Found path to target. Updating with waypoints: " << path.size() << std::endl;

      for (auto p : path) {
        m_path_waypoints.push_back(glm::vec2(p.x, p.y));
      }

      popNextTarget(currentTime);
      found = true;
    }
    m_path_search_started_at = -1.0;
  } else if (res == JPS_NEED_MORE_STEPS) {
    // Otherwise, defer to future frames
    if (m_debug) std::cout << currentTime << " " << m_config.AiName << " DEBUG: " << "- SetCurrentTarget(): JPS_NEED_MORE_STEPS. Deferring to furture frames" << std::endl;
    m_path_search_started_at = currentTime;
    found = true;
  }
  
  m_tracked_target = tileCoords;
  
  return found;
}

bool CEAIGenericAmbientManager::NoticesLocalPlayer(std::shared_ptr<CELocalPlayerController> localPlayer) { 
  float dist = glm::distance(localPlayer->getPosition(), m_player_controller->getPosition());
  if (dist < m_view_range * m_map->getTileLength()) return true;
  
  return false;
}

std::shared_ptr<CERemotePlayerController> CEAIGenericAmbientManager::GetPlayerController()
{
  return m_player_controller;
}

float CEAIGenericAmbientManager::CalculateAttackChance(float distance, float maxDist, float minAttackChance, float maxAttackChance) {
    if (distance > maxDist) {
        return 0.0f; // No chance of attack if out of range, indicating FEAR.
    }

    // Linear interpolation between min and max attack chance
    float attackChance = minAttackChance + (maxAttackChance - minAttackChance) * (distance / maxDist);
    return attackChance;
}
