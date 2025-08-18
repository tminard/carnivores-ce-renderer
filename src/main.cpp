//
//  main.cpp
//  CE Character Lab
//
//  Created by Tyler Minard on 8/6/15.
//  Copyright (c) 2015 Tyler Minard. All rights reserved.
//
#define GLM_ENABLE_EXPERIMENTAL
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/common.hpp>

#include <iostream>
#include <fstream>
#include <filesystem>
#include <iomanip>
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "C2CarFile.h"
#include "C2CarFilePreloader.h"

#include "Hunt.h"
#include "CEGeometry.h"
#include "CETexture.h"

#include "CEObservable.hpp"

#include "g_shared.h"
#include <chrono>

#include "C2MapFile.h"
#include "C2MapRscFile.h"

#include "CEWorldModel.h"

#include "TerrainRenderer.h"

#include "LocalInputManager.hpp"
#include "LocalVideoManager.hpp"
#include "LocalAudioManager.hpp"

#include "CELocalPlayerController.hpp"
#include "CERemotePlayerController.hpp"
#include "CEAIGenericAmbientManager.hpp"

#include "C2Sky.h"

#include <OpenAL/al.h>
#include <OpenAL/alc.h>

#include <mutex>
#include <chrono>

#include <nlohmann/json.hpp>

#include "CEAnimation.h"
#include "CEShadowManager.h"
#include "CEUIRenderer.h"
#include "CEBulletProjectileManager.h"
#include "CEPhysicsWorld.h"
#include "CESimpleGeometry.h"
#include "CECapsuleCollision.h"
#include "vertex.h"

// ImGui includes
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

namespace fs = std::filesystem;
using json = nlohmann::json;

struct ConfigSpawn {
  json data;
  std::string file;
  std::string animation;
  std::vector<int> position;
  std::string aiControllerName;
};

struct ImpactEvent {
  glm::vec3 location;
  std::string surfaceType;
  float distance;
  float damage;
  double timestamp;
  std::string impactType; // "Contact Manifold", "Raycast", "Ground", "Face Intersection", etc.
  std::string objectName; // For object hits, store the object name
  int objectIndex; // For object hits, store the object index
  int instanceIndex; // For object hits, store the instance index
  
  // Face intersection specific data
  glm::vec3 surfaceNormal; // Surface normal at hit point
  glm::vec3 incomingDirection; // Bullet direction when hitting
  int tileX, tileZ; // For terrain hits - which heightfield tile
  int faceIndex; // Which triangle/face was hit (when available)
};

struct VisualImpactMarker {
  glm::vec3 position;
  glm::vec3 color;  // Different colors for different impact types
  double spawnTime;
  std::string surfaceType;
  float scale;
};

struct ProjectileTraceLine {
  glm::vec3 startPoint;
  glm::vec3 endPoint;
  glm::vec3 color;
  double spawnTime;
  std::string surfaceType;
  std::string collisionMethod;
  float thickness;
};

struct DebugSphere {
  glm::vec3 position;
  float radius;
  glm::vec3 color;
  std::string label;
  double spawnTime;
};

struct ProjectileTrajectory {
  std::vector<glm::vec3> points;
  glm::vec3 color;
  double spawnTime;
  float thickness;
  std::string surfaceType;
};

// Sphere generation utility for impact markers
std::vector<Vertex> generateSphere(float radius, int segments) {
  std::vector<Vertex> vertices;
  
  for (int i = 0; i <= segments; i++) {
    float phi = M_PI * i / segments;
    for (int j = 0; j <= segments; j++) {
      float theta = 2.0f * M_PI * j / segments;
      
      float x = radius * sin(phi) * cos(theta);
      float y = radius * cos(phi);
      float z = radius * sin(phi) * sin(theta);
      
      float u = (float)j / segments;
      float v = (float)i / segments;
      
      Vertex vertex(glm::vec3(x, y, z), glm::vec2(u, v), glm::vec3(x, y, z), false, 1.0f, 0, 0); // position, UV, normal, hidden, alpha, owner, flags
      vertices.push_back(vertex);
    }
  }
  
  return vertices;
}

// Line generation utility for trace lines
std::vector<Vertex> generateLine(const glm::vec3& start, const glm::vec3& end, float thickness) {
  std::vector<Vertex> vertices;
  
  // Create a simple thick line using a thin box/cylinder approach
  // For simplicity, we'll create a line as a stretched cube
  glm::vec3 direction = glm::normalize(end - start);
  float length = glm::distance(start, end);
  
  // Create 8 vertices for a thin box (line with thickness)
  float halfThickness = thickness * 0.5f;
  
  // Calculate perpendicular vectors to create thickness
  glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
  if (glm::abs(glm::dot(direction, up)) > 0.9f) {
    up = glm::vec3(1.0f, 0.0f, 0.0f); // Use different up if direction is too close to Y
  }
  
  glm::vec3 right = glm::normalize(glm::cross(direction, up));
  up = glm::normalize(glm::cross(right, direction));
  
  // Create vertices for the line box
  glm::vec3 offsets[8] = {
    glm::vec3(-halfThickness, -halfThickness, 0.0f),
    glm::vec3( halfThickness, -halfThickness, 0.0f),
    glm::vec3( halfThickness,  halfThickness, 0.0f),
    glm::vec3(-halfThickness,  halfThickness, 0.0f),
    glm::vec3(-halfThickness, -halfThickness, length),
    glm::vec3( halfThickness, -halfThickness, length),
    glm::vec3( halfThickness,  halfThickness, length),
    glm::vec3(-halfThickness,  halfThickness, length)
  };
  
  // Transform offsets to world space
  for (int i = 0; i < 8; i++) {
    glm::vec3 localPos = right * offsets[i].x + up * offsets[i].y + direction * offsets[i].z;
    glm::vec3 worldPos = start + localPos;
    
    Vertex vertex(worldPos, glm::vec2(0.0f), direction, false, 1.0f, 0, 0);
    vertices.push_back(vertex);
  }
  
  // Add indices to form triangles for the line box (12 triangles, 36 vertices)
  // For simplicity, we'll create a basic line using just 2 vertices
  vertices.clear();
  Vertex startVertex(start, glm::vec2(0.0f), direction, false, 1.0f, 0, 0);
  Vertex endVertex(end, glm::vec2(1.0f), direction, false, 1.0f, 0, 0);
  vertices.push_back(startVertex);
  vertices.push_back(endVertex);
  
  return vertices;
}

// Global impact tracking
std::vector<ImpactEvent> recentImpacts;
std::vector<VisualImpactMarker> visualImpactMarkers;
std::vector<ProjectileTraceLine> projectileTraceLines;
std::vector<DebugSphere> debugSpheres;
std::vector<ProjectileTrajectory> projectileTrajectories;
const size_t MAX_IMPACT_HISTORY = 10;
const double IMPACT_MARKER_LIFETIME = 10.0; // Show markers for 10 seconds
const double TRACE_LINE_LIFETIME = 8.0; // Show trace lines for 8 seconds
const double DEBUG_SPHERE_LIFETIME = 10.0; // Show debug spheres for 10 seconds
const double TRAJECTORY_LIFETIME = 60.0; // Show trajectories for 60 seconds

// Impact marker rendering resources
std::unique_ptr<CESimpleGeometry> impactMarkerGeometry;
std::vector<glm::mat4> impactMarkerTransforms;
std::vector<glm::vec3> impactMarkerColors;

// Trace line rendering resources
std::unique_ptr<CESimpleGeometry> traceLineGeometry;
std::vector<glm::mat4> traceLineTransforms;
std::vector<glm::vec3> traceLineColors;

// Update impact markers: clean up old ones and prepare transforms for rendering
void updateImpactMarkers(double currentTime) {
  // Remove expired markers
  size_t beforeCount = visualImpactMarkers.size();
  visualImpactMarkers.erase(
    std::remove_if(visualImpactMarkers.begin(), visualImpactMarkers.end(),
      [currentTime](const VisualImpactMarker& marker) {
        return (currentTime - marker.spawnTime) > IMPACT_MARKER_LIFETIME;
      }), 
    visualImpactMarkers.end());
  
  // Debug: Log marker count changes
  if (beforeCount != visualImpactMarkers.size()) {
    std::cout << "🗑️ Cleaned up markers: " << beforeCount << " -> " << visualImpactMarkers.size() << std::endl;
  }
  
  // Clear previous frame data
  impactMarkerTransforms.clear();
  impactMarkerColors.clear();
  
  // Build transforms and colors for each active marker
  for (const auto& marker : visualImpactMarkers) {
    // Create transform matrix: scale by marker size and translate to position
    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::translate(transform, marker.position);
    
    // Fade out markers as they age for better visual effect
    double age = currentTime - marker.spawnTime;
    float fadeAlpha = 1.0f - (float)(age / IMPACT_MARKER_LIFETIME);
    fadeAlpha = glm::clamp(fadeAlpha, 0.0f, 1.0f);
    
    // Scale marker based on age - start small and grow, then fade
    float ageScale = std::min(1.0f, (float)(age * 2.0)); // Grow quickly in first 0.5 seconds
    transform = glm::scale(transform, glm::vec3(marker.scale * ageScale));
    
    impactMarkerTransforms.push_back(transform);
    
    // Store color with fade alpha
    glm::vec3 fadedColor = marker.color * fadeAlpha;
    impactMarkerColors.push_back(fadedColor);
  }
}

