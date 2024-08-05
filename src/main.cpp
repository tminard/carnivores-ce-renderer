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

namespace fs = std::filesystem;
using json = nlohmann::json;

struct ConfigSpawn {
  std::string file;
  std::string animation;
  std::vector<int> position;
  bool isAmbient;
  std::string ambientWalkAnim;
  json data;
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

const int FPS = 80;
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
      spawn.isAmbient = false;
      spawn.ambientWalkAnim = "";
      // TODO: temporary. Switch to generic controlled character class from direct remote controller.
      if (spawnJson.contains("attachAI")) {
        if (spawnJson["attachAI"].contains("controller")) {
          if (spawnJson["attachAI"]["controller"] == "GenericAmbient") {
            spawn.isAmbient = true;
            spawn.ambientWalkAnim = spawnJson["attachAI"]["args"]["animations"]["WALK"];
          }
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
  
  std::cout << "Base Path: " << basePath << std::endl;
  std::cout << "Map: " << data["map"]["type"] << std::endl;
  std::cout << "MAP: " << mapPath << std::endl;
  std::cout << "RSC: " << mapRscPath << std::endl;
  std::cout << "Spawns: " << spawns.size() << std::endl;
  
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
      
      float walkSpeed = spawn.data["attachAI"]["args"]["character"]["walkSpeed"];
      float heightOffset = spawn.data["attachAI"]["args"]["character"].contains("heightOffset") ? (float)spawn.data["attachAI"]["args"]["character"]["heightOffset"] : 0.f;
  
      character->setWalkSpeed(walkSpeed);
      character->setHeightOffset(heightOffset);

      characters.push_back(character);
      if (spawn.isAmbient) {
        AIGenericAmbientManagerConfig aiConfig;
        aiConfig.WalkAnimName = spawn.ambientWalkAnim;
        auto ambientMg = std::make_unique<CEAIGenericAmbientManager>(
                                                                     aiConfig,
                                                                     character,
                                                                     cMap,
                                                                     cMapRsc);
        ambients.push_back(std::move(ambientMg));
      }
      
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
  
  while (!glfwWindowShouldClose(window) && !input_manager->GetShouldShutdown()) {
    glfwMakeContextCurrent(window);
    
    // Process input before rendering
    auto frameStart = std::chrono::high_resolution_clock::now();
    double currentTime = glfwGetTime();
    double timeDelta = currentTime - lastTime;
    lastTime = currentTime;
    
    input_manager->ProcessLocalInput(window, timeDelta);
    g_player_controller->update(timeDelta);
    
    Camera* camera = g_player_controller->getCamera();
    
    // Process AI
    glm::vec2 player_world_pos = g_player_controller->getWorldPosition();
    for (const auto& character : characters) {
      if (character) {
        character->update(currentTime, g_terrain_transform, *camera, player_world_pos);
      }
    }
    
    for (const auto& ambient : ambients) {
      if (ambient) {
        ambient->Process(currentTime);
      }
    }
    
    // Clear color, depth, and stencil buffers at the beginning of each frame
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    checkGLError("After glClear");
    
    glm::vec3 currentPosition = g_player_controller->getPosition();
    double rnTimeDelta = currentTime - lastRndAudioTime;
    
    if (rnTimeDelta >= 32.0) {
      m_random_ambient = cMapRsc->getRandomAudio(currentPosition.x, currentPosition.y, currentPosition.z - cMap->getTileLength());
      if (m_random_ambient) {
        g_audio_manager->play(std::move(m_random_ambient));
        lastRndAudioTime = currentTime;
      }
    }
    
    glm::vec2 next_world_pos = g_player_controller->getWorldPosition();
    if (next_world_pos != current_world_pos) {
      bool outOfBounds = next_world_pos.x < 0 || next_world_pos.x > cMap->getWidth() || next_world_pos.y < 0 || next_world_pos.y > cMap->getHeight();
      
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
    
    // Render the terrain
    terrain->Update(g_terrain_transform, *camera);
    
    if (render_terrain) {
      glDepthFunc(GL_LESS); // Depth test for terrain
      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK); // Cull back faces
      
      cMapRsc->getTexture(0)->use();
      
      terrain->Render();
      
      glDisable(GL_CULL_FACE);
      
      // Render the water
      if (render_water) {
        glDepthFunc(GL_LESS);
        terrain->RenderWater();
      }
    }
    
    // Render the terrain objects
    if (render_objects) {
      glDepthFunc(GL_LESS); // Depth test for objects
      
      glDisable(GL_CULL_FACE);
      
      glEnable(GL_DEPTH_TEST);

      terrain->RenderObjects(*camera);
      
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
