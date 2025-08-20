#include "CEBasePlayerController.hpp"
#include "C2MapFile.h"
#include "C2MapRscFile.h"
#include "camera.h"

#include <algorithm>
#include <cmath>
#include <glm/geometric.hpp>

CEBasePlayerController::CEBasePlayerController(std::shared_ptr<C2MapFile> map, std::shared_ptr<C2MapRscFile> rsc)
    : m_map(map), m_rsc(rsc) {
    // Initialize sensory data
    m_sensoryData.selfPosition = glm::vec3(0.0f);
    m_sensoryData.selfVelocity = glm::vec3(0.0f);
    m_sensoryData.selfHealth = 100.0f;
}

const SensoryData& CEBasePlayerController::getSensoryData(double currentTime) {
    // Update sensory data if enough time has passed
    if (currentTime - m_lastSensoryUpdate >= m_sensoryUpdateInterval) {
        m_sensoryData.selfPosition = getPosition();
        updateTerrainKnowledge(currentTime);
        pruneOldMemories(currentTime);
        m_lastSensoryUpdate = currentTime;
    }
    
    return m_sensoryData;
}

void CEBasePlayerController::updateSensoryData(double currentTime, const std::vector<std::shared_ptr<CEBasePlayerController>>& otherControllers) {
    if (currentTime - m_lastSensoryUpdate < m_sensoryUpdateInterval) {
        return;
    }
    
    // Update self information
    m_sensoryData.selfPosition = getPosition();
    
    // Update visual contacts
    updateVisualContacts(currentTime, otherControllers);
    
    // Update terrain knowledge
    updateTerrainKnowledge(currentTime);
    
    // Clean up old memories
    pruneOldMemories(currentTime);
    
    m_lastSensoryUpdate = currentTime;
}

void CEBasePlayerController::setSensoryCapabilities(float viewRange, float audioRange, float terrainRadius, bool godView) {
    m_viewRange = viewRange;
    m_audioRange = audioRange;
    m_terrainAwarenessRadius = terrainRadius;
    m_hasGodView = godView;
}

bool CEBasePlayerController::canSee(const glm::vec3& targetPosition, double currentTime) const {
    if (m_hasGodView) return true;
    
    glm::vec3 myPosition = getPosition();
    float distance = glm::distance(myPosition, targetPosition);
    
    // Check if within view range
    if (distance > m_viewRange) return false;
    
    // Check line of sight
    return isInLineOfSight(myPosition, targetPosition);
}

bool CEBasePlayerController::canHear(const glm::vec3& soundPosition, float soundVolume) const {
    if (m_hasGodView) return true;
    
    glm::vec3 myPosition = getPosition();
    float distance = glm::distance(myPosition, soundPosition);
    
    // Sound falloff calculation
    float effectiveRange = m_audioRange * soundVolume;
    return distance <= effectiveRange;
}

bool CEBasePlayerController::isInLineOfSight(const glm::vec3& fromPos, const glm::vec3& toPos) const {
    if (!m_map) return true; // No map data, assume clear line of sight
    
    // Simple line-of-sight check using height sampling
    glm::vec3 direction = toPos - fromPos;
    float distance = glm::length(direction);
    direction = glm::normalize(direction);
    
    // Sample points along the line of sight
    int numSamples = std::max(5, static_cast<int>(distance / m_map->getTileLength()));
    float stepSize = distance / numSamples;
    
    float eyeHeight = 1.5f * m_map->getTileLength(); // Approximate eye height
    
    for (int i = 1; i < numSamples; i++) {
        glm::vec3 samplePos = fromPos + direction * (stepSize * i);
        float terrainHeight = m_map->getHeightAtWorldPosition(samplePos);
        float lineHeight = fromPos.y + (direction.y * stepSize * i);
        
        // If terrain blocks the line of sight
        if (terrainHeight + eyeHeight > lineHeight) {
            return false;
        }
    }
    
    return true;
}

float CEBasePlayerController::calculateVisibilityFactor(const glm::vec3& targetPosition, double currentTime) const {
    glm::vec3 myPosition = getPosition();
    float distance = glm::distance(myPosition, targetPosition);
    
    // Distance factor (closer = more visible)
    float distanceFactor = 1.0f - (distance / m_viewRange);
    distanceFactor = std::max(0.0f, distanceFactor);
    
    // Line of sight factor
    float loseFactor = isInLineOfSight(myPosition, targetPosition) ? 1.0f : 0.0f;
    
    // TODO: Add factors for:
    // - Time of day (night reduces visibility)
    // - Weather conditions
    // - Target movement speed (moving targets easier to spot)
    // - Terrain type (camouflage effects)
    
    return distanceFactor * loseFactor;
}