// Update trace lines: clean up old ones and prepare transforms for rendering
void updateTraceLines(double currentTime) {
  // Remove expired trace lines
  size_t beforeCount = projectileTraceLines.size();
  projectileTraceLines.erase(
    std::remove_if(projectileTraceLines.begin(), projectileTraceLines.end(),
      [currentTime](const ProjectileTraceLine& traceLine) {
        return (currentTime - traceLine.spawnTime) > TRACE_LINE_LIFETIME;
      }),
    projectileTraceLines.end());
  
  // Remove expired debug spheres
  size_t beforeSphereCount = debugSpheres.size();
  debugSpheres.erase(
    std::remove_if(debugSpheres.begin(), debugSpheres.end(),
      [currentTime](const DebugSphere& sphere) {
        return (currentTime - sphere.spawnTime) > DEBUG_SPHERE_LIFETIME;
      }),
    debugSpheres.end());
  
  // Remove expired trajectories
  size_t beforeTrajectoryCount = projectileTrajectories.size();
  projectileTrajectories.erase(
    std::remove_if(projectileTrajectories.begin(), projectileTrajectories.end(),
      [currentTime](const ProjectileTrajectory& trajectory) {
        return (currentTime - trajectory.spawnTime) > TRAJECTORY_LIFETIME;
      }),
    projectileTrajectories.end());
  
  // Debug: Log cleanup changes
  if (beforeCount != projectileTraceLines.size()) {
    std::cout << "🗑️ Cleaned up trace lines: " << beforeCount << " -> " << projectileTraceLines.size() << std::endl;
  }
  if (beforeSphereCount != debugSpheres.size()) {
    std::cout << "🗑️ Cleaned up debug spheres: " << beforeSphereCount << " -> " << debugSpheres.size() << std::endl;
  }
  if (beforeTrajectoryCount != projectileTrajectories.size()) {
    std::cout << "🗑️ Cleaned up trajectories: " << beforeTrajectoryCount << " -> " << projectileTrajectories.size() << std::endl;
  }
  
  // Clear previous frame data
  traceLineTransforms.clear();
  traceLineColors.clear();
  
  // Build transforms and colors for each active trace line
  for (const auto& traceLine : projectileTraceLines) {
    // For lines, we'll use the identity transform since the line geometry already contains world positions
    glm::mat4 transform = glm::mat4(1.0f);
    
    // Fade out trace lines as they age
    double age = currentTime - traceLine.spawnTime;
    float fadeAlpha = 1.0f - (float)(age / TRACE_LINE_LIFETIME);
    fadeAlpha = glm::clamp(fadeAlpha, 0.0f, 1.0f);
    
    traceLineTransforms.push_back(transform);
    
    // Store color with fade alpha
    glm::vec3 fadedColor = traceLine.color * fadeAlpha;
    traceLineColors.push_back(fadedColor);
  }
}

// Debug UI control
bool showDebugUI = false; // Set to false to disable all ImGui panels for maximum performance

// Forward declarations for external use
extern void addImpactEvent(const glm::vec3& location, const std::string& surfaceType, float distance, float damage, const std::string& impactType);
extern void addImpactEvent(const glm::vec3& location, const std::string& surfaceType, float distance, float damage, const std::string& impactType, const std::string& objectName, int objectIndex, int instanceIndex);
extern void addFaceIntersectionEvent(const glm::vec3& location, const std::string& surfaceType, float distance, const glm::vec3& normal, const glm::vec3& direction, int tileX, int tileZ, int faceIndex, const std::string& objectName, int objectIndex, int instanceIndex);
extern void addProjectileTraceLine(const glm::vec3& startPoint, const glm::vec3& endPoint, const std::string& surfaceType);
extern void addProjectileTraceLineWithMethod(const glm::vec3& startPoint, const glm::vec3& endPoint, const std::string& surfaceType, const std::string& collisionMethod);
extern void addDebugSphere(const glm::vec3& position, float radius, const glm::vec3& color, const std::string& label);
extern void addProjectileTrajectory(const std::vector<glm::vec3>& trajectoryPoints, const std::string& surfaceType);

void addImpactEvent(const glm::vec3& location, const std::string& surfaceType, float distance, float damage, const std::string& impactType) {
  ImpactEvent event;
  event.location = location;
  event.surfaceType = surfaceType;
  event.distance = distance;
  event.damage = damage;
  event.timestamp = glfwGetTime();
  event.impactType = impactType;
  event.objectName = ""; // No object info for basic version
  event.objectIndex = -1;
  event.instanceIndex = -1;
  
  // Initialize face intersection data
  event.surfaceNormal = glm::vec3(0, 1, 0);
  event.incomingDirection = glm::vec3(0, 0, 0);
  event.tileX = -1;
  event.tileZ = -1;
  event.faceIndex = -1;
  
  recentImpacts.insert(recentImpacts.begin(), event);
  if (recentImpacts.size() > MAX_IMPACT_HISTORY) {
    recentImpacts.resize(MAX_IMPACT_HISTORY);
  }
  
  // Create visual impact marker
  VisualImpactMarker marker;
  marker.position = location;
  marker.spawnTime = glfwGetTime();
  marker.surfaceType = surfaceType;
  marker.scale = 0.5f; // Larger for better visibility
  
  // Use dark/muted colors for impact markers (contrast with bright trace lines)
  if (surfaceType == "terrain") {
    marker.color = glm::vec3(0.4f, 0.3f, 0.1f); // DARK BROWN for terrain
  } else if (surfaceType == "water") {
    marker.color = glm::vec3(0.1f, 0.3f, 0.5f); // DARK BLUE for water
  } else if (surfaceType == "object") {
    marker.color = glm::vec3(0.5f, 0.1f, 0.1f); // DARK RED for objects
  } else if (surfaceType == "out-of-bounds") {
    marker.color = glm::vec3(0.3f, 0.3f, 0.3f); // DARK GRAY for out-of-bounds
  } else {
    marker.color = glm::vec3(0.5f, 0.5f, 0.1f); // DARK YELLOW for unknown
  }
  
  visualImpactMarkers.push_back(marker);
}

void addFaceIntersectionEvent(const glm::vec3& location, const std::string& surfaceType, float distance, 
                             const glm::vec3& normal, const glm::vec3& direction, int tileX, int tileZ, int faceIndex,
                             const std::string& objectName, int objectIndex, int instanceIndex) {
  ImpactEvent event;
  event.location = location;
  event.surfaceType = surfaceType;
  event.distance = distance;
  event.damage = 0.0f; // Face intersections don't have damage, just tracking
  event.timestamp = glfwGetTime();
  event.impactType = "Face Intersection";
  event.objectName = objectName;
  event.objectIndex = objectIndex;
  event.instanceIndex = instanceIndex;
  
  // Face intersection specific data
  event.surfaceNormal = normal;
  event.incomingDirection = direction;
  event.tileX = tileX;
  event.tileZ = tileZ;
  event.faceIndex = faceIndex;
  
  recentImpacts.insert(recentImpacts.begin(), event);
  if (recentImpacts.size() > MAX_IMPACT_HISTORY) {
    recentImpacts.resize(MAX_IMPACT_HISTORY);
  }
  
  // Also create visual markers for face intersections
  VisualImpactMarker marker;
  marker.position = location;
  marker.spawnTime = glfwGetTime();
  marker.surfaceType = surfaceType;
  marker.scale = 2.0f;
  
  // Use different colors for different surface types
  if (surfaceType == "terrain") {
    marker.color = glm::vec3(1.0f, 0.8f, 0.0f); // Orange for terrain
  } else if (surfaceType == "object") {
    marker.color = glm::vec3(1.0f, 0.0f, 1.0f); // Magenta for objects
  } else if (surfaceType == "water") {
    marker.color = glm::vec3(0.0f, 0.8f, 1.0f); // Cyan for water
  } else {
    marker.color = glm::vec3(0.5f, 0.5f, 0.5f); // Gray for unknown
  }
  
  visualImpactMarkers.push_back(marker);
  
  std::cout << "🎯 Created face intersection marker at [" << location.x << ", " << location.y << ", " << location.z << "] - " << surfaceType;
  if (surfaceType == "terrain") {
    std::cout << " tile[" << tileX << "," << tileZ << "]";
  } else if (surfaceType == "object") {
    std::cout << " object: " << objectName;
  }
  std::cout << " face: " << faceIndex << std::endl;
}

