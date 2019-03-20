#include "LocalInputManager.hpp"
#include "camera.h"
#include "CEPlayer.hpp"

void LocalInputManager::Bind(std::shared_ptr<CEPlayer> &player)
{
    this->m_player.swap(player);
    this->lastTime = glfwGetTime();
}

float horizontalAngle = 3.14f;
float verticalAngle = 0.0f;

void LocalInputManager::cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (this->m_player) {
        Camera* m_following = m_player->getCamera();
        float mouseSpeed = 1.0f;
        float deltaTime = glfwGetTime() - this->lastTime;
        
        // Mouse
        int winw, winh;
        glfwGetWindowSize(window, &winw, &winh);
        
        // process mouse movement
        
        horizontalAngle += mouseSpeed * deltaTime * float( winw/2 - xpos );
        verticalAngle   += mouseSpeed * deltaTime * float( winh/2 - ypos );
        glm::vec3 direction(
                            cos(verticalAngle) * sin(horizontalAngle),
                            sin(verticalAngle),
                            cos(verticalAngle) * cos(horizontalAngle)
                            );
        // Right vector
//        glm::vec3 right = glm::vec3(
//                                    sin(horizontalAngle - 3.14f/2.0f),
//                                    0,
//                                    cos(horizontalAngle - 3.14f/2.0f)
//                                    );
        //glm::vec3 up = glm::cross( right, direction );
        
        m_following->SetLookAt(direction);
        glfwSetCursorPos(window, winw/2, winh/2);
    }
}

void LocalInputManager::ProcessLocalInput(GLFWwindow* window, float deltaTime)
{
    if (this->m_player) {
        this->lastTime = glfwGetTime();
        Camera* m_following = m_player->getCamera();
        float FLY_SPEED = 150.f;
        
        // Keyboard
        
        if (glfwGetKey(window, GLFW_KEY_UP ) == GLFW_PRESS) {
            m_following->MoveForward(FLY_SPEED);
        }
        
        if (glfwGetKey(window, GLFW_KEY_DOWN ) == GLFW_PRESS) {
            m_following->MoveForward(-(FLY_SPEED));
        }
        
        if (glfwGetKey(window, GLFW_KEY_RIGHT ) == GLFW_PRESS) {
            m_following->MoveRight(-(FLY_SPEED));
        }
        
        if (glfwGetKey(window, GLFW_KEY_LEFT ) == GLFW_PRESS) {
            m_following->MoveRight(FLY_SPEED);
        }
        
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            m_following->RotateY(0.025f);
        }
        
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            m_following->RotateY(-0.025f);
        }
        
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            m_following->MoveUp(FLY_SPEED);
        }
        
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            m_following->MoveUp(-FLY_SPEED);
        }
        
        if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS && this->m_last_key_state[GLFW_KEY_O] != GLFW_PRESS) {
            this->m_last_key_state[GLFW_KEY_O] = GLFW_PRESS;
            glm::vec3 cur_pos = m_following->GetCurrentPos();
            glm::vec2 cur_world_pos = m_player->getWorldPosition();
            
            std::cout << '\n' << "===> CURRENT POSITION IN SPACE <===" <<
                '\n' << "X: " << std::to_string(cur_pos.x) <<
                '\n' << "Y: " << std::to_string(cur_pos.y) <<
                '\n' << "Z: " << std::to_string(cur_pos.z);
            
            std::cout << '\n' << "===> CURRENT WORLD POSITION <===" <<
                '\n' << "X: " << std::to_string(cur_world_pos.x) <<
                '\n' << "Y: " << std::to_string(cur_world_pos.y);
            
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
