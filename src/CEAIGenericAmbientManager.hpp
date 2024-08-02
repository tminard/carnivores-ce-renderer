#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

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

    double m_last_process_time;
    double m_target_expire_time;
    glm::vec3 m_current_target;
    glm::vec3 m_previous_position;
    double m_stuck_timer;

    std::unordered_map<glm::ivec2, int, std::hash<glm::ivec2>> m_visit_map;

    bool isTilePassable(glm::vec2 tile, const glm::vec3& currentForward, const glm::vec3& potentialForward);
    void chooseNewTarget(glm::vec3 currentPosition, bool getUnstuck, double timeDelta);
    float calculateEnergyCost(glm::vec3 currentPos, glm::vec3 targetPos);
  void updateLookAtDirection(glm::vec3 desiredLookAt, float deltaTime);

public:
    CEAIGenericAmbientManager(AIGenericAmbientManagerConfig config, std::shared_ptr<CERemotePlayerController> playerController, std::shared_ptr<C2MapFile> map, std::shared_ptr<C2MapRscFile> rsc);
    void Process(double currentTime);
};