void addImpactEvent(const glm::vec3& location, const std::string& surfaceType, float distance, float damage, const std::string& impactType, const std::string& objectName, int objectIndex, int instanceIndex) {
  ImpactEvent event;
  event.location = location;
  event.surfaceType = surfaceType;
  event.distance = distance;
  event.damage = damage;
  event.timestamp = glfwGetTime();
  event.impactType = impactType;
  event.objectName = objectName;
  event.objectIndex = objectIndex;
  event.instanceIndex = instanceIndex;
  
  // Initialize face intersection data
  event.surfaceNormal = glm::vec3(0, 1, 0);
  event.incomingDirection = glm::vec3(0, 0, 0);
  event.tileX = -1;
  event.tileZ = -1;
  event.faceIndex = -1;
  
  recentImpacts.insert(recentImpacts.begin(), event);
  if (recentImpacts.size() > MAX_IMPACT_HISTORY) {
    recentImpacts.resize(MAX_IMPACT_HISTORY);
  }
  
  // Create visual impact marker (same logic as basic version)
  VisualImpactMarker marker;
  marker.position = location;
  marker.spawnTime = glfwGetTime();
  marker.surfaceType = surfaceType;
  marker.scale = 0.5f; // Larger for better visibility
  
  // Use dark/muted colors for impact markers (contrast with bright trace lines)
  if (surfaceType == "terrain") {
    marker.color = glm::vec3(0.4f, 0.3f, 0.1f); // DARK BROWN for terrain
  } else if (surfaceType == "water") {
    marker.color = glm::vec3(0.1f, 0.3f, 0.5f); // DARK BLUE for water
  } else if (surfaceType == "object") {
    marker.color = glm::vec3(0.5f, 0.1f, 0.1f); // DARK RED for objects
  } else if (surfaceType == "out-of-bounds") {
    marker.color = glm::vec3(0.3f, 0.3f, 0.3f); // DARK GRAY for out-of-bounds
  } else {
    marker.color = glm::vec3(0.5f, 0.5f, 0.1f); // DARK YELLOW for unknown
  }
  
  visualImpactMarkers.push_back(marker);
}

void addProjectileTraceLine(const glm::vec3& startPoint, const glm::vec3& endPoint, const std::string& surfaceType) {
  addProjectileTraceLineWithMethod(startPoint, endPoint, surfaceType, "legacy");
}

void addProjectileTraceLineWithMethod(const glm::vec3& startPoint, const glm::vec3& endPoint, const std::string& surfaceType, const std::string& collisionMethod) {
  ProjectileTraceLine traceLine;
  traceLine.startPoint = startPoint;
  traceLine.endPoint = endPoint;
  traceLine.spawnTime = glfwGetTime();
  traceLine.surfaceType = surfaceType;
  traceLine.collisionMethod = collisionMethod;
  traceLine.thickness = 4.0f; // Thin but visible line
  
  // Use distinct colors based on collision method and surface type
  if (collisionMethod == "bullet_heightfield") {
    traceLine.color = glm::vec3(0.0f, 1.0f, 0.0f); // BRIGHT GREEN for Bullet heightfield
  } else if (collisionMethod == "legacy_terrain" || collisionMethod == "legacy_terrain_fallback") {
    traceLine.color = glm::vec3(0.0f, 0.0f, 1.0f); // BLUE for legacy terrain
  } else if (collisionMethod == "raycast_water") {
    traceLine.color = glm::vec3(0.0f, 1.0f, 1.0f); // CYAN for water raycast
  } else if (collisionMethod == "raycast_object") {
    traceLine.color = glm::vec3(1.0f, 0.0f, 0.0f); // RED for object raycast
  } else if (collisionMethod == "contact_manifold") {
    traceLine.color = glm::vec3(1.0f, 1.0f, 0.0f); // YELLOW for contact manifold
  } else {
    // Fallback to surface type colors
    if (surfaceType == "terrain") {
      traceLine.color = glm::vec3(0.0f, 0.0f, 1.0f); // BLUE for terrain
    } else if (surfaceType == "water") {
      traceLine.color = glm::vec3(0.0f, 1.0f, 1.0f); // CYAN for water
    } else if (surfaceType == "object") {
      traceLine.color = glm::vec3(1.0f, 0.0f, 0.0f); // RED for objects
    } else if (surfaceType == "out-of-bounds") {
      traceLine.color = glm::vec3(1.0f, 1.0f, 1.0f); // WHITE for out-of-bounds
    } else {
      traceLine.color = glm::vec3(1.0f, 0.0f, 1.0f); // MAGENTA for unknown
    }
  }
  
  projectileTraceLines.push_back(traceLine);
  
  float distance = glm::distance(startPoint, endPoint);
  float distanceMeters = distance / 4.0f; // Convert game units to meters (4 units = 1 meter in scaled world)
  std::cout << "🎯 Created trace line from [" << startPoint.x << ", " << startPoint.y << ", " << startPoint.z 
            << "] to [" << endPoint.x << ", " << endPoint.y << ", " << endPoint.z 
            << "] - " << surfaceType << " (" << collisionMethod << ") (Distance: " << std::fixed << std::setprecision(1) << distanceMeters << "m, Total: " << projectileTraceLines.size() << ")" << std::endl;
}

void addDebugSphere(const glm::vec3& position, float radius, const glm::vec3& color, const std::string& label) {
  DebugSphere sphere;
  sphere.position = position;
  sphere.radius = radius;
  sphere.color = color;
  sphere.label = label;
  sphere.spawnTime = glfwGetTime();
  
  debugSpheres.push_back(sphere);
  
  std::cout << "🔵 Created debug sphere at [" << position.x << ", " << position.y << ", " << position.z 
            << "] - " << label << " (Radius: " << radius << ", Total: " << debugSpheres.size() << ")" << std::endl;
}

void addProjectileTrajectory(const std::vector<glm::vec3>& trajectoryPoints, const std::string& surfaceType) {
  if (trajectoryPoints.size() < 2) return; // Need at least 2 points for a line
  
  ProjectileTrajectory trajectory;
  trajectory.points = trajectoryPoints;
  trajectory.spawnTime = glfwGetTime();
  trajectory.surfaceType = surfaceType;
  trajectory.thickness = 2.0f;
  
  // Use green variations for better visibility
  if (surfaceType == "terrain") {
    trajectory.color = glm::vec3(0.0f, 1.0f, 0.0f); // Bright green for terrain
  } else if (surfaceType == "water") {
    trajectory.color = glm::vec3(0.0f, 0.8f, 0.2f); // Blue-green for water
  } else if (surfaceType == "object") {
    trajectory.color = glm::vec3(0.2f, 1.0f, 0.0f); // Lime green for objects
  } else if (surfaceType == "out-of-bounds") {
    trajectory.color = glm::vec3(0.5f, 1.0f, 0.5f); // Light green for out-of-bounds
  } else {
    trajectory.color = glm::vec3(0.0f, 0.8f, 0.0f); // Dark green for unknown
  }
  
  projectileTrajectories.push_back(trajectory);
  
  float trajectoryLength = 0.0f;
  for (size_t i = 1; i < trajectoryPoints.size(); i++) {
    trajectoryLength += glm::distance(trajectoryPoints[i-1], trajectoryPoints[i]);
  }
  
  float trajectoryLengthMeters = trajectoryLength / 4.0f; // Convert game units to meters (4 units = 1 meter in scaled world)
  std::cout << "🎯 Created trajectory with " << trajectoryPoints.size() << " points - " 
            << surfaceType << " (Length: " << std::fixed << std::setprecision(1) << trajectoryLengthMeters 
            << "m, Total: " << projectileTrajectories.size() << ")" << std::endl;
}

void updateAndRenderImpactMarkers(double currentTime) {
  // Remove expired markers
  visualImpactMarkers.erase(
    std::remove_if(visualImpactMarkers.begin(), visualImpactMarkers.end(),
      [currentTime](const VisualImpactMarker& marker) {
        return (currentTime - marker.spawnTime) > IMPACT_MARKER_LIFETIME;
      }),
    visualImpactMarkers.end()
  );
  
  // For now, just print marker info to console (we'll add 3D rendering later)
  static double lastDebugTime = 0;
  if (currentTime - lastDebugTime > 2.0) { // Print every 2 seconds
    if (!visualImpactMarkers.empty()) {
      std::cout << "Active impact markers: " << visualImpactMarkers.size() << std::endl;
      for (const auto& marker : visualImpactMarkers) {
        double age = currentTime - marker.spawnTime;
        std::cout << "  " << marker.surfaceType << " at (" 
                  << std::fixed << std::setprecision(1)
                  << marker.position.x << ", " << marker.position.y << ", " << marker.position.z 
                  << ") age: " << age << "s" << std::endl;
      }
    }
    lastDebugTime = currentTime;
  }
}

