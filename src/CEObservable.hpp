#pragma once

#include <glm/vec3.hpp>
#include <memory>
#include "camera.h"

const float MAX_VIEW_R = (256.f * 1024.f * 2.f);

class CEObservable {
protected:
  Camera m_camera;

public:
  CEObservable();

  glm::vec3 virtual getPosition();
  Camera* getCamera();

  /*
   * The absolute farthest range before hard culling.
   * Fog and fade out should begin well before this.
   */
  float getMaxViewRange();
};
