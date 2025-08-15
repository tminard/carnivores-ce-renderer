#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>

#include "CEWalkableTerrainPathFinder.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <nlohmann/json.hpp>
#include "jps.hpp"

class CERemotePlayerController;
class CELocalPlayerController;
class C2MapFile;
class C2MapRscFile;

using json = nlohmann::json;

struct AIGenericAmbientManagerConfig {
  std::string AiName;
  std::string WalkAnimName;
  std::vector<std::string> IdleAnimNames;
  std::string RunAnimName;
  // Odds we choose to idle
  float m_idle_odds;
  // Range in tiles when selecting next destination
  float m_pf_range;
  // Walk speed factor
  float m_walk_speed;
};

enum AIGenericMood {
  CURIOUS,
  ANGRY
};

enum AIAttackDecision {
  ESCAPE,
  ATTACK
};

class CEAIGenericAmbientManager {
  const float DEFAULT_VIEW_RANGE = 60.f;
  const float DEFAULT_MIN_ATTACK = 0.3f;
  const float DEFAULT_MAX_ATTACK = 1.f;
  const float DEFAULT_IS_DANGER = true;
  const double DEFAULT_LOST_TARGET_GIVEUP_TIME = 3.0;
  
  // Predefined directions for cardinal and intercardinal directions
  const std::vector<glm::vec3> directions = {
    glm::vec3(1, 0, 0),   // East
    glm::vec3(1, 0, 1),   // Northeast
    glm::vec3(0, 0, 1),   // North
    glm::vec3(-1, 0, 1),  // Northwest
    glm::vec3(-1, 0, 0),  // West
    glm::vec3(-1, 0, -1), // Southwest
    glm::vec3(0, 0, -1),  // South
    glm::vec3(1, 0, -1)   // Southeast
  };

  std::shared_ptr<CERemotePlayerController> m_player_controller;
  std::shared_ptr<C2MapFile> m_map;
  std::shared_ptr<C2MapRscFile> m_rsc;
  
  AIGenericAmbientManagerConfig m_config;
  
  CEWalkableTerrainPathFinder m_path_finder;
  std::unique_ptr< JPS::Searcher<CEWalkableTerrainPathFinder> > m_path_search_instance;
  
  double m_last_process_time;
  double m_target_expire_time;
  double m_last_upload_time = 0;
  double m_last_idle_time = 0;
  double m_path_search_started_at = -1.0;
  
  float m_view_range = DEFAULT_VIEW_RANGE;
  
  // Chance of attack behavior the closer to get to max view range
  float m_max_attack_chance = DEFAULT_MAX_ATTACK;
  // Chance of attack behavior the closer you get to the target
  float m_min_attack_chance = DEFAULT_MIN_ATTACK;
  
  bool m_is_dangerous = DEFAULT_IS_DANGER;
  
  AIGenericMood m_mood = CURIOUS;
  AIAttackDecision m_mood_decision = ESCAPE;
  double m_last_attack_decision_at = 0.0;
  double m_last_safe_target_calculation = 0.0;
  
  glm::vec3 m_current_target;
  glm::vec2 m_tracked_target;
  double m_danger_last_spotted_at = 0.0;
  bool m_debug = false;
  
  // Smooth target transition variables
  glm::vec3 m_interpolated_target;
  glm::vec3 m_previous_target;
  double m_target_transition_start = -1.0;
  double m_target_transition_duration = 0.8f;  // Default transition time
  bool m_is_transitioning = false;
  
  // Attack target smoothing variables
  glm::vec3 m_last_attack_target_position = glm::vec3(0.f);
  double m_last_attack_target_update = 0.0;
  const double m_attack_target_update_cooldown = 0.5; // Minimum time between attack target updates
  const float m_attack_target_distance_threshold = 3.0f; // Minimum distance player must move to update target
  
  // Dynamic speed variation variables
  float m_current_speed_multiplier = 1.0f;
  float m_base_urgency = 1.0f;
  double m_last_speed_update = 0.0;
  const double m_speed_update_interval = 0.1; // Update speed calculations every 100ms
  
  std::vector<glm::vec2> m_path_waypoints = {};
  
  void chooseNewTarget(glm::vec3 currentPosition, double currentTime);
  glm::vec2 popNextTarget(double currentTime);
  void initiateTargetTransition(glm::vec3 newTarget, double currentTime, bool forceImmediate = false);
  bool shouldUpdateAttackTarget(glm::vec3 playerPosition, double currentTime);
  float calculateTerrainDifficulty(glm::vec3 position);
  float calculateUrgencyMultiplier(double currentTime);
  void updateDynamicSpeed(double currentTime);
  
  bool isIdleAnimation(std::string animationName);
  std::string chooseIdleAnimation();
  
  void updateInflightPathsearch(double currentTime);
  float CalculateAttackChance(float distance, float maxDist, float minAttackChance, float maxAttackChance);
  
  glm::vec3 findSafeTarget(glm::vec3 direction);

public:
  CEAIGenericAmbientManager(json jsonConfig, std::shared_ptr<CERemotePlayerController> playerController, std::shared_ptr<C2MapFile> map, std::shared_ptr<C2MapRscFile> rsc);
  void Process(double currentTime);
  bool SetCurrentTarget(glm::vec3 position, double currentTime);
  void Reset(double currentTime);
  void ReportNotableEvent(glm::vec3 position, std::string eventType, double currentTime);
  bool NoticesLocalPlayer(std::shared_ptr<CELocalPlayerController> localPlayer);
  bool NoticesPlayerFromSensory(double currentTime);
  bool IsDangerous();
  std::shared_ptr<CERemotePlayerController> GetPlayerController();
};