std::unique_ptr<LocalInputManager> input_manager(new LocalInputManager());

template <typename T, typename... Args>
auto make_unique(Args&&... args) -> std::unique_ptr<T>
{
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
  input_manager->cursorPosCallback(window, xpos, ypos);
}

fs::path constructPath(const fs::path& basePath, const std::string& relativePath) {
  return basePath / relativePath;
}

void checkGLError(const std::string& location) {
  GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR) {
    std::cerr << "OpenGL error at " << location << ": " << err << std::endl;
  }
}

void readStencilBuffer(int width, int height) {
  std::vector<GLint> stencilValues(width * height);
  glReadPixels(0, 0, width, height, GL_STENCIL_INDEX, GL_INT, stencilValues.data());
  GLint minStencilValue = *std::min_element(stencilValues.begin(), stencilValues.end());
  GLint maxStencilValue = *std::max_element(stencilValues.begin(), stencilValues.end());
  std::cerr << "Stencil bits: min=" << minStencilValue << " max=" << maxStencilValue << std::endl;
}

const int FPS = 90;
const int frameDelay = 1000 / FPS;

int _fpsCount = 0;
int fps = 0;
double _averageFrameTime = 0.0;

std::chrono::time_point<std::chrono::steady_clock> lastTime = std::chrono::steady_clock::now();

void CalculateFrameRate() {
  auto currentTime = std::chrono::steady_clock::now();
  
  const auto elapsedTime = std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime - lastTime).count();
  double frameTimeMs = elapsedTime / 1000000.0; // Convert to milliseconds
  
  // Update running average
  _averageFrameTime = (_averageFrameTime * _fpsCount + frameTimeMs) / (_fpsCount + 1);
  ++_fpsCount;
  
  if (elapsedTime > 1000000000) {
    lastTime = currentTime;
    fps = _fpsCount;
    double avgFrameTime = 1000.0 / fps;
    _fpsCount = 0;
    _averageFrameTime = 0.0;
  }
}

