#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

#include "CEWalkableTerrainPathFinder.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

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

    AIGenericAmbientManagerConfig m_config;
  
    CEWalkableTerrainPathFinder m_path_finder;

    double m_last_process_time;
    double m_target_expire_time;
    double m_last_upload_time = 0;
    glm::vec3 m_current_target;
  
    std::vector<glm::vec2> m_path_waypoints = {};

    bool isTilePassable(glm::vec2 tile, const glm::vec3& currentForward, const glm::vec3& potentialForward);
    bool isTileSafe(glm::vec2 tile);
    void chooseNewTarget(glm::vec3 currentPosition, double currentTime);

public:
    CEAIGenericAmbientManager(AIGenericAmbientManagerConfig config, std::shared_ptr<CERemotePlayerController> playerController, std::shared_ptr<C2MapFile> map, std::shared_ptr<C2MapRscFile> rsc);
    void Process(double currentTime);
};
