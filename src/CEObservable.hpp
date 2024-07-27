#pragma once

#include <glm/vec3.hpp>
#include <memory>
#include "camera.h"

class CEObservable {
protected:
  Camera m_camera;

public:
  CEObservable();

  glm::vec3 virtual getPosition();
  Camera* getCamera();
};
