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

#include "CE_ArtificialIntelligence.h"

#include "CEObservable.hpp"
#include "CEPlayer.hpp"

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

#include "C2Sky.h"

#include <OpenAL/al.h>
#include <OpenAL/alc.h>

#include <mutex>

#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;

void CreateFadeTab();
// unsigned int timeGetTime();
WORD  FadeTab[65][0x8000];

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

int main(int argc, const char * argv[])
{
  std::ifstream f("config.json");
  json data = json::parse(f);
  
  fs::path basePath = fs::path(data["basePath"].get<std::string>());
  fs::path mapRscPath = constructPath(basePath, data["map"]["rsc"]);
  fs::path mapPath = constructPath(basePath, data["map"]["map"]);
  
  std::cout << "Base Path: " << basePath << std::endl;
  std::cout << "Map: " << data["map"]["type"] << std::endl;
  std::cout << "MAP: " << mapPath << std::endl;
  std::cout << "RSC: " << mapRscPath << std::endl;
  
  auto mapType = CEMapType::C2;
  if (data["map"]["type"] == "C1") {
    mapType = CEMapType::C1;
  }
  
  alDistanceModel(AL_LINEAR_DISTANCE);
  
  std::unique_ptr<C2CarFilePreloader> cFileLoad(new C2CarFilePreloader);
  
  std::unique_ptr<LocalVideoManager> video_manager(new LocalVideoManager());
  std::unique_ptr<LocalAudioManager> g_audio_manager(new LocalAudioManager());
  
  std::unique_ptr<C2MapRscFile> cMapRsc(new C2MapRscFile(mapType, mapRscPath.string(), basePath.string()));
  std::shared_ptr<C2MapFile> cMap(new C2MapFile(mapType, mapPath.string(), cMapRsc.get()));
  
  std::unique_ptr<TerrainRenderer> terrain(new TerrainRenderer(cMap.get(), cMapRsc.get()));
  
  GLFWwindow* window = video_manager->GetWindow();
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  
  std::shared_ptr<CEPlayer> g_player_state(new CEPlayer());
  std::shared_ptr<CELocalPlayerController> g_player_controller(new CELocalPlayerController(std::move(g_player_state), cMap->getWidth(), cMap->getHeight(), cMap->getTileLength(), cMap));
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
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  
  double lastTime = glfwGetTime();
  double lastRndAudioTime = glfwGetTime();
  
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CLIP_DISTANCE0);
  
  glViewport(0, 0, width, height);
  std::shared_ptr<CEAudioSource> m_ambient;
  std::shared_ptr<CEAudioSource> m_random_ambient;
  
  m_ambient = cMapRsc->getAmbientAudio(0);
  g_audio_manager->playAmbient(m_ambient);
  
  m_ambient.reset();
  
  glm::vec3 landing = cMap->getRandomLanding();
  g_player_controller->setPosition(landing);
  
  glm::vec2 current_world_pos = g_player_controller->getWorldPosition();
  int current_ambient_id = 0;
  
  while (!glfwWindowShouldClose(window) && !input_manager->GetShouldShutdown())
  {
    glfwMakeContextCurrent(window);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Camera* camera = g_player_controller->getCamera();
    
    glm::vec3 currentPosition = g_player_controller->getPosition();
    double currentTime = glfwGetTime();
    double timeDelta = currentTime - lastTime;
    
    double rnTimeDelta = currentTime - lastRndAudioTime;
    
    if (rnTimeDelta >= 12.0) {
      // TODO: handle maps that have no random audio
      m_random_ambient = cMapRsc->getRandomAudio(currentPosition.x, currentPosition.y, currentPosition.z - 256.f);
      g_audio_manager->play(std::move(m_random_ambient));
      
      lastRndAudioTime = currentTime;
    }
    
    glm::vec2 next_world_pos = g_player_controller->getWorldPosition();
    if (next_world_pos != current_world_pos) {
      int next_ambient_id = cMap->getAmbientAudioIDAt((int)next_world_pos.x, (int)next_world_pos.y);
      
      if (next_ambient_id != current_ambient_id) {
        m_ambient = cMapRsc->getAmbientAudio(next_ambient_id);
        g_audio_manager->playAmbient(std::move(m_ambient));
        
        current_ambient_id = next_ambient_id;
      }
      
      current_world_pos = next_world_pos;
    }
    
    // Render the sky first
    if (render_sky) {
      glDepthFunc(GL_LEQUAL);
      glDisable(GL_CULL_FACE);
      glDepthMask(GL_FALSE); // Disable depth writes
      cMapRsc->getDaySky()->Render(window, *camera);
      glDepthMask(GL_TRUE); // Re-enable depth writes
      glEnable(GL_CULL_FACE);
    }
    
    // Render the terrain objects
    if (render_objects) {
      glDisable(GL_CULL_FACE);
      glEnable(GL_DEPTH_TEST);
      glDepthFunc(GL_LEQUAL);
      terrain->RenderObjects(*camera);
      glEnable(GL_CULL_FACE);
      glEnable(GL_DEPTH_TEST);
    }
    
    // Render the terrain
    if (render_terrain) {
      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK); // Cull back faces
      glFrontFace(GL_CCW); // Define front faces as counter-clockwise
      glEnable(GL_POLYGON_OFFSET_FILL);
      glPolygonOffset(1.0f, 1.0f);
      cMapRsc->getTexture(0)->use();
      terrain->Update(g_terrain_transform, *camera);
      glDepthFunc(GL_LESS);
      terrain->Render();
      glDisable(GL_POLYGON_OFFSET_FILL);
      glDisable(GL_CULL_FACE);
    }
    
    // Render the water
    if (render_water) {
      glDepthFunc(GL_LEQUAL);
      terrain->RenderWater();
    }
    
    g_player_controller->update(timeDelta);
    
    glfwSwapBuffers(window);
    
    input_manager->ProcessLocalInput(window, timeDelta);
    
    glfwPollEvents();
  }
  
  return 0;
}
