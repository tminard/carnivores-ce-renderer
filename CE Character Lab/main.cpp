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
  window = glfwCreateWindow(800, 600, "Hello World", NULL, NULL);
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

  
  C2WorldModel* plant = cMapRsc->getWorldModel(0);
  C2WorldModel* tree_plant = cMapRsc->getWorldModel(12);
  C2Geometry* allG = allo->getCurrentModelForRender();
  
  Shader shader("/Users/tminard/Source/CE Character Lab/CE Character Lab/basicShader");
  Shader t_shader("/Users/tminard/Source/CE Character Lab/CE Character Lab/terrain");
  //1024.0f / 768.0f
  //60.f, 1.7777777777777777777777777777778f, 1.f, 150000.f );
  Camera cam(glm::vec3(0.f,6.f,-6.f), 70.f, 1024.0f / 768.0f, 0.1f, 350.f);
  Transform mTrans(glm::vec3(0,0,0), glm::vec3(0,90.f,0), glm::vec3(0.25f, 0.25f, 0.25f));
  
  TerrainRenderer* ter = new TerrainRenderer();
  
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
    /*mTrans.GetScale()->x = 0.005;
    mTrans.GetScale()->y = 0.005;
    mTrans.GetScale()->z = 0.005;*/
    mTrans.GetPos()->z = 15.f; //20 yards (60ft) , dino at 45mph hits character in 1 second, 13 seconds for human walk
    shader.Update(mTrans, cam);
    //allG->Draw();

    Transform mTrans_c(glm::vec3(100.f,0,310.f), glm::vec3(0,180.f,0), glm::vec3(0.25f, 0.25f, 0.25f));
    /*mTrans_c.GetScale()->x = 0.005;
    mTrans_c.GetScale()->y = 0.005;
    mTrans_c.GetScale()->z = 0.005;*/
    shader.Update(mTrans_c, cam);
    tree_plant->render();

    Transform mTrans_b(glm::vec3(-1.f,0,60.f), glm::vec3(0,180.f,0), glm::vec3(0.25f, 0.25f, 0.25f));
    /*mTrans_b.GetScale()->x = 0.005;
    mTrans_b.GetScale()->y = 0.005;
    mTrans_b.GetScale()->z = 0.005;*/
    shader.Update(mTrans_b, cam);
    plant->render();
    
    /* Render loop*/
    
    // Render things in order of back to front
    // Render sky
    // Render characters
    // Render models
    // Render terrain
    
    
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


