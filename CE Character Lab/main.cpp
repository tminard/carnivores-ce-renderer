//
//  main.cpp
//  CE Character Lab
//
//  Created by Tyler Minard on 8/6/15.
//  Copyright (c) 2015 Tyler Minard. All rights reserved.
//

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

#include <iostream>

#include "CE_Allosaurus.h"
#include "C2CarFile.h"
#include "C2CarFilePreloader.h"

#include "Hunt.h"
#include "C2Geometry.h"

#include "CE_ArtificialIntelligence.h"

#include "g_shared.h"
#include <chrono>

#include "C2MapFile.h"
#include "C2MapRscFile.h"

#include "shader.h"

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
  GLFWwindow* window;
  if (!glfwInit())
    return -1;

  /*glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
  glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 2 );
  glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );*/

  int major, minor, rev;
  glfwGetVersion(&major, &minor, &rev);
  std::cout << "OpenGL - " << major << "." << minor << "." << rev << std::endl;
  
  window = glfwCreateWindow(800, 600, "Hello World", NULL, NULL);
  if (!window)
  {
    glfwTerminate();
    return -1;
  }
  
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);
  
  int RealTime, PrevTime, TimeDt;
  PrevTime = timeGetTime();
  
  std::unique_ptr<C2MapFile> cMap(new C2MapFile("/Users/tminard/Source/CE Character Lab/CE Character Lab/AREA1.MAP"));
  std::unique_ptr<C2MapRscFile> cMapRsc(new C2MapRscFile("/Users/tminard/Source/CE Character Lab/CE Character Lab/AREA1.RSC"));

  std::unique_ptr<CE_Allosaurus> allo(new CE_Allosaurus(cFileLoad.get(), "/Users/tminard/Source/CE Character Lab/CE Character Lab/ALLO.CAR"));
  allo->setScale(2.f);
  C2Geometry* allG = allo->getCurrentModelForRender();
  
  Shader shader("/Users/tminard/Source/CE Character Lab/CE Character Lab/basicShader");
  
  while (!glfwWindowShouldClose(window))
  {
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
  
    glClear(GL_COLOR_BUFFER_BIT);

    shader.Bind();
    allG->Render();
    
    /* Swap front and back buffers */
    glfwSwapBuffers(window);
    
    /* Poll for and process events */
    glfwPollEvents();
  }
  
  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}


