  //
  //  LocalVideoManager.cpp
  //  CE Character Lab
  //
  //  Created by Minard, Tyler on 6/8/18.
  //  Copyright © 2018 Tyler Minard. All rights reserved.
  //

#include "LocalVideoManager.hpp"

LocalVideoManager::LocalVideoManager()
{
  initGLFW();
  initGL();
}

void LocalVideoManager::printStats()
{
  int major, minor, rev;
  glfwGetVersion(&major, &minor, &rev);
  std::cout << "OpenGL - " << major << "." << minor << "." << rev << std::endl;
  std::cout << glfwGetVersionString() << std::endl;
  const GLubyte* renderer = glGetString(GL_RENDERER);
  std::cout << renderer << " : " << glGetString(GL_VERSION) << std::endl;
  std::cout << "=== Supported Extensions ===" << std::endl;

  if (glfwExtensionSupported("GL_EXT_texture_array")) {
    std::cout << "\t GL_EXT_texture_array :: true" << std::endl;
  } else {
    std::cout << "\t GL_EXT_texture_array :: false" << std::endl;
  }
}

GLFWwindow* LocalVideoManager::GetWindow()
{
  return this->m_main_window;
}

void LocalVideoManager::initGLFW()
{
  if (!glfwInit()) {
    throw;
  }

  glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
  glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
  glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
  glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
  glfwWindowHint( GLFW_SAMPLES, 4);

  GLFWmonitor* primary_monitor = glfwGetPrimaryMonitor();
  const GLFWvidmode* v_mode = glfwGetVideoMode(primary_monitor);

  std::cout << "== Primary Monitor detected == " << std::endl;
  std::cout << "\t[Video Mode] width: " << v_mode->width << "; height: " << v_mode->height << std::endl;

  this->m_main_window = glfwCreateWindow(v_mode->width, v_mode->height, "Carnivores Renderer", primary_monitor, NULL);

  if (!this->m_main_window) {
    throw;
  }

  glfwMakeContextCurrent(this->m_main_window);
  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
  
  glfwSwapInterval(1);

  this->initGL();
}

void LocalVideoManager::initGL()
{
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  glDisable(GL_CULL_FACE);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glClearColor(0.94f, 0.97f, 1.f, 0);


  this->printStats();
}

void LocalVideoManager::destroyGLFW()
{
  if (this->m_main_window) {
    glfwDestroyWindow(this->m_main_window);
  }
  glfwTerminate();
}

LocalVideoManager::~LocalVideoManager()
{
  std::cout << "\nDestroying video manager..." << '\n';
  destroyGLFW();
}