void CEBasePlayerController::updateVisualContacts(double currentTime, const std::vector<std::shared_ptr<CEBasePlayerController>>& otherControllers) {
    m_sensoryData.visualContacts.clear();
    
    glm::vec3 myPosition = getPosition();
    
    for (const auto& otherController : otherControllers) {
        if (otherController.get() == this) continue; // Skip self
        
        glm::vec3 otherPosition = otherController->getPosition();
        float distance = glm::distance(myPosition, otherPosition);
        
        // Check if within view range and visible
        if (distance <= m_viewRange && canSee(otherPosition, currentTime)) {
            SensoryData::VisualContact contact;
            contact.position = otherPosition;
            contact.entityType = otherController->getControllerType();
            contact.distance = distance;
            contact.lastSeenTime = currentTime;
            contact.currentlyVisible = true;
            
            // TODO: Calculate velocity based on position history
            contact.velocity = glm::vec3(0.0f);
            
            m_sensoryData.visualContacts.push_back(contact);
            
            // Remember player positions for AI persistence
            if (contact.entityType == "local_player") {
                m_sensoryData.rememberedPlayerPositions.push_back(otherPosition);
                m_sensoryData.rememberedPlayerTimes.push_back(currentTime);
            }
        }
    }
}

void CEBasePlayerController::updateTerrainKnowledge(double currentTime) {
    if (!m_map) return;
    
    glm::vec3 myPosition = getPosition();
    glm::vec2 myWorldPos = getWorldPosition();
    
    float tileSize = m_map->getTileLength();
    int radiusInTiles = static_cast<int>(m_terrainAwarenessRadius / tileSize);
    
    // Update terrain knowledge in a radius around the controller
    for (int dx = -radiusInTiles; dx <= radiusInTiles; dx++) {
        for (int dz = -radiusInTiles; dz <= radiusInTiles; dz++) {
            std::pair<int, int> tilePos(myWorldPos.x + dx, myWorldPos.y + dz);
            
            // Check if tile is within awareness radius
            glm::vec2 tileCenterWorld(tilePos.first * tileSize, tilePos.second * tileSize);
            glm::vec2 myPosWorld(myPosition.x, myPosition.z);
            float distanceToTile = glm::distance(tileCenterWorld, myPosWorld);
            
            if (distanceToTile <= m_terrainAwarenessRadius) {
                SensoryData::TerrainInfo terrainInfo;
                
                // Sample terrain height and properties
                if (tilePos.first >= 0 && tilePos.first < m_map->getWidth() && 
                    tilePos.second >= 0 && tilePos.second < m_map->getHeight()) {
                    
                    int tileIndex = (m_map->getWidth() * tilePos.second) + tilePos.first;
                    terrainInfo.height = m_map->getHeightAt(tileIndex);
                    terrainInfo.walkable = true; // TODO: Implement walkability checks
                    terrainInfo.terrainType = "unknown"; // TODO: Get terrain type from map
                    
                    // Calculate slope (simplified)
                    terrainInfo.slope = 0.0f; // TODO: Calculate actual slope
                    
                    m_sensoryData.knownTerrain[tilePos] = terrainInfo;
                }
            }
        }
    }
}

void CEBasePlayerController::pruneOldMemories(double currentTime) {
    const double memoryRetentionTime = 30.0; // Remember positions for 30 seconds
    
    // Prune old remembered player positions
    auto it = m_sensoryData.rememberedPlayerTimes.begin();
    auto posIt = m_sensoryData.rememberedPlayerPositions.begin();
    
    while (it != m_sensoryData.rememberedPlayerTimes.end()) {
        if (currentTime - *it > memoryRetentionTime) {
            it = m_sensoryData.rememberedPlayerTimes.erase(it);
            posIt = m_sensoryData.rememberedPlayerPositions.erase(posIt);
        } else {
            ++it;
            ++posIt;
        }
    }
    
    // Mark visual contacts as not currently visible if they haven't been updated
    for (auto& contact : m_sensoryData.visualContacts) {
        if (currentTime - contact.lastSeenTime > m_sensoryUpdateInterval * 2) {
            contact.currentlyVisible = false;
        }
    }
}