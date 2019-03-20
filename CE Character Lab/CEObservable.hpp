#pragma once

#include <glm/glm.hpp>
#include <memory>
#include "camera.h"

class CEObservable {
    Camera m_camera;
public:
    CEObservable();
    
    glm::vec3 virtual getPosition();
    Camera* getCamera();
};
