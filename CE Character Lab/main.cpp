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

#include "C2WorldModel.h"

#include "shader.h"

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

struct ObjLoc {
    C2WorldModel* mModel;
    Transform mTrans;
    bool isFar;
};

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    input_manager->cursorPosCallback(window, xpos, ypos);
}

int main(int argc, const char * argv[])
{
    CreateFadeTab();
    std::unique_ptr<C2CarFilePreloader> cFileLoad(new C2CarFilePreloader);
    std::unique_ptr<LocalVideoManager> video_manager(new LocalVideoManager());
    std::shared_ptr<C2MapFile> cMap(new C2MapFile("resources/AREA1.MAP"));
    std::unique_ptr<C2MapRscFile> cMapRsc(new C2MapRscFile("resources/AREA1.RSC"));
    std::shared_ptr<CEPlayer> m_player(new CEPlayer(cMap));
    std::unique_ptr<TerrainRenderer> terrain(new TerrainRenderer(cMap.get(), cMapRsc.get()));
    
    std::cout << "Map texture atlas width: " << cMapRsc->getTextureAtlasWidth();
    
    Shader shader("resources/basicShader");

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
    
    std::vector<ObjLoc> visible_objects = {};

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

        /* Render here */
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDisable(GL_DEPTH_TEST);
        cMapRsc->getDaySky()->Render(window, *camera);
        glEnable(GL_DEPTH_TEST);

        glm::vec3 current_pos = camera->GetCurrentPos();
        std::vector<ObjLoc> m_objects;
        m_objects.clear();

        if (m_objects.empty()) {
            float cur_x = current_pos.x;
            float cur_y = current_pos.z;
            int current_row = static_cast<int>(cur_y / cMap->getTileLength());
            int current_col = static_cast<int>(cur_x / cMap->getTileLength());
            int view_distance_squares = 100;
            int rendered_objects = 0;
            const int MAX_OBJECTS = 2000;
            
            for (int view_row = current_row + view_distance_squares; view_row > (current_row - view_distance_squares); view_row--) {
                for (int view_col = current_col - view_distance_squares; view_col < current_col + view_distance_squares; view_col++) {
                    if (view_col < 0 || view_row < 0) continue;
                    bool isFar = true;
                    int xy = (view_row*cMap->getWidth()) + view_col;
                    
                    if (abs(view_col - current_col) < 20 && (view_row - current_row) < 20) {
                        isFar = false;
                    }
                    
                    if (rendered_objects >= MAX_OBJECTS && !isFar) continue;
                    
                    int obj_id = cMap->getObjectAt(xy);
                    
                    if (obj_id != 255 && obj_id != 254) {
                        float map_height = cMap->getHeightAt(xy);
                        C2WorldModel* w_obj = cMapRsc->getWorldModel(obj_id);
                        float obj_height = cMap->getObjectHeightAt(xy);
                        if (obj_height == 0.f) {
                            obj_height = map_height + (w_obj->getObjectInfo()->YLo / 2.f);
                        }
                        
                        int rotation_idx = (cMap->getFlagsAt(xy) >> 2) & 3;
                        glm::vec3 rotation;
                        switch (rotation_idx) {
                            case 0:
                                rotation = glm::vec3(0, glm::radians(0.f), 0);
                                break;
                            case 1:
                                rotation = glm::vec3(0, glm::radians(90.f), 0);
                                break;
                            case 2:
                                rotation = glm::vec3(0, glm::radians(180.f), 0);
                                break;
                            case 3:
                                rotation = glm::vec3(0, glm::radians(270.f), 0);
                                break;
                        }
                        
                        Transform mTrans_c(
                                           glm::vec3(((float)(view_col)*cMap->getTileLength()) + 256.f, obj_height, ((float)(view_row)*cMap->getTileLength()) + 256.f),
                                           rotation,
                                           glm::vec3(1.f, 1.f, 1.f)
                                           );
                        if (isFar) {
                            m_objects.push_back(ObjLoc {w_obj, mTrans_c, true});
                            //glDepthFunc(GL_LEQUAL);
                            //w_obj->renderFar(mTrans_c, *camera);
                        } else {
                            //rendered_objects++;
                            m_objects.push_back(ObjLoc {w_obj, mTrans_c, false});
                            
//                            glDepthFunc(GL_LEQUAL);
//                            shader.Bind();
//                            shader.Update(mTrans_c, *camera);
//                            w_obj->render();
                        }
                    }
                }
            }
        }

        cMapRsc->getTexture(0)->Use();
        terrain->Update(mTrans_land, *camera);
        glDepthFunc(GL_LESS);
        terrain->Render();
        
        /* User Input */
        input_manager->ProcessLocalInput(window, deltaTime);
        
        if (glfwGetKey(window, GLFW_KEY_M ) == GLFW_PRESS) {
            visible_objects.clear();
        }

        /* Swap front and back buffers */
        glfwSwapBuffers(window);
        glFinish();
        
        /* Poll for and process events */
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

// loop through visible objects
//    float cur_x = current_pos.x;
//    float cur_y = current_pos.z;
//    int current_row = static_cast<int>(cur_y / cMap->getTileLength());
//    int current_col = static_cast<int>(cur_x / cMap->getTileLength());
//    int view_distance_squares = (VIEW_R / cMap->getTileLength()) / 2;


// For each row/col, decide whether or not to draw
//    for (int view_row = current_row + view_distance_squares; view_row > (current_row - view_distance_squares); view_row--) {
//      for (int view_col = current_col - view_distance_squares; view_col < current_col + view_distance_squares; view_col++) {
//        int obj_id = cMap->getObjectAt(((view_row)*cMap->getWidth())+view_col);
//
//        if (obj_id != 255 && obj_id != 254) {
//          C2WorldModel* w_obj = cMapRsc->getWorldModel(obj_id);
//          float obj_height = cMap->getHeightAt((view_row*cMap->getWidth()) + view_col);
//
//          if (obj_height == 0.0f) {
//            obj_height = cMap->getObjectHeightAt((view_row*cMap->getWidth()) + view_col);
//          }
//
//          Transform mTrans_c(glm::vec3(view_col*cMap->getTileLength(),obj_height,view_row*cMap->getTileLength()), glm::vec3(0,0,0), glm::vec3(2.f, 2.f, 2.f));
//          shader.Update(mTrans_c, cam);
//          w_obj->render();
//        }
//      }
//    }

