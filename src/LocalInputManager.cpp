#include "LocalInputManager.hpp"
#include "camera.h"
#include "CELocalPlayerController.hpp"

void LocalInputManager::Bind(std::shared_ptr<CELocalPlayerController> player_controller)
{
  this->m_player_controller = player_controller;
  this->lastTime = glfwGetTime();
}

float horizontalAngle = glm::radians(3.14f);
float verticalAngle = 0.0f;
bool first = true;

void LocalInputManager::cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
  if (this->m_player_controller) {
    float mouseSpeed = 180.0f;
    float deltaTime = glfwGetTime() - this->lastTime;
    
    // Mouse
    int winw, winh;
    glfwGetWindowSize(window, &winw, &winh);
    
    // process mouse movement
    int start_x, start_y;

    if (first) {
      first = false;
      
      start_x = xpos;
      start_y = ypos;
    } else {
      start_x = winw / 2;
      start_y = winh / 2;
    }

    horizontalAngle += (mouseSpeed * deltaTime * float( start_x - xpos )); // Yaw
    verticalAngle += (mouseSpeed * deltaTime * float( start_y - ypos )); // Pitch

    if (verticalAngle > 87.f) verticalAngle = 87.f;
    if (verticalAngle < -85.f) verticalAngle = -85.f;

    glm::vec3 direction(
                        cos(glm::radians(verticalAngle)) * sin(glm::radians(horizontalAngle)),
                        sin(glm::radians(verticalAngle)),
                        cos(glm::radians(verticalAngle)) * cos(glm::radians(horizontalAngle))
                        );
    
    m_player_controller->lookAt(direction);
    glfwSetCursorPos(window, winw / static_cast<double>(2), winh / static_cast<double>(2));
  }
}

void LocalInputManager::ProcessLocalInput(GLFWwindow* window, float deltaTime)
{
  if (this->m_player_controller) {
    this->lastTime = glfwGetTime();

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        this->m_should_shutdown = true;
    }

    if (glfwGetKey(window, GLFW_KEY_UP ) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
      m_player_controller->moveForward();
    }
    
    if (glfwGetKey(window, GLFW_KEY_DOWN ) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
      m_player_controller->moveBackward();
    }
    
    if (glfwGetKey(window, GLFW_KEY_RIGHT ) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
      m_player_controller->strafeRight();
    }
    
    if (glfwGetKey(window, GLFW_KEY_LEFT ) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
      m_player_controller->strafeLeft();
    }
    
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS && this->m_last_key_state[GLFW_KEY_O] != GLFW_PRESS) {
      this->m_last_key_state[GLFW_KEY_O] = GLFW_PRESS;
      m_player_controller->DBG_printLocationInformation();
    } else if (glfwGetKey(window, GLFW_KEY_O) == GLFW_RELEASE) {
      this->m_last_key_state[GLFW_KEY_O] = GLFW_RELEASE;
    }

    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && this->m_last_key_state[GLFW_KEY_P] != GLFW_PRESS) {
      this->m_last_key_state[GLFW_KEY_P] = GLFW_PRESS;
      
      this->m_wireframe = !this->m_wireframe;
      
      if (this->m_wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      }
    } else if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE) {
      this->m_last_key_state[GLFW_KEY_P] = GLFW_RELEASE;
    }
  }
}
