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
#include <chrono>

#include <nlohmann/json.hpp>

#include "CE_Allosaurus.h"
#include "CEAnimation.h"

namespace fs = std::filesystem;
using json = nlohmann::json;

struct ConfigSpawn {
    std::string file;
    std::string animation;
    std::vector<int> position;
};

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

std::unique_ptr<C2CarFile> spawnCarFile(const std::filesystem::path& fPath, std::shared_ptr<C2MapFile> cMap, const glm::vec2& alloWorldPos, const glm::vec3& initialScale) {
    // We need to load from disk to get a unique instance of our car file
    std::unique_ptr<C2CarFile> carFile = std::unique_ptr<C2CarFile>(new C2CarFile(fPath.string()));
    auto geo = carFile->getGeometry();

    float alloHeight = cMap->getPlaceGroundHeight(alloWorldPos.x, alloWorldPos.y);
    glm::vec3 alloPos = glm::vec3(
        (alloWorldPos.x * cMap->getTileLength()) + (cMap->getTileLength() / 2),
        alloHeight - 12.f,
        (alloWorldPos.y * cMap->getTileLength()) + (cMap->getTileLength() / 2)
    );

    // Set the transform for the Allosaurus with the given initial position and scale
    Transform mTrans_allo(alloPos, glm::vec3(0, 0, 0), initialScale);
    std::vector<glm::mat4> aTM = { mTrans_allo.GetStaticModel() };

    // Only ever ONE instance!
    geo->UpdateInstances(aTM);

    // Return control
    return std::move(carFile);
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
  
  fs::path basePath = fs::path(data["basePath"].get<std::string>());
  fs::path mapRscPath = constructPath(basePath, data["map"]["rsc"]);
  fs::path mapPath = constructPath(basePath, data["map"]["map"]);

  if (data.contains("spawns")) {
      for (const auto& spawnJson : data["spawns"]) {
          ConfigSpawn spawn;
          spawn.file = spawnJson.value("file", "");
          spawn.animation = spawnJson.value("animation", "");
          spawn.position = spawnJson.value("position", std::vector<int>{});
          spawns.push_back(spawn);
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
  
  alDistanceModel(AL_LINEAR_DISTANCE);
  
  std::unique_ptr<LocalVideoManager> video_manager(new LocalVideoManager());
  std::unique_ptr<LocalAudioManager> g_audio_manager(new LocalAudioManager());
  
  std::shared_ptr<C2MapRscFile> cMapRsc(new C2MapRscFile(mapType, mapRscPath.string(), basePath.string()));
  std::shared_ptr<C2MapFile> cMap(new C2MapFile(mapType, mapPath.string(), cMapRsc.get()));
  
  std::unique_ptr<TerrainRenderer> terrain(new TerrainRenderer(cMap.get(), cMapRsc.get()));
  
  std::vector<std::unique_ptr<C2CarFile>> dinos = {};
  
  for (const auto& spawn : spawns) {
    dinos.push_back(spawnCarFile(spawn.file, cMap, glm::vec2(spawn.position[0], spawn.position[1]), glm::vec3(1.f)));
  }
  
  GLFWwindow* window = video_manager->GetWindow();
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  
  std::shared_ptr<CEPlayer> g_player_state(new CEPlayer());
  std::shared_ptr<CELocalPlayerController> g_player_controller(new CELocalPlayerController(std::move(g_player_state), cMap->getWidth(), cMap->getHeight(), cMap->getTileLength(), cMap, cMapRsc));
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
  
  while (!glfwWindowShouldClose(window) && !input_manager->GetShouldShutdown()) {
    glfwMakeContextCurrent(window);
    
    // Process input before rendering
    double currentTime = glfwGetTime();
    double timeDelta = currentTime - lastTime;
    lastTime = currentTime;
    
    input_manager->ProcessLocalInput(window, timeDelta);
    g_player_controller->update(timeDelta);
    
    // Process AI
    int di = 0;
    for (const auto& dino : dinos) {
      auto aniName = spawns.at(di).animation;
      if (dino) {
        dino->getGeometry()->SetAnimation(dino->getAnimationByName(aniName), currentTime);
      }
      di++;
    }
    
    // Clear color, depth, and stencil buffers at the beginning of each frame
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    checkGLError("After glClear");
    
    Camera* camera = g_player_controller->getCamera();
    
    glm::vec3 currentPosition = g_player_controller->getPosition();
    double rnTimeDelta = currentTime - lastRndAudioTime;
    
    if (rnTimeDelta >= 6.0) {
      m_random_ambient = cMapRsc->getRandomAudio(currentPosition.x, currentPosition.y, currentPosition.z - cMap->getTileLength());
      g_audio_manager->play(std::move(m_random_ambient));
      lastRndAudioTime = currentTime;
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
    checkGLError("After terrain->Update (terrain)");
    
    if (render_terrain) {
      glDepthFunc(GL_LESS); // Depth test for terrain
      checkGLError("After glDepthFunc (terrain)");
      
      glEnable(GL_CULL_FACE);
      checkGLError("After glEnable(GL_CULL_FACE) (terrain)");
      
      glCullFace(GL_BACK); // Cull back faces
      checkGLError("After glCullFace (terrain)");
      
      cMapRsc->getTexture(0)->use();
      checkGLError("After getTexture(0)->use (terrain)");
      
      terrain->Render();
      checkGLError("After terrain->Render (terrain)");
      
      glDisable(GL_CULL_FACE);
      checkGLError("After glDisable(GL_CULL_FACE) (terrain)");
    }
    
    // Render the terrain objects
    if (render_objects) {
      glDepthFunc(GL_LESS); // Depth test for objects
      checkGLError("After glDepthFunc (objects)");
      
      glDisable(GL_CULL_FACE);
      checkGLError("After glDisable(GL_CULL_FACE) (objects)");
      
      glEnable(GL_DEPTH_TEST);
      checkGLError("After glEnable(GL_DEPTH_TEST) (objects)");

      terrain->RenderObjects(*camera);
      checkGLError("After terrain->RenderObjects (objects)");
      
      glEnable(GL_CULL_FACE);
      checkGLError("After glEnable(GL_CULL_FACE) (objects)");
    }
    
    // Render models
    if (render_objects) {
      glDepthFunc(GL_LESS);
      glDisable(GL_CULL_FACE);
      
      glEnable(GL_DEPTH_TEST);

      for (const auto& dino : dinos) {
          if (dino) {
            dino->getGeometry()->Update(g_terrain_transform, *camera);
            dino->getGeometry()->DrawInstances();
            checkGLError("After draw dino");
          }
      }
      
      glEnable(GL_CULL_FACE);
    }
    
    // Render the water
    if (render_water) {
      glDepthFunc(GL_LESS);
      checkGLError("After glDepthFunc (water)");
      
      terrain->RenderWater();
      checkGLError("After terrain->RenderWater (water)");
    }
    
    // Render the sky
    if (render_sky) {
      glDepthFunc(GL_LESS);
      checkGLError("After glDepthFunc (sky)");
      
      glDisable(GL_CULL_FACE);
      checkGLError("After glDisable(GL_CULL_FACE) (sky)");
      
      glDepthMask(GL_FALSE); // Disable depth writes
      checkGLError("After glDepthMask(GL_FALSE) (sky)");
      
      cMapRsc->getDaySky()->Render(window, *camera);
      checkGLError("After getDaySky()->Render (sky)");
      
      glDepthMask(GL_TRUE); // Re-enable depth writes
      checkGLError("After glDepthMask(GL_TRUE) (sky)");
      
      glEnable(GL_CULL_FACE);
      checkGLError("After glEnable(GL_CULL_FACE) (sky)");
    }
    
    glfwSwapBuffers(window);
    glfwPollEvents();
    
    checkGLError("End of frame");
    
    // CalculateFrameRate();
  }
  
  dinos.clear();
  dinos.shrink_to_fit();
  
  return 0;
}
