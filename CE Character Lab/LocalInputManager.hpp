#pragma once

#include <stdio.h>
#include <iostream>
#include <memory>
#include <map>
#include <GLFW/glfw3.h>

class CEPlayer;

class LocalInputManager
{
private:
    std::shared_ptr<CEPlayer> m_player;
    
    std::map<int, int> m_last_key_state;
    
    bool m_wireframe = false; // TODO: move this to a LocalVideoManager
    double lastTime;
public:
    void Bind(std::shared_ptr<CEPlayer> &player);
    void ProcessLocalInput(GLFWwindow* window, float time_delta);
    void cursorPosCallback(GLFWwindow* window, double x, double y);
};
