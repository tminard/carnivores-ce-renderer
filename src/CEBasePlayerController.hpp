#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <utility>

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

// Hash function for std::pair<int, int>
namespace std {
    template<>
    struct hash<std::pair<int, int>> {
        std::size_t operator()(const std::pair<int, int>& p) const {
            return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 1);
        }
    };
}

class C2MapFile;
class C2MapRscFile;
struct Camera;

/**
 * Sensory data structure that controllers can access
 * Provides filtered information based on what the controller can actually "sense"
 */
struct SensoryData {
    // Visual information (limited by view range and line of sight)
    struct VisualContact {
        glm::vec3 position;
        glm::vec3 velocity;
        std::string entityType;  // "player", "ai", "item", etc.
        float distance;
        float lastSeenTime;
        bool currentlyVisible;
    };
    
    // Audio information (limited by audio range)
    struct AudioContact {
        glm::vec3 position;
        std::string soundType;  // "footsteps", "gunshot", "roar", etc.
        float volume;
        float distance;
        double timestamp;
    };
    
    // Terrain and environmental awareness
    struct TerrainInfo {
        float height;
        float slope;
        std::string terrainType;  // "grass", "water", "rock", etc.
        bool walkable;
    };
    
    // Current sensory state
    glm::vec3 selfPosition;
    glm::vec3 selfVelocity;
    float selfHealth;
    
    // Detected contacts
    std::vector<VisualContact> visualContacts;
    std::vector<AudioContact> audioContacts;
    
    // Local terrain knowledge (limited radius around controller)
    std::unordered_map<std::pair<int, int>, TerrainInfo> knownTerrain;
    
    // Memory of previously seen areas (for AI persistence)
    std::vector<glm::vec3> rememberedPlayerPositions;
    std::vector<double> rememberedPlayerTimes;
};

/**
 * Base class for all player controllers in the game
 * Provides common interface and sensory system for fog of war mechanics
 */
class CEBasePlayerController {
protected:
    std::shared_ptr<C2MapFile> m_map;
    std::shared_ptr<C2MapRscFile> m_rsc;
    
    // Sensory capabilities (configurable per controller type)
    float m_viewRange = 60.0f;           // How far this controller can see
    float m_audioRange = 80.0f;          // How far this controller can hear
    float m_terrainAwarenessRadius = 20.0f; // Radius of known terrain
    bool m_hasGodView = false;           // Whether this controller has unrestricted access
    
    // Cached sensory data
    SensoryData m_sensoryData;
    double m_lastSensoryUpdate = 0.0;
    const double m_sensoryUpdateInterval = 0.1; // Update sensory data every 100ms

public:
    CEBasePlayerController(std::shared_ptr<C2MapFile> map, std::shared_ptr<C2MapRscFile> rsc);
    virtual ~CEBasePlayerController() = default;
    
    // Core interface that all controllers must implement
    virtual glm::vec3 getPosition() const = 0;
    virtual glm::vec2 getWorldPosition() const = 0;
    virtual void setPosition(glm::vec3 position) = 0;
    virtual void setElevation(float elevation) = 0;
    virtual void lookAt(glm::vec3 direction) = 0;
    virtual Camera* getCamera() = 0;
    
    // Update methods
    virtual void update(double currentTime, double deltaTime) = 0;
    
    // Sensory system - provides filtered information based on controller capabilities
    const SensoryData& getSensoryData(double currentTime);
    void updateSensoryData(double currentTime, const std::vector<std::shared_ptr<CEBasePlayerController>>& otherControllers);
    
    // Sensory configuration
    void setSensoryCapabilities(float viewRange, float audioRange, float terrainRadius, bool godView = false);
    float getViewRange() const { return m_viewRange; }
    float getAudioRange() const { return m_audioRange; }
    bool hasGodView() const { return m_hasGodView; }
    
    // Visibility and line-of-sight checking
    bool canSee(const glm::vec3& targetPosition, double currentTime) const;
    bool canHear(const glm::vec3& soundPosition, float soundVolume) const;
    
    // Controller type identification
    virtual std::string getControllerType() const = 0;
    
    // Network/remote controller support
    virtual bool isRemoteController() const { return false; }
    virtual void processRemoteCommand(const std::string& command, const std::vector<float>& parameters) {}

protected:
    // Helper methods for sensory system
    bool isInLineOfSight(const glm::vec3& fromPos, const glm::vec3& toPos) const;
    float calculateVisibilityFactor(const glm::vec3& targetPosition, double currentTime) const;
    void updateVisualContacts(double currentTime, const std::vector<std::shared_ptr<CEBasePlayerController>>& otherControllers);
    void updateTerrainKnowledge(double currentTime);
    void pruneOldMemories(double currentTime);
};