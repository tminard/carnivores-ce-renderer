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

#include <iostream>
#include <fstream>
#include <filesystem>

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

namespace fs = std::filesystem;
using json = nlohmann::json;

struct ConfigSpawn {
  json data;
  std::string file;
  std::string animation;
  std::vector<int> position;
  std::string aiControllerName;
};

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

int _fpsCount = 0;
int fps = 0;

std::chrono::time_point<std::chrono::steady_clock> lastTime = std::chrono::steady_clock::now();

void CalculateFrameRate() {
  auto currentTime = std::chrono::steady_clock::now();
  
  const auto elapsedTime = std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime - lastTime).count();
  ++_fpsCount;
  
  if (elapsedTime > 1000000000) {
    lastTime = currentTime;
    fps = _fpsCount;
    _fpsCount = 0;
    
    std::cout << "fps: " << fps << std::endl;
  }
}

const int FPS = 90;
const int frameDelay = 1000 / FPS;

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
  
  std::cout << "Base Path: " << basePath << std::endl;
  std::cout << "Map: " << data["map"]["type"] << std::endl;
  std::cout << "MAP: " << mapPath << std::endl;
  std::cout << "RSC: " << mapRscPath << std::endl;
  std::cout << "Spawns: " << spawns.size() << std::endl;
  if (!compassPath.empty()) {
    std::cout << "Compass: " << compassPath << std::endl;
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
  
  // Load audio assets
  std::shared_ptr<Sound> die = std::make_shared<Sound>(basePath / "game" / "SOUNDFX" / "HUM_DIE1.WAV");
  std::shared_ptr<CEAudioSource> dieAudioSrc = std::make_shared<CEAudioSource>(die);
  dieAudioSrc->setLooped(false);
  dieAudioSrc->setGain(2.0f);
  dieAudioSrc->setClampDistance(256*6);
  dieAudioSrc->setMaxDistance(256*80);

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
          spawnHeight - 12.f,
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
  
  // Initialize shadow manager after OpenGL context is ready
  shadowManager->initialize();
  
  // Set light direction to be extremely vertical (like sun directly overhead)
  // Almost perfectly straight down
  shadowManager->setLightDirection(glm::vec3(0.05f, -1.0f, 0.02f));
  
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

  // Initialize UI renderer
  std::unique_ptr<CEUIRenderer> uiRenderer;
  if (compass) {
    uiRenderer = std::make_unique<CEUIRenderer>(width, height);
    std::cout << "UI renderer initialized" << std::endl;
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
  
  while (!glfwWindowShouldClose(window) && !input_manager->GetShouldShutdown()) {
    glfwMakeContextCurrent(window);
    
    // Process input before rendering
    auto frameStart = std::chrono::high_resolution_clock::now();
    double currentTime = glfwGetTime();
    double timeDelta = currentTime - lastTime;
    lastTime = currentTime;
    
    input_manager->ProcessLocalInput(window, timeDelta);
    g_player_controller->update(timeDelta, currentTime);

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
      float sceneRadius = tileLength * 200.0f; // Large radius to cover entire visible map area
      
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
            bodyPos.y = cMap->getPlaceGroundHeight(player_world_pos.x, player_world_pos.y) + 12.f;
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
    
    // Render the sky
    if (render_sky) {
      glDepthFunc(GL_LESS);
      
      glDisable(GL_CULL_FACE);
      
      glDepthMask(GL_FALSE); // Disable depth writes
      
      cMapRsc->getDaySky()->Render(window, *camera);
      
      glDepthMask(GL_TRUE); // Re-enable depth writes
      
      glEnable(GL_CULL_FACE);
    }

    // Render UI elements (compass, etc.)
    if (uiRenderer && compass) {
      // Calculate compass rotation based on camera direction
      Camera* camera = g_player_controller->getCamera();
      float compassRotation = 0.0f; // For now, static compass
      
      // TODO: Calculate rotation based on camera yaw
      // compassRotation = -camera->getYaw();
      
      uiRenderer->renderCompass(compass.get(), compassRotation);
    }
    
    glfwSwapBuffers(window);
    glfwPollEvents();

    // CalculateFrameRate();
    
    auto frameEnd = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float, std::milli> frameDuration = frameEnd - frameStart;

    if (frameDuration.count() < frameDelay) {
        std::this_thread::sleep_for(std::chrono::milliseconds(frameDelay) - frameDuration);
    }
  }
  
  characters.clear();
  characters.shrink_to_fit();
  
  return 0;
}
