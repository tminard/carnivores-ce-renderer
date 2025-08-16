#pragma once

#include <stdio.h>
#include <iostream>
#include <memory>
#include <map>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class CELocalPlayerController;

class CEUIRenderer;

class LocalInputManager
{
private:
    std::shared_ptr<CELocalPlayerController> m_player_controller;
    CEUIRenderer* m_ui_renderer = nullptr;
    
    std::map<int, int> m_last_key_state;
    std::map<int, int> m_last_mouse_state;
    
    bool m_should_shutdown = false;
    bool m_wireframe = false; // TODO: move this to a LocalVideoManager
    bool m_show_bounding_boxes = false; // Toggle for debug bounding box visualization
    double lastTime = 0.0;
public:
    void Bind(std::shared_ptr<CELocalPlayerController> player_controller);
    void BindUIRenderer(CEUIRenderer* ui_renderer);
    void ProcessLocalInput(GLFWwindow* window, float time_delta);
    void cursorPosCallback(GLFWwindow* window, double x, double y);
    bool GetShouldShutdown() {
        return m_should_shutdown;
    }
    
    bool GetShowBoundingBoxes() {
        return m_show_bounding_boxes;
    }
};
