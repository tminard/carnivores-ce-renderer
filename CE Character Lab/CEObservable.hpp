#pragma once

#include <glm/glm.hpp>
#include <memory>
#include "camera.h"

// Previous view range max was 49
const float VIEW_R = (128.f * 1024.f);

class CEObservable {
    Camera m_camera;

public:
    CEObservable();

    glm::vec3 virtual getPosition();
    Camera* getCamera();
    float getViewRange();
};