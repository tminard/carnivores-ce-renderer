#pragma once

#include <stdio.h>
#include <iostream>
#include <memory>
#include <map>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class CELocalPlayerController;

class LocalInputManager
{
private:
    std::shared_ptr<CELocalPlayerController> m_player_controller;
    
    std::map<int, int> m_last_key_state;
    
    bool m_wireframe = false; // TODO: move this to a LocalVideoManager
    double lastTime = 0.0;
public:
    void Bind(std::shared_ptr<CELocalPlayerController> player_controller);
    void ProcessLocalInput(GLFWwindow* window, float time_delta);
    void cursorPosCallback(GLFWwindow* window, double x, double y);
};
