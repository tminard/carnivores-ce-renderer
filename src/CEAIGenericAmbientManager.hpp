#pragma once

#include <stdio.h>
#include <memory>
#include <vector>
#include <string>
#include <glm/glm.hpp>

class CERemotePlayerController;
class C2MapFile;
class C2MapRscFile;

struct AIGenericAmbientManagerConfig {
  std::string WalkAnimName;
};

class CEAIGenericAmbientManager {
  std::shared_ptr<CERemotePlayerController> m_player_controller;
  std::shared_ptr<C2MapFile> m_map;
  std::shared_ptr<C2MapRscFile> m_rsc;
  
  std::vector<glm::vec2> m_planned_route;

  AIGenericAmbientManagerConfig m_config;
  
  double m_last_process_time;

  double m_target_expire_time;
  glm::vec3 m_current_target;
  
  bool isTilePassable(glm::vec2 tile);
  
  /**
   A single tick to add an item to the planned route.
   Only consider tiles I could theoretically "see".
   
   Also use to plan escape if needed.
   */
  void planNextRouteTick();
public:
  CEAIGenericAmbientManager(AIGenericAmbientManagerConfig config, std::shared_ptr<CERemotePlayerController> playerController, std::shared_ptr<C2MapFile> map, std::shared_ptr<C2MapRscFile> rsc);
  
  void Process(double currentTime);
};
