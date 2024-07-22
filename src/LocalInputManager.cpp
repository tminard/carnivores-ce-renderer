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
    static double lastX = xpos, lastY = ypos;

    if (this->m_player_controller) {
        float mouseSpeed = 0.1f;  // Adjusted speed for more sensitivity control
        float deltaTime = glfwGetTime() - this->lastTime;

        if (first) {
            lastX = xpos;
            lastY = ypos;
            first = false;
        }

        // Calculate the difference in mouse positions
        double deltaX = xpos - lastX;
        double deltaY = ypos - lastY;

        // Update last positions
        lastX = xpos;
        lastY = ypos;

        // Update angles based on mouse movement
        horizontalAngle -= mouseSpeed * deltaX;
        verticalAngle -= mouseSpeed * deltaY;

        if (verticalAngle > 87.0f) verticalAngle = 87.0f;
        if (verticalAngle < -85.0f) verticalAngle = -85.0f;

        glm::vec3 direction(
            cos(glm::radians(verticalAngle)) * sin(glm::radians(horizontalAngle)),
            sin(glm::radians(verticalAngle)),
            cos(glm::radians(verticalAngle)) * cos(glm::radians(horizontalAngle))
        );

        m_player_controller->lookAt(direction);

        // Center the cursor after processing movement
        int winw, winh;
        glfwGetWindowSize(window, &winw, &winh);
        glfwSetCursorPos(window, winw / 2.0, winh / 2.0);
        lastX = winw / 2.0;
        lastY = winh / 2.0;
    }
}

void LocalInputManager::ProcessLocalInput(GLFWwindow* window, float deltaTime)
{
    double currentTime = glfwGetTime();
    double timeDelta = currentTime - this->lastTime;
    if (this->m_player_controller) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            this->m_should_shutdown = true;
        }
      
      bool forwardPressed = glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
      bool backwardPressed = glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
      bool rightPressed = glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
      bool leftPressed = glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;

      m_player_controller->move(currentTime, timeDelta, forwardPressed, backwardPressed, rightPressed, leftPressed);
      
      if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && this->m_last_key_state[GLFW_KEY_SPACE] != GLFW_PRESS) {
          this->m_last_key_state[GLFW_KEY_SPACE] = GLFW_PRESS;
          m_player_controller->jump(currentTime);
      } else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
        this->m_last_key_state[GLFW_KEY_SPACE] = GLFW_RELEASE;
      }
        if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS && this->m_last_key_state[GLFW_KEY_O] != GLFW_PRESS) {
            this->m_last_key_state[GLFW_KEY_O] = GLFW_PRESS;
            m_player_controller->DBG_printLocationInformation();
        }
        else if (glfwGetKey(window, GLFW_KEY_O) == GLFW_RELEASE) {
            this->m_last_key_state[GLFW_KEY_O] = GLFW_RELEASE;
        }

        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && this->m_last_key_state[GLFW_KEY_P] != GLFW_PRESS) {
            this->m_last_key_state[GLFW_KEY_P] = GLFW_PRESS;

            this->m_wireframe = !this->m_wireframe;

            if (this->m_wireframe) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }
            else {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
        }
        else if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE) {
            this->m_last_key_state[GLFW_KEY_P] = GLFW_RELEASE;
        }
    }
  
  this->lastTime = currentTime;
}
