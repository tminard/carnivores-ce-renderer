#pragma once

#include <iostream>

#include "glad/glad.h"
#include <GLFW/glfw3.h>

class LocalVideoManager
{
private:
    GLFWwindow* m_main_window;
    
    void initGLFW(bool fullscreen);
    void initGL();
    void destroyGLFW();
    void printStats();
public:
    LocalVideoManager(bool fullscreen);
    ~LocalVideoManager();
    
    GLFWwindow* GetWindow();
};
