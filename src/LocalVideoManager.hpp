#pragma once

#include <iostream>

#include "glad/glad.h"
#include <GLFW/glfw3.h>

class LocalVideoManager
{
private:
    GLFWwindow* m_main_window;
    
    void initGLFW();
    void initGL();
    void destroyGLFW();
    void printStats();
public:
    LocalVideoManager();
    ~LocalVideoManager();
    
    GLFWwindow* GetWindow();
};
