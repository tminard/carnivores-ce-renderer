//
//  main.cpp
//  CE Character Lab
//
//  Created by Tyler Minard on 8/6/15.
//  Copyright (c) 2015 Tyler Minard. All rights reserved.
//

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

#include "CE_Allosaurus.h"
#include "C2CarFile.h"
#include "C2CarFilePreloader.h"

#include "Hunt.h"
#include "C2Geometry.h"
#include "C2Texture.h"

#include "CE_ArtificialIntelligence.h"

#include "g_shared.h"
#include <chrono>

#include "C2MapFile.h"
#include "C2MapRscFile.h"

#include "C2WorldModel.h"

#include "shader.h"

#include "TerrainRenderer.h"

void CreateFadeTab();
unsigned int timeGetTime();
WORD  FadeTab[65][0x8000];

template <typename T, typename... Args>
auto make_unique(Args&&... args) -> std::unique_ptr<T>
{
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

int main(int argc, const char * argv[])
{
  /* Load Resources */
  CreateFadeTab();

  std::unique_ptr<C2CarFilePreloader> cFileLoad(new C2CarFilePreloader);

  /* Load rendering */
  if (!glfwInit())
    return -1;

  glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
  glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 2 );
  glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
  glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
  
  GLFWwindow* window;
  window = glfwCreateWindow(800, 600, "Carnivores Renderer", NULL, NULL);
  if (!window)
  {
    glfwTerminate();
    return -1;
  }
  
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glDisable(GL_CULL_FACE);
  
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glClearColor(.5,.5,.5,0);
  glFrontFace (GL_CCW); // GL_CCW for counter clock-wise
  
  int major, minor, rev;
  glfwGetVersion(&major, &minor, &rev);
  std::cout << "OpenGL - " << major << "." << minor << "." << rev << std::endl;
  std::cout << glfwGetVersionString() << std::endl;
  const GLubyte* renderer = glGetString(GL_RENDERER);
  std::cout << renderer << " : " << glGetString(GL_VERSION) << std::endl;
  
  int RealTime, PrevTime, TimeDt;
  PrevTime = timeGetTime();
  
  std::unique_ptr<CE_Allosaurus> allo(new CE_Allosaurus(cFileLoad.get(), "/Users/tminard/Source/CE Character Lab/CE Character Lab/ALLO.CAR"));
  allo->setScale(2.f);
  std::unique_ptr<C2MapFile> cMap(new C2MapFile("/Users/tminard/Source/CE Character Lab/CE Character Lab/AREA1.MAP"));
  std::unique_ptr<C2MapRscFile> cMapRsc(new C2MapRscFile("/Users/tminard/Source/CE Character Lab/CE Character Lab/AREA1.RSC"));

  // C2Geometry* allG = allo->getCurrentModelForRender();

  std::cout << "Map texture atlas width: " << cMapRsc->getTextureAtlasWidth();
  
  Shader shader("/Users/tminard/Source/CE Character Lab/CE Character Lab/basicShader");
  Shader t_shader("/Users/tminard/Source/CE Character Lab/CE Character Lab/terrain");

  //60.f, 1.7777777777777777777777777777778f, 1.f, 150000.f );

  float VIEW_R = 100.5f * cMap->getTileLength();
  Camera cam(glm::vec3(0.f,15.f,0), 45.f, (float)800.f / (float)600.f, 0.1f, VIEW_R);
  //Transform mTrans(glm::vec3(0,0,0), glm::vec3(0,90.f,0), glm::vec3(0.25f, 0.25f, 0.25f));
  
  TerrainRenderer* ter = new TerrainRenderer(cMap.get(), cMapRsc.get());
  bool wireframe_mode = false;
  
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
    
    glm::vec3 current_pos = cam.GetCurrentPos();

    // Process AI
    allo->intelligence->think(TimeDt);

    /* Render here */
    float ratio;
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    ratio = width / (float) height;
    glViewport(0, 0, width, height);
  
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    t_shader.Bind();
    Transform mTrans_land(glm::vec3(0,0,0), glm::vec3(0,0,0), glm::vec3(1.f, 1.f, 1.f));
    t_shader.Update(mTrans_land, cam);
    cMapRsc->getTexture(0)->Use();

    ter->Render();

    shader.Bind();
    allo->render();

    // loop through visible objects
    float cur_x = current_pos.x;
    float cur_y = current_pos.z;
    int current_row = static_cast<int>(cur_y / cMap->getTileLength());
    int current_col = static_cast<int>(cur_x / cMap->getTileLength());
    int view_distance_squares = (VIEW_R / cMap->getTileLength()) / 2;


    // For each row/col, decide whether or not to draw
//    for (int view_row = current_row + view_distance_squares; view_row > (current_row - view_distance_squares); view_row--) {
//      for (int view_col = current_col - view_distance_squares; view_col < current_col + view_distance_squares; view_col++) {
//        int obj_id = cMap->getObjectAt(((view_row)*cMap->getWidth())+view_col);
//        
//        if (obj_id != 255 && obj_id != 254) {
//          C2WorldModel* w_obj = cMapRsc->getWorldModel(obj_id);
//          int obj_height = cMap->getHeightAt((view_row*cMap->getWidth()) + view_col);
//          Transform mTrans_c(glm::vec3(view_col*cMap->getTileLength(),obj_height,view_row*cMap->getTileLength()), glm::vec3(0,0,0), glm::vec3(2.f, 2.f, 2.f));
//          shader.Update(mTrans_c, cam);
//          w_obj->render();
//        }
//      }
//    }

    /*
    //int view_range = int(VIEW_R);
    for (int vy = cur_y+(VIEW_R/TerrainRenderer::TILE_SIZE); vy > cur_y-(VIEW_R/TerrainRenderer::TILE_SIZE); vy -= TerrainRenderer::TILE_SIZE) {
      for (int x = cur_x-(VIEW_R/TerrainRenderer::TILE_SIZE); x < cur_x+(VIEW_R/TerrainRenderer::TILE_SIZE); x += TerrainRenderer::TILE_SIZE) {
        int obj_id = cMap->getObjectAt(((vy/TerrainRenderer::TILE_SIZE)*TerrainRenderer::WORLD_SIZE)+(x/TerrainRenderer::TILE_SIZE));
        
        if (obj_id != 255 && obj_id != 254) {
          C2WorldModel* w_obj = cMapRsc->getWorldModel(obj_id);
          int obj_height = cMap->getHeightAt(int((floorf(vy/TerrainRenderer::TILE_SIZE)*TerrainRenderer::WORLD_SIZE)+floorf(x/TerrainRenderer::TILE_SIZE)));
          Transform mTrans_c(glm::vec3(x,obj_height,vy), glm::vec3(0,0,0), glm::vec3(0.125f, 0.125f, 0.125f));
          shader.Update(mTrans_c, cam);
          w_obj->render();
        }
      }
    }*/
    
    /* Render loop*/
    
    // Render things in order of back to front
    // Render sky
    // Render characters
    // Render models
    // Render terrain
    
    float FLY_SPEED = 150.f;//10.5f;
    
    if (glfwGetKey(window, GLFW_KEY_UP ) == GLFW_PRESS) {
      cam.MoveForward(FLY_SPEED);
    }
    
    if (glfwGetKey(window, GLFW_KEY_DOWN ) == GLFW_PRESS) {
      //current_pos.z -= 1.125f;
      cam.MoveForward(-(FLY_SPEED));
    }
    
    if (glfwGetKey(window, GLFW_KEY_RIGHT ) == GLFW_PRESS) {
      //current_pos.x -= 1.125f;
      cam.MoveRight(-(FLY_SPEED));
    }
    
    if (glfwGetKey(window, GLFW_KEY_LEFT ) == GLFW_PRESS) {
      //current_pos.x += 1.125f;
      cam.MoveRight(FLY_SPEED);
    }
    
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
      cam.RotateY(0.025f);
    }
    
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
      cam.RotateY(-0.025f);
    }
    
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
      cam.MoveUp(FLY_SPEED);
    }
    
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
      cam.MoveUp(-FLY_SPEED);
    }

    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
      wireframe_mode = !wireframe_mode;

      if (wireframe_mode) {
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
      } else {
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
      }
    }

    //cam.SetHeight(cMap->getHeightAt(int((floorf(current_pos.z/TerrainRenderer::TILE_SIZE)*TerrainRenderer::WORLD_SIZE)+floorf(current_pos.x/TerrainRenderer::TILE_SIZE))) + 200.f);
    
    
    /* Swap front and back buffers */
    glfwSwapBuffers(window);
    
    /* Poll for and process events */
    glfwPollEvents();
  }
  
  glfwDestroyWindow(window);
  glfwTerminate();
  
  delete ter;

  return 0;
}


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


