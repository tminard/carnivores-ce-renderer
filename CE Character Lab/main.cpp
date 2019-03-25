//
//  main.cpp
//  CE Character Lab
//
//  Created by Tyler Minard on 8/6/15.
//  Copyright (c) 2015 Tyler Minard. All rights reserved.
//
#define GLM_ENABLE_EXPERIMENTAL
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>

#include "CE_Allosaurus.h"
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

#include "C2Sky.h"

void CreateFadeTab();
unsigned int timeGetTime();
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

int main(int argc, const char * argv[])
{
  CreateFadeTab();
  std::unique_ptr<C2CarFilePreloader> cFileLoad(new C2CarFilePreloader);
  std::unique_ptr<LocalVideoManager> video_manager(new LocalVideoManager());
  std::unique_ptr<C2MapRscFile> cMapRsc(new C2MapRscFile("resources/game/ice/area2.rsc"));
  std::shared_ptr<C2MapFile> cMap(new C2MapFile("resources/game/ice/area2.map", cMapRsc.get()));
  std::shared_ptr<CEPlayer> m_player(new CEPlayer(cMap));
  std::unique_ptr<TerrainRenderer> terrain(new TerrainRenderer(cMap.get(), cMapRsc.get()));
  
  std::cout << "Map texture atlas width: " << cMapRsc->getTextureAtlasWidth();
  
  Transform mTrans_land(glm::vec3(0,0,0), glm::vec3(0,0,0), glm::vec3(1.f, 1.f, 1.f));
  Camera* camera = m_player->getCamera();
  GLFWwindow* window = video_manager->GetWindow();
  
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  
  input_manager->Bind(m_player);
  glfwSetCursorPosCallback(window, &cursorPosCallback);
  
  int RealTime, PrevTime, TimeDt;
  PrevTime = timeGetTime();
  double preTime, curTime, deltaTime;
  preTime = glfwGetTime();
  
  glfwSwapInterval(1);
  
  while (!glfwWindowShouldClose(window))
  {
    glfwMakeContextCurrent(window);
    
    /* Game loop */
    RealTime = timeGetTime();
    srand( (unsigned)RealTime );
    TimeDt = RealTime - PrevTime;
    if (TimeDt<0) TimeDt = 10;
    if (TimeDt>10000) TimeDt = 10;
    if (TimeDt>1000) TimeDt = 1000;
    
    PrevTime = RealTime;
    
    curTime = glfwGetTime();
    deltaTime = curTime - preTime;
    preTime = curTime;

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glDisable(GL_DEPTH_TEST);
    cMapRsc->getDaySky()->Render(window, *camera);
    glEnable(GL_DEPTH_TEST);

    terrain->RenderObjects(*camera);

    cMapRsc->getTexture(0)->use();
    terrain->Update(mTrans_land, *camera);
    glDepthFunc(GL_LESS);
    terrain->Render();
    terrain->RenderWater();

    glfwSwapBuffers(window);
    
    input_manager->ProcessLocalInput(window, deltaTime);
  
    glfwPollEvents();
  }
  
  return 0;
}

//cam.SetHeight(cMap->getHeightAt(int((floorf(current_pos.z/TerrainRenderer::TILE_SIZE)*TerrainRenderer::WORLD_SIZE)+floorf(current_pos.x/TerrainRenderer::TILE_SIZE))) + 200.f);
//float real_fps = 1000.f / (float)TimeDt;

/*
 // Process AI
 std::unique_ptr<CE_Allosaurus> allo(new CE_Allosaurus(cFileLoad.get(), "ALLO.CAR"));
 allo->setScale(2.f);
 allo->intelligence->think(TimeDt);
 allo->render();
 */

/*
 mTrans.GetScale()->x = 0.005;
 mTrans.GetScale()->y = 0.005;
 mTrans.GetScale()->z = 0.005;
 mTrans.GetPos()->z = 190.f; //20 yards (60ft) , dino at 45mph hits character in 1 second, 13 seconds for human walk
 shader.Update(mTrans, cam);
 allG->Draw();
 
 Transform mTrans_c(glm::vec3(100.f,0,310.f), glm::vec3(0,180.f,0), glm::vec3(0.25f, 0.25f, 0.25f));
 mTrans_c.GetScale()->x = 0.005;
 mTrans_c.GetScale()->y = 0.005;
 mTrans_c.GetScale()->z = 0.005;
 shader.Update(mTrans_c, cam);
 tree_plant->render();
 
 Transform mTrans_b(glm::vec3(-25.f,0,60.f), glm::vec3(0,180.f,0), glm::vec3(0.25f, 0.25f, 0.25f));
 mTrans_b.GetScale()->x = 0.005;
 mTrans_b.GetScale()->y = 0.005;
 mTrans_b.GetScale()->z = 0.005;
 shader.Update(mTrans_b, cam);
 tree_plant->render();
 
 */

