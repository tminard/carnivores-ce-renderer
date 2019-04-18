#pragma once

#include <glm/glm.hpp>
#include <memory>
#include "camera.h"

const float MAX_VIEW_R = (128.f * 256.f);

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
