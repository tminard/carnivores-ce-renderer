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
  FEAR,
  ANGRY
};

class CEAIGenericAmbientManager {
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
  
  AIGenericMood m_mood = CURIOUS;
  
  glm::vec3 m_current_target;
  glm::vec2 m_tracked_target;
  
  std::vector<glm::vec2> m_path_waypoints = {};
  
  void chooseNewTarget(glm::vec3 currentPosition, double currentTime);
  glm::vec2 popNextTarget(double currentTime);
  
  bool isIdleAnimation(std::string animationName);
  std::string chooseIdleAnimation();
  
  void updateInflightPathsearch(double currentTime);
  
public:
  CEAIGenericAmbientManager(json jsonConfig, std::shared_ptr<CERemotePlayerController> playerController, std::shared_ptr<C2MapFile> map, std::shared_ptr<C2MapRscFile> rsc);
  void Process(double currentTime);
  bool SetCurrentTarget(glm::vec3 position, double currentTime);
  void Reset(double currentTime);
  void ReportNotableEvent(glm::vec3 position, std::string eventType, double currentTime);
};