int main(int argc, const char * argv[])
{
  std::ifstream f("config.json");
  if (!f.is_open()) {
    std::cerr << "Unable to open config.json!" << std::endl;
    return 1;
  }
  
  json data = json::parse(f);
  std::vector<ConfigSpawn> spawns;
  
  bool fullscreen = true;
  
  fs::path basePath = fs::path(data["basePath"].get<std::string>());
  fs::path mapRscPath = constructPath(basePath, data["map"]["rsc"]);
  fs::path mapPath = constructPath(basePath, data["map"]["map"]);
  
  if (data.contains("spawns")) {
    for (const auto& spawnJson : data["spawns"]) {
      ConfigSpawn spawn;
      spawn.data = spawnJson;
      spawn.file = spawnJson.value("file", "");
      spawn.animation = spawnJson.value("animation", "");
      spawn.position = spawnJson.value("position", std::vector<int>{});
      // TODO: temporary. Switch to generic controlled character class from direct remote controller.
      if (spawnJson.contains("attachAI")) {
        if (spawnJson["attachAI"].contains("controller")) {
          spawn.aiControllerName = spawnJson["attachAI"]["controller"];
        }
      }
      spawns.push_back(spawn);
    }
  }
  
  if (data.contains("video") && data["video"].is_object()) {
    if (data["video"].contains("fullscreen") && data["video"]["fullscreen"].is_boolean()) {
      fullscreen = data["video"]["fullscreen"];
    }
  }
  
  // Parse UI configuration
  std::string compassPath;
  if (data.contains("ui") && data["ui"].is_object()) {
    if (data["ui"].contains("compass") && data["ui"]["compass"].is_string()) {
      compassPath = constructPath(basePath, data["ui"]["compass"].get<std::string>()).string();
    }
  }
  
  // Parse weapon configuration
  std::string primaryWeaponPath;
  std::string weaponDrawAnimation = ""; // Will fallback to first animation
  std::string weaponHolsterAnimation = ""; // Will fallback to third animation
  std::string weaponFireAnimation = ""; // Will fallback to second animation
  std::string weaponReloadAnimation = ""; // Will fallback to fourth animation
  
  // Projectile configuration
  float muzzleVelocity = 800.0f;
  glm::vec3 muzzleOffset(0.0f, 0.0f, 0.5f);
  float projectileDamage = 45.0f;
  float weaponRecoil = 1.0f; // Default recoil strength
  float weaponSway = 0.0f; // Default sway strength
  
  // Ammo configuration
  int weaponMaxRounds = 30; // Default ammo capacity
  
  if (data.contains("weapons") && data["weapons"].is_object()) {
    if (data["weapons"].contains("primary") && data["weapons"]["primary"].is_object()) {
      auto& primaryWeapon = data["weapons"]["primary"];
      
      if (primaryWeapon.contains("file") && primaryWeapon["file"].is_string()) {
        primaryWeaponPath = constructPath(basePath, primaryWeapon["file"].get<std::string>()).string();
      }
      
      // Load weapon ammo configuration
      if (primaryWeapon.contains("rounds") && primaryWeapon["rounds"].is_number()) {
        weaponMaxRounds = primaryWeapon["rounds"].get<int>();
      }
      
      if (primaryWeapon.contains("animations") && primaryWeapon["animations"].is_object()) {
        auto& animations = primaryWeapon["animations"];
        if (animations.contains("draw") && animations["draw"].is_string()) {
          weaponDrawAnimation = animations["draw"].get<std::string>();
        }
        if (animations.contains("holster") && animations["holster"].is_string()) {
          weaponHolsterAnimation = animations["holster"].get<std::string>();
        }
        
        if (animations.contains("fire") && animations["fire"].is_string()) {
          weaponFireAnimation = animations["fire"].get<std::string>();
        }
        if (animations.contains("reload") && animations["reload"].is_string()) {
          weaponReloadAnimation = animations["reload"].get<std::string>();
        }
      }
      
      if (primaryWeapon.contains("projectiles") && primaryWeapon["projectiles"].is_object()) {
        auto& projectiles = primaryWeapon["projectiles"];
        if (projectiles.contains("muzzleVelocity") && projectiles["muzzleVelocity"].is_number()) {
          muzzleVelocity = projectiles["muzzleVelocity"].get<float>();
        }
        if (projectiles.contains("muzzleOffset") && projectiles["muzzleOffset"].is_array()) {
          auto offset = projectiles["muzzleOffset"];
          if (offset.size() >= 3) {
            muzzleOffset.x = offset[0].get<float>();
            muzzleOffset.y = offset[1].get<float>();
            muzzleOffset.z = offset[2].get<float>();
          }
        }
        if (projectiles.contains("damage") && projectiles["damage"].is_number()) {
          projectileDamage = projectiles["damage"].get<float>();
        }
        if (projectiles.contains("recoil") && projectiles["recoil"].is_number()) {
          weaponRecoil = projectiles["recoil"].get<float>();
        }
        if (projectiles.contains("sway") && projectiles["sway"].is_number()) {
          weaponSway = projectiles["sway"].get<float>();
        }
      }
    }
  }
  
  std::cout << "Base Path: " << basePath << std::endl;
  std::cout << "Map: " << data["map"]["type"] << std::endl;
  std::cout << "MAP: " << mapPath << std::endl;
  std::cout << "RSC: " << mapRscPath << std::endl;
  std::cout << "Spawns: " << spawns.size() << std::endl;
  if (!compassPath.empty()) {
    std::cout << "Compass: " << compassPath << std::endl;
  }
  if (!primaryWeaponPath.empty()) {
    std::cout << "Primary Weapon: " << primaryWeaponPath << std::endl;
    std::cout << "Draw Animation: " << (weaponDrawAnimation.empty() ? "[first animation]" : weaponDrawAnimation) << std::endl;
    std::cout << "Holster Animation: " << (weaponHolsterAnimation.empty() ? "[third animation]" : weaponHolsterAnimation) << std::endl;
    std::cout << "Fire Animation: " << (weaponFireAnimation.empty() ? "[second animation]" : weaponFireAnimation) << std::endl;
    std::cout << "Reload Animation: " << (weaponReloadAnimation.empty() ? "[fourth animation]" : weaponReloadAnimation) << std::endl;
    std::cout << "Muzzle Velocity: " << muzzleVelocity << " m/s" << std::endl;
    std::cout << "Muzzle Offset: [" << muzzleOffset.x << ", " << muzzleOffset.y << ", " << muzzleOffset.z << "]" << std::endl;
    std::cout << "Projectile Damage: " << projectileDamage << std::endl;
    std::cout << "Weapon Recoil: " << weaponRecoil << std::endl;
    std::cout << "Weapon Sway: " << weaponSway << std::endl;
    std::cout << "Weapon Rounds: " << weaponMaxRounds << std::endl;
  }
  
  auto mapType = CEMapType::C2;
  if (data["map"]["type"] == "C1") {
    mapType = CEMapType::C1;
  }
  
  std::unique_ptr<LocalVideoManager> video_manager = std::make_unique<LocalVideoManager>(fullscreen);
  std::shared_ptr<LocalAudioManager> g_audio_manager = std::make_shared<LocalAudioManager>();
  
  std::shared_ptr<C2MapRscFile> cMapRsc;
  std::shared_ptr<C2MapFile> cMap;
  
  try {
    cMapRsc = std::make_shared<C2MapRscFile>(mapType, mapRscPath.string(), basePath.string());
    cMap = std::make_shared<C2MapFile>(mapType, mapPath.string(), cMapRsc);
  } catch (const std::exception& e) {
    std::cerr << "Error loading map files: " << e.what() << std::endl;
    return 1;
  }
  alDistanceModel(AL_LINEAR_DISTANCE);
  
  std::unique_ptr<TerrainRenderer> terrain = std::make_unique<TerrainRenderer>(cMap, cMapRsc);
  
  // Initialize shadow manager
  std::unique_ptr<CEShadowManager> shadowManager(new CEShadowManager());
  
  // Initialize Bullet Physics projectile manager
  std::unique_ptr<CEBulletProjectileManager> projectileManager;
  
  // Load audio assets
  std::shared_ptr<Sound> die = std::make_shared<Sound>(basePath / "game" / "SOUNDFX" / "HUM_DIE1.WAV");
  std::shared_ptr<CEAudioSource> dieAudioSrc = std::make_shared<CEAudioSource>(die);
  dieAudioSrc->setLooped(false);
  dieAudioSrc->setGain(2.0f);
  dieAudioSrc->setClampDistance(16*6); // Scaled down 16x (was 256*6)
  dieAudioSrc->setMaxDistance(16*80); // Scaled down 16x (was 256*80)
  
  // shared loader to minimize resource usage
  std::unique_ptr<C2CarFilePreloader> cFileLoad(new C2CarFilePreloader);
  
  std::vector<std::shared_ptr<CERemotePlayerController>> characters = {};
  std::vector<std::unique_ptr<CEAIGenericAmbientManager>> ambients = {};
  
  int dCount = 0;
  for (const auto& spawn : spawns) {
    if (dCount < 512) {
      auto character = std::make_shared<CERemotePlayerController>(
                                                                  g_audio_manager,
                                                                  cFileLoad->fetch(spawn.file),
                                                                  cMap,
                                                                  cMapRsc,
                                                                  spawn.animation
                                                                  );
      float spawnHeight = cMap->getPlaceGroundHeight(spawn.position[0], spawn.position[1]);
      glm::vec3 spawnPos = glm::vec3(
                                     (spawn.position[0] * cMap->getTileLength()) + (cMap->getTileLength() / 2),
                                     spawnHeight - 0.75f, // Scaled down 16x (was 12.f)
                                     (spawn.position[1] * cMap->getTileLength()) + (cMap->getTileLength() / 2)
                                     );
      
      character->setPosition(spawnPos);
      
      if (spawn.aiControllerName == "GenericAmbient") {
        auto aiArgs = spawn.data["attachAI"]["args"];
        auto ambientMg = std::make_unique<CEAIGenericAmbientManager>(
                                                                     aiArgs,
                                                                     character,
                                                                     cMap,
                                                                     cMapRsc);
        ambients.push_back(std::move(ambientMg));
      }
      
      characters.push_back(character);
      
      std::cout << "Spawned " << spawn.file << " # " << dCount << " @ [" << spawn.position[0] << "," << spawn.position[1] << "];" << std::endl;
    } else {
      std::cerr << "Failed to spawn CAR: " << spawn.file << " @ [" << spawn.position[0] << "," << spawn.position[1] << "]; max limit of 512 exceeded!" << std::endl;
    }
    dCount++;
  }
  
  GLFWwindow* window = video_manager->GetWindow();
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  
  // Create basic player controller first
  std::shared_ptr<CELocalPlayerController> g_player_controller = std::make_shared<CELocalPlayerController>(cMap->getWidth(), cMap->getHeight(), cMap->getTileLength(), cMap, cMapRsc);
  g_audio_manager->bind(g_player_controller);
  input_manager->Bind(g_player_controller);
  
  glfwSetCursorPosCallback(window, &cursorPosCallback);
  
  Transform g_terrain_transform(glm::vec3(0,0,0), glm::vec3(0,0,0), glm::vec3(1.f, 1.f, 1.f));
  bool render_water, render_sky, render_objects, render_terrain;
  
  render_sky = true;
  render_water = true;
  render_objects = true;
  render_terrain = true;
  
  glfwMakeContextCurrent(window);
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD" << std::endl;
    return -1;
  }
  
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  
  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  
  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 330 core");
  
  std::cout << "ImGui initialized successfully" << std::endl;
  
  // Initialize shadow manager after OpenGL context is ready
  shadowManager->initialize();
  
  // Initialize Bullet Physics projectile manager
  projectileManager = std::make_unique<CEBulletProjectileManager>(cMap.get(), cMapRsc.get(), g_audio_manager.get()); // Re-enabled with performance optimizations
  
  // Initialize player capsule collision component using the physics world
  if (projectileManager && projectileManager->getPhysicsWorld()) {
    btDiscreteDynamicsWorld* dynamicsWorld = projectileManager->getPhysicsWorld()->getDynamicsWorld();
    if (dynamicsWorld) {
      // Create capsule collision component with appropriate dimensions
      auto capsuleCollision = std::make_unique<CECapsuleCollision>(
        dynamicsWorld,
        cMap->getTileLength() * 0.2f,   // Radius: even narrower for tighter collision
        cMap->getTileLength() * 1.0f,   // Height: roughly body height  
        g_player_controller->getPosition()  // Initial position
      );
      
      // Set the capsule collision component on the player controller
      g_player_controller->setCapsuleCollision(std::move(capsuleCollision));
      
      std::cout << "Player capsule collision initialized successfully" << std::endl;
    } else {
      std::cerr << "Warning: Failed to get dynamics world for player collision" << std::endl;
    }
  } else {
    std::cerr << "Warning: Failed to initialize player capsule collision - no physics world available" << std::endl;
  }
    
  // Initialize impact marker geometry (small sphere for collision visualization)
  std::vector<Vertex> sphereVertices = generateSphere(1.0f, 8); // Small sphere, low detail for performance
  
  // Create a simple white texture for markers (16x16 white pixels)
  const int markerTexSize = 16;
  std::vector<uint16_t> whiteTextureData(markerTexSize * markerTexSize, 0xFFFF); // All white pixels
  std::unique_ptr<CETexture> markerTexture = std::make_unique<CETexture>(whiteTextureData, markerTexSize * markerTexSize * 2, markerTexSize, markerTexSize);
  
  impactMarkerGeometry = std::make_unique<CESimpleGeometry>(sphereVertices, std::move(markerTexture));
  
  // Initialize trace line geometry (simple line for trajectory visualization)
  std::vector<Vertex> lineVertices = generateLine(glm::vec3(0, 0, 0), glm::vec3(1, 0, 0), 2.0f); // Simple horizontal line template
  
  // Create a bright colored texture for trace lines (16x16 bright pixels) 
  std::vector<uint16_t> brightTextureData(markerTexSize * markerTexSize, 0xFFFF); // All bright pixels
  std::unique_ptr<CETexture> lineTexture = std::make_unique<CETexture>(brightTextureData, markerTexSize * markerTexSize * 2, markerTexSize, markerTexSize);
  
  traceLineGeometry = std::make_unique<CESimpleGeometry>(lineVertices, std::move(lineTexture));
  
  // Set light direction for more interesting angled lighting (like late afternoon sun)
  shadowManager->setLightDirection(glm::vec3(0.6f, -0.5f, 0.4f));
  
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  
  double lastTime = glfwGetTime();
  double lastRndAudioTime = glfwGetTime();
  
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  //glEnable(GL_CLIP_DISTANCE0);
  glEnable(GL_STENCIL_TEST);
  
  glViewport(0, 0, width, height);
  checkGLError("After viewport setup");
  
  std::shared_ptr<CEAudioSource> m_ambient;
  std::shared_ptr<CEAudioSource> m_random_ambient;
  
  m_ambient = cMapRsc->getAmbientAudio(0);
  g_audio_manager->playAmbient(m_ambient);
  
  m_ambient.reset();
  
  glm::vec3 landing = cMap->getRandomLanding();
  g_player_controller->setPosition(landing);
  
  glm::vec2 current_world_pos = g_player_controller->getWorldPosition();
  int current_ambient_id = 0;
  
  // Load compass if specified
  std::shared_ptr<C2CarFile> compass;
  if (!compassPath.empty()) {
    try {
      compass = cFileLoad->fetch(compassPath);
      std::cout << "Compass loaded successfully" << std::endl;
    } catch (const std::exception& e) {
      std::cerr << "Failed to load compass: " << e.what() << std::endl;
    }
  }
  
  // Load primary weapon if specified
  std::shared_ptr<C2CarFile> primaryWeapon;
  if (!primaryWeaponPath.empty()) {
    try {
      primaryWeapon = cFileLoad->fetch(primaryWeaponPath, true);  // Use pixel-perfect textures for weapons
      std::cout << "Primary weapon loaded successfully" << std::endl;
    } catch (const std::exception& e) {
      std::cerr << "Failed to load primary weapon: " << e.what() << std::endl;
    }
  }
  
  // Initialize UI renderer
  std::unique_ptr<CEUIRenderer> uiRenderer;
  if (compass || primaryWeapon) {
    uiRenderer = std::make_unique<CEUIRenderer>(width, height);
    
    // Configure weapon animations if we have a weapon
    if (primaryWeapon) {
      uiRenderer->configureWeaponAnimations(weaponDrawAnimation, weaponHolsterAnimation, weaponFireAnimation, weaponReloadAnimation);
      uiRenderer->setAudioManager(g_audio_manager.get());
      uiRenderer->setProjectileManager(projectileManager.get()); // Re-enabled with performance optimizations
      uiRenderer->setGameCamera(g_player_controller->getCamera());
      uiRenderer->configureProjectiles(muzzleVelocity, muzzleOffset, projectileDamage);
      uiRenderer->configureAmmo(weaponMaxRounds);
      uiRenderer->configureRecoil(weaponRecoil);
      uiRenderer->configureSway(weaponSway);
    }
    
    std::cout << "UI renderer initialized" << std::endl;
  }
  
  // Bind UI renderer to input manager if we have it
  if (uiRenderer) {
    input_manager->BindUIRenderer(uiRenderer.get());
  }
  
  std::cout << "== Entering render loop ==" << std::endl;
  
  GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR) {
    std::cerr << "OpenGL error at " << "entering render loop" << ": " << err << std::endl;
  }
  
  for (const auto& character : characters) {
    if (character) {
      character->uploadStateToHardware();
    }
  }
  
  // Prepare shadow-casting models (do this once at startup)
  // Prepare shadow-casting models (debug output removed for performance)
  std::vector<CEWorldModel*> allModels;
  for (int i = 0; i < cMapRsc->getWorldModelCount(); i++) {
    CEWorldModel* model = cMapRsc->getWorldModel(i);
    if (model && CEShadowManager::shouldCastShadow(model)) {
      allModels.push_back(model);
    }
  }
  
  // Found models that should cast shadows (debug output removed for performance)
  
  // Get basic map info for shadow calculations
  float tileLength = cMap->getTileLength();
  float mapWidth = cMap->getWidth() * tileLength;
  float mapHeight = cMap->getHeight() * tileLength;
  
  std::cout << "Map dimensions: " << cMap->getWidth() << "x" << cMap->getHeight() << " tiles" << std::endl;
  std::cout << "Tile length: " << tileLength << std::endl;
  
  // Shadow system state for optimization
  glm::vec3 lastShadowCenter(0, 0, 0);
  float shadowUpdateDistance = tileLength * 185.0f;
  bool shadowsGenerated = false;
  
  // grab a character
  auto charac = characters.at(1);
  g_player_controller->update(glfwGetTime(), 0.0);

  while (!glfwWindowShouldClose(window) && !input_manager->GetShouldShutdown()) {
    glfwMakeContextCurrent(window);
    
    // Process input before rendering
    auto frameStart = std::chrono::high_resolution_clock::now();
    double currentTime = glfwGetTime();
    double timeDelta = currentTime - lastTime;
    lastTime = currentTime;
    
    input_manager->ProcessLocalInput(window, timeDelta);
    g_player_controller->update(currentTime, timeDelta);
    
    // Update projectile physics simulation (Re-enabled with performance optimizations)
    if (projectileManager) {
      projectileManager->update(currentTime, timeDelta);
    }
    
    // Update visual impact markers
    updateImpactMarkers(currentTime);
    
    // Update trace lines
    updateTraceLines(currentTime);
    
    Camera* camera = g_player_controller->getCamera();
    
    // Generate shadows around player position (only when player moves significantly)
    glm::vec3 playerPos = g_player_controller->getPosition();
    glm::vec2 playerWorldPos = g_player_controller->getWorldPosition();
    
    // Get ground height at player position
    float playerGroundHeight = cMap->getPlaceGroundHeight((int)playerWorldPos.x, (int)playerWorldPos.y);
    
    // Create scene center around player position
    glm::vec3 sceneCenter(
                          playerPos.x,  // Use actual player X coordinate
                          playerGroundHeight,  // Ground height at player location
                          playerPos.z   // Use actual player Z coordinate
                          );
    
    // Only regenerate shadows if player moved significantly or first time
    float distanceFromLastShadow = glm::distance(sceneCenter, lastShadowCenter);
    if (!shadowsGenerated || distanceFromLastShadow > shadowUpdateDistance) {
      // Use a much larger shadow radius to cover more of the map
      float sceneRadius = tileLength * 200.0f; // Radius auto-scales with tileLength
      
      // Generate shadows for this area (this will use cache if available)
      // Light matrices are calculated once during generation and remain stable
      shadowManager->generateShadowMap(allModels, sceneCenter, sceneRadius);
      
      lastShadowCenter = sceneCenter;
      shadowsGenerated = true;
    }
    // No recalculation needed - shadows remain stable until player moves significantly
    
    // Process AI for deployed characters
    glm::vec2 player_world_pos = g_player_controller->getWorldPosition();
    for (const auto& character : characters) {
      if (character) {
        character->updateWithObserver(currentTime, g_terrain_transform, *camera, player_world_pos);
      }
    }
    
    glm::vec3 currentPosition = g_player_controller->getPosition();
    
    for (const auto& ambient : ambients) {
      if (ambient) {
        ambient->Process(currentTime);
        
        // TODO: totally change this for multi-player?
        // For now, it's only enabled if you spawn GenericAmbients, but we'd probably want to update it to track all "players" or entities.
        if (ambient->IsDangerous()) {
          auto character = ambient->GetPlayerController();
          auto contactDist = glm::distance(character->getPosition(), g_player_controller->getPosition());
          if (contactDist < cMap->getTileLength() && g_player_controller->isAlive(currentTime)) {
            g_player_controller->kill(currentTime);
            auto body = std::make_shared<CERemotePlayerController>(
                                                                   g_audio_manager,
                                                                   cFileLoad->fetch(basePath / "DEAD.CAR"),
                                                                   cMap,
                                                                   cMapRsc,
                                                                   "Hr_dead1"
                                                                   );
            auto bodyPos = g_player_controller->getPosition();
            bodyPos.y = cMap->getPlaceGroundHeight(player_world_pos.x, player_world_pos.y) + 0.75f; // Scaled down 16x (was 12.f)
            body->setPosition(bodyPos);
            body->setNextAnimation("Hr_dead1");
            body->StopMovement();
            body->uploadStateToHardware();
            characters.push_back(body);
            
            dieAudioSrc->setPosition(bodyPos);
            g_audio_manager->play(dieAudioSrc);
            ambient->ReportNotableEvent(currentPosition, "PLAYER_ELIMINATED", currentTime);
          }
        }
        
        if (ambient->NoticesLocalPlayer(g_player_controller)) {
          if (g_player_controller->isAlive(currentTime)) {
            ambient->ReportNotableEvent(currentPosition, "PLAYER_SPOTTED", currentTime);
          }
        }
      }
    }
    
    // Clear color, depth, and stencil buffers at the beginning of each frame
    // Check framebuffer status before clearing
    GLenum framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (framebufferStatus != GL_FRAMEBUFFER_COMPLETE) {
      std::cerr << "Framebuffer not complete before clear: " << framebufferStatus << std::endl;
    }
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    checkGLError("After glClear");
    
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    // Toggle debug UI with F1 key
    static bool f1Pressed = false;
    if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS) {
      if (!f1Pressed) {
        showDebugUI = !showDebugUI;
        f1Pressed = true;
      }
    } else {
      f1Pressed = false;
    }
    
    glm::vec2 currentWorldPos = g_player_controller->getWorldPosition();
    glm::vec2 next_world_pos = g_player_controller->getWorldPosition();
    bool outOfBounds = next_world_pos.x < 0 || next_world_pos.x > cMap->getWidth() || next_world_pos.y < 0 || next_world_pos.y > cMap->getHeight();
    
    if (next_world_pos != current_world_pos) {
      if (!outOfBounds) {
        int next_ambient_id = cMap->getAmbientAudioIDAt((int)next_world_pos.x, (int)next_world_pos.y);
        
        if (next_ambient_id != current_ambient_id) {
          m_ambient = cMapRsc->getAmbientAudio(next_ambient_id);
          g_audio_manager->playAmbient(std::move(m_ambient));
          current_ambient_id = next_ambient_id;
        }
      }
      
      current_world_pos = next_world_pos;
    }
    
    // Handle random audio.
    int currentAmbientArea = cMap->getAmbientAudioIDAt((int)currentWorldPos.x, (int)currentWorldPos.y);
    
    // Update timer FIRST, THEN check if we need to play a track
    cMapRsc->updateRandomSoundTimer(currentAmbientArea, timeDelta);
    
    if (!outOfBounds && cMapRsc->shouldPlayRandomSound(currentAmbientArea, currentTime)) {
      m_random_ambient = cMapRsc->getRandomAudioForArea(currentAmbientArea,
                                                        currentPosition.x,
                                                        currentPosition.y,
                                                        currentPosition.z);
      if (m_random_ambient) {
        g_audio_manager->play(std::move(m_random_ambient));
      }
    }
    
    // Render the terrain
    terrain->Update(g_terrain_transform, *camera);
    
    if (render_terrain) {
      glDepthFunc(GL_LESS); // Depth test for terrain
      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK); // Cull back faces
      
      cMapRsc->getTexture(0)->use();
      
      // Enable shadows on terrain to receive object shadows
      terrain->RenderWithShadows(*camera, shadowManager.get());
      
      glDisable(GL_CULL_FACE);
      
      // Render the water
      if (render_water) {
        glDepthFunc(GL_LESS);
        terrain->RenderWater();
      }
      
      // Render fog volumes
      terrain->RenderFogVolumes();
    }
    
    // Render the terrain objects
    if (render_objects) {
      glDepthFunc(GL_LESS); // Depth test for objects
      
      glDisable(GL_CULL_FACE);
      
      glEnable(GL_DEPTH_TEST);
      
      terrain->RenderObjectsWithShadows(*camera, shadowManager.get());
      
      glEnable(GL_CULL_FACE);
    }
    
    // Render debug bounding boxes if enabled
    if (render_objects && input_manager->GetShowBoundingBoxes()) {
      glDepthFunc(GL_LESS);
      glEnable(GL_DEPTH_TEST);
      
      // Get view-projection matrix for shader setup (if needed)
      glm::mat4 viewProjectionMatrix = camera->getProjectionMatrix() * camera->getViewMatrix();
      
      // Render bounding boxes for all world object models
      for (int m = 0; m < cMapRsc->getWorldModelCount(); m++) {
        CEWorldModel* model = cMapRsc->getWorldModel(m);
        if (model) {
          model->renderBoundingBox(viewProjectionMatrix);
          model->renderRadiusCylinder(viewProjectionMatrix);
          model->renderPhysicsMeshDebug(viewProjectionMatrix); // Show accurate physics mesh wireframes
        }
      }
    }
    
    // Render Bullet Physics debug visualization if enabled
    static bool lastPhysicsDebugState = false;
    bool currentPhysicsDebugState = input_manager->GetShowPhysicsDebug();
    
    if (projectileManager) {
      // Only call enablePhysicsDebugRendering when state changes (major performance optimization)
      if (currentPhysicsDebugState != lastPhysicsDebugState) {
        projectileManager->getPhysicsWorld()->enablePhysicsDebugRendering(currentPhysicsDebugState);
        lastPhysicsDebugState = currentPhysicsDebugState;
      }
      
      // Render debug info if enabled
      if (currentPhysicsDebugState) {
        glDepthFunc(GL_LEQUAL); // Allow debug to render over objects
        glEnable(GL_DEPTH_TEST);
        
        // Get view-projection matrix and camera position for physics debug rendering
        glm::mat4 viewProjectionMatrix = camera->getProjectionMatrix() * camera->getViewMatrix();
        glm::vec3 cameraPosition = g_player_controller->getPosition();
        
        // Render the debug information with selective object drawing (no terrain)
        projectileManager->getPhysicsWorld()->renderPhysicsDebug(viewProjectionMatrix, cameraPosition);
      }
    }
    
    // Render models
    if (render_objects) {
      glDepthFunc(GL_LESS);
      glDisable(GL_CULL_FACE);
      
      glEnable(GL_DEPTH_TEST);
      
      for (const auto& character : characters) {
        if (character) {
          character->Render();
        }
      }
      
      glEnable(GL_CULL_FACE);
    }
    
    // Render impact markers (always on for debugging)
    if (impactMarkerGeometry && !impactMarkerTransforms.empty()) {
      glDepthFunc(GL_LESS);
      glDisable(GL_CULL_FACE);
      glEnable(GL_DEPTH_TEST);
      
      // Get shader and enable custom coloring for impact markers
      auto shader = impactMarkerGeometry->getShader();
      if (shader) {
        shader->use();
        shader->setBool("useCustomColor", true);
        
        // For now, use a single color for all markers. 
        // TODO: Implement per-instance coloring
        if (!impactMarkerColors.empty()) {
          shader->setVec3("customColor", impactMarkerColors[0]);
        } else {
          shader->setVec3("customColor", glm::vec3(1.0f, 0.0f, 1.0f)); // Magenta fallback
        }
      }
      
      // Update instanced transforms for markers
      impactMarkerGeometry->UpdateInstances(impactMarkerTransforms);
      impactMarkerGeometry->Update(*camera);
      impactMarkerGeometry->DrawInstances();
      
      // Reset custom color flag to not affect other objects
      if (shader) {
        shader->setBool("useCustomColor", false);
      }
      
      glEnable(GL_CULL_FACE);
    }
    
    // Render trace lines (always on for debugging) - simulate with large spheres for now
    if (!projectileTraceLines.empty()) {
      // For now, render trace lines as pairs of large colored spheres at start and end points
      // This is a temporary solution until we implement proper line rendering
      glDepthFunc(GL_LESS);
      glDisable(GL_CULL_FACE);
      glEnable(GL_DEPTH_TEST);
      
      // Use impact marker geometry to render large spheres at start/end points
      if (impactMarkerGeometry) {
        auto shader = impactMarkerGeometry->getShader();
        if (shader) {
          shader->use();
          shader->setBool("useCustomColor", true);
        }
        
        for (const auto& traceLine : projectileTraceLines) {
          // Render start point as large sphere
          std::vector<glm::mat4> startTransforms;
          glm::mat4 startTransform = glm::mat4(1.0f);
          startTransform = glm::translate(startTransform, traceLine.startPoint);
          startTransform = glm::scale(startTransform, glm::vec3(100.0f)); // Very large spheres
          startTransforms.push_back(startTransform);
          
          if (shader) {
            shader->setVec3("customColor", traceLine.color);
          }
          
          impactMarkerGeometry->UpdateInstances(startTransforms);
          impactMarkerGeometry->Update(*camera);
          impactMarkerGeometry->DrawInstances();
          
          // Render end point as large sphere with slightly different color
          std::vector<glm::mat4> endTransforms;
          glm::mat4 endTransform = glm::mat4(1.0f);
          endTransform = glm::translate(endTransform, traceLine.endPoint);
          endTransform = glm::scale(endTransform, glm::vec3(80.0f)); // Slightly smaller for end point
          endTransforms.push_back(endTransform);
          
          glm::vec3 endColor = traceLine.color * 1.2f; // Brighter end point
          if (shader) {
            shader->setVec3("customColor", endColor);
          }
          
          impactMarkerGeometry->UpdateInstances(endTransforms);
          impactMarkerGeometry->Update(*camera);
          impactMarkerGeometry->DrawInstances();
        }
        
        // Reset custom color flag
        if (shader) {
          shader->setBool("useCustomColor", false);
        }
      }
      
      glEnable(GL_CULL_FACE);
    }
    
    // Trajectory path rendering removed to prevent collision with bullet spheres
    // Only final impact markers are rendered below
    
    // Render the sky
    if (render_sky) {
      glDepthFunc(GL_LESS);
      
      glDisable(GL_CULL_FACE);
      
      glDepthMask(GL_FALSE); // Disable depth writes
      
      cMapRsc->getDaySky()->Render(window, *camera);
      
      glDepthMask(GL_TRUE); // Re-enable depth writes
      
      glEnable(GL_CULL_FACE);
    }
    
    // Render UI elements (compass, weapon, etc.)
    if (uiRenderer) {
      Camera* camera = g_player_controller->getCamera();
      
      // Render compass if available
      if (compass) {
        uiRenderer->renderCompass(compass.get(), camera);
      }
      
      // Update weapon sway based on player movement
      float playerSpeed = g_player_controller->getCurrentSpeed();
      uiRenderer->updateSway(currentTime, playerSpeed);
      
      // Render weapon if available
      if (primaryWeapon) {
        uiRenderer->renderWeapon(primaryWeapon.get(), currentTime);
      }
    } // End uiRenderer
      
    // Only show debug UI if enabled
    if (showDebugUI) {
        // Create ImGui FPS display window in upper-left corner as requested (update less frequently)
        static double lastFpsUpdate = 0;
        static int cachedFps = 0;
        static double cachedAvgFrameTime = 0;
        static float cachedPerfPercent = 0;
        
        if (currentTime - lastFpsUpdate > 0.5) { // Update every 0.5 seconds instead of every frame
          cachedFps = fps;
          cachedAvgFrameTime = fps > 0 ? 1000.0 / fps : 0.0;
          cachedPerfPercent = fps > 0 ? (100.0f * fps / FPS) : 0.0f;
          lastFpsUpdate = currentTime;
        }
        
        {
          ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
          ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
          ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
          
          if (ImGui::Begin("Performance Monitor", nullptr, window_flags)) {
            // Color-coded FPS display (using cached values)
            if (cachedFps >= FPS * 0.8f) {
              ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "FPS: %d", cachedFps);
            } else if (cachedFps >= FPS * 0.6f) {
              ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "FPS: %d", cachedFps);
            } else {
              ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "FPS: %d", cachedFps);
            }
            
            ImGui::Text("Frame Time: %.1f ms", cachedAvgFrameTime);
            ImGui::Text("Performance: %.0f%% of target", cachedPerfPercent);
          }
          ImGui::End();
        }
        
        // Add collision debug panel (update less frequently)
        static double lastCollisionUpdate = 0;
        static glm::vec3 cachedPlayerPos;
        static glm::vec2 cachedWorldPos;
        static int cachedActiveProjectiles = 0;
        
        if (currentTime - lastCollisionUpdate > 0.1) { // Update every 0.1 seconds instead of every frame
          cachedPlayerPos = g_player_controller->getPosition();
          cachedWorldPos = g_player_controller->getWorldPosition();
          if (projectileManager) {
            cachedActiveProjectiles = static_cast<int>(projectileManager->getActiveProjectileCount());
          }
          lastCollisionUpdate = currentTime;
        }
        
        {
          ImGuiWindowFlags debug_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
          ImGui::SetNextWindowPos(ImVec2(10, 80), ImGuiCond_Always);
          ImGui::SetNextWindowBgAlpha(0.35f);
          
          if (ImGui::Begin("Collision Debug", nullptr, debug_flags)) {
            ImGui::Text("Player Pos: %.1f, %.1f, %.1f", cachedPlayerPos.x, cachedPlayerPos.y, cachedPlayerPos.z);
            ImGui::Text("World Pos: %.1f, %.1f", cachedWorldPos.x, cachedWorldPos.y);
            ImGui::Text("Active Projectiles: %d", cachedActiveProjectiles);
            ImGui::Text("Impact Markers: %zu", visualImpactMarkers.size());
          }
          ImGui::End();
        }
        
        // Add impact history panel (simplified and less frequent updates)
        static double lastImpactUpdate = 0;
        static bool hasRecentImpacts = false;
        static size_t cachedImpactCount = 0;
        
        // Only update impact display every 0.2 seconds to reduce performance impact
        if (currentTime - lastImpactUpdate > 0.2) {
          hasRecentImpacts = !recentImpacts.empty();
          cachedImpactCount = recentImpacts.size();
          lastImpactUpdate = currentTime;
        }
        
        {
          ImGuiWindowFlags impact_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
          ImGui::SetNextWindowPos(ImVec2(10, 140), ImGuiCond_Always);
          ImGui::SetNextWindowBgAlpha(0.35f);
          
          if (ImGui::Begin("Impact History", nullptr, impact_flags)) {
            if (!hasRecentImpacts) {
              ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No impacts recorded");
            } else {
              ImGui::Text("Recent Impacts: %zu", cachedImpactCount);
              ImGui::Separator();
              
              // Show only the most recent impact to reduce performance cost
              if (!recentImpacts.empty()) {
                const auto& impact = recentImpacts[0];
                double age = currentTime - impact.timestamp;
                
                // Simplified display without emojis to improve performance
                ImGui::Text("Latest: %s Impact", impact.impactType.c_str());
                ImGui::Text("Location: [%.1f, %.1f, %.1f]", impact.location.x, impact.location.y, impact.location.z);
                ImGui::Text("Surface: %s (%.1fm, %.0fdmg)", impact.surfaceType.c_str(), impact.distance, impact.damage);
                
                // Show object information if available
                if (!impact.objectName.empty() && impact.objectIndex >= 0) {
                  ImGui::Text("Object: %s", impact.objectName.c_str());
                  ImGui::Text("Index: %d, Instance: %d", impact.objectIndex, impact.instanceIndex);
                }
                
                // Show face intersection details for detailed impacts
                if (impact.impactType == "Face Intersection") {
                  ImGui::Separator();
                  ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Face Details:");
                  
                  // Show surface normal
                  ImGui::Text("Normal: [%.2f, %.2f, %.2f]", impact.surfaceNormal.x, impact.surfaceNormal.y, impact.surfaceNormal.z);
                  
                  // Show incoming direction
                  ImGui::Text("Direction: [%.2f, %.2f, %.2f]", impact.incomingDirection.x, impact.incomingDirection.y, impact.incomingDirection.z);
                  
                  // Show terrain-specific information
                  if (impact.surfaceType == "terrain" && impact.tileX >= 0 && impact.tileZ >= 0) {
                    ImGui::Text("Tile: [%d, %d]", impact.tileX, impact.tileZ);
                  }
                  
                  // Show face index if available
                  if (impact.faceIndex >= 0) {
                    ImGui::Text("Face Index: %d", impact.faceIndex);
                  } else {
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Face Index: N/A");
                  }
                }
                
                ImGui::Text("Age: %.1fs ago", age);
              }
            }
          }
          ImGui::End();
      } // End showDebugUI
    } // End UI rendering
    
    // Always show ammo counter when weapon is drawn (hunter sim style HUD)
    if (uiRenderer && uiRenderer->isWeaponDrawn()) {
      ImGuiWindowFlags ammo_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | 
                                    ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | 
                                    ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;
      
      // Position in lower-right corner (classic hunter sim style)
      ImVec2 windowSize = ImGui::GetIO().DisplaySize;
      ImGui::SetNextWindowPos(ImVec2(windowSize.x - 160, windowSize.y - 80), ImGuiCond_Always);
      ImGui::SetNextWindowBgAlpha(0.8f); // More opaque for important ammo info
      
      if (ImGui::Begin("Ammo Counter", nullptr, ammo_flags)) {
        int currentAmmo = uiRenderer->getCurrentAmmo();
        int maxAmmo = uiRenderer->getMaxAmmo();
        
        // Color-coded ammo display: green=full, yellow=medium, red=low/empty
        ImVec4 ammoColor;
        if (currentAmmo <= 0) {
          ammoColor = ImVec4(1.0f, 0.2f, 0.2f, 1.0f); // Bright red for empty
        } else if (currentAmmo <= maxAmmo / 3) {
          ammoColor = ImVec4(1.0f, 0.6f, 0.0f, 1.0f); // Orange for low
        } else if (currentAmmo <= maxAmmo / 2) {
          ammoColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f); // Yellow for medium
        } else {
          ammoColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // Green for good
        }
        
        // Push larger font size for better visibility
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 4));
        ImFont* font = ImGui::GetFont();
        ImGui::PushFont(font);
        ImGui::SetWindowFontScale(1.8f); // Make text 80% larger
        
        // Display ammo in classic style: "12/12" or "0/12" format
        ImGui::TextColored(ammoColor, "%d/%d", currentAmmo, maxAmmo);
        
        // Show reload status if reloading
        if (uiRenderer->getWeaponState() == CEUIRenderer::WeaponState::RELOADING) {
          ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "RELOADING...");
        }
        
        ImGui::SetWindowFontScale(1.0f); // Reset font scale
        ImGui::PopFont();
        ImGui::PopStyleVar();
      }
      ImGui::End();
    }
    
    // Always render ImGui (even if no UI is shown) to maintain frame consistency
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    glfwSwapBuffers(window);
    glfwPollEvents();
    
    CalculateFrameRate();
    
    auto frameEnd = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float, std::milli> frameDuration = frameEnd - frameStart;
    
    const int frameDelay = 1000 / FPS; // Compute frame delay from FPS
    if (frameDuration.count() < frameDelay) {
      std::this_thread::sleep_for(std::chrono::milliseconds(frameDelay) - frameDuration);
    }
  }
  
  characters.clear();
  characters.shrink_to_fit();
  
  // Cleanup ImGui
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  
  return 0;
}
