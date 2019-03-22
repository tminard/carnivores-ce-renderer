#ifndef TRANSFORM_INCLUDED_H
#define TRANSFORM_INCLUDED_H

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "camera.h"

struct Transform
{
public:
  Transform(const glm::vec3& pos = glm::vec3(), const glm::vec3& rot = glm::vec3(), const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f))
  {
    this->pos = pos;
    this->rot = rot;
    this->scale = scale;
    this->precalc_model = GetModel();
  }

  inline glm::mat4 GetStaticModel()
  {
    return this->precalc_model;
  }
  
  inline glm::mat4 GetStaticModelVP(Camera& camera)
  {
    glm::mat4 VP = camera.GetViewProjection();
    glm::mat4 M = this->precalc_model;

    return VP * M;
  }
  
  inline glm::mat4 GetMVP(Camera& camera)
  {
    return GetStaticModelVP(camera);
  }
  
  inline glm::vec3* GetPos() { return &pos; }
  inline glm::vec3* GetRot() { return &rot; }
  inline glm::vec3* GetScale() { return &scale; }
protected:
private:
  glm::vec3 pos;
  glm::vec3 rot;
  glm::vec3 scale;
  glm::mat4 precalc_model;
  
  inline glm::mat4 GetModel()
  {
    glm::mat4 posMat = glm::translate(pos);
    glm::mat4 scaleMat = glm::scale(scale);
    glm::mat4 rotX = glm::rotate(rot.x, glm::vec3(1.0, 0.0, 0.0));
    glm::mat4 rotY = glm::rotate(rot.y, glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 rotZ = glm::rotate(rot.z, glm::vec3(0.0, 0.0, 1.0));
    glm::mat4 rotMat = rotX * rotY * rotZ;
    
    glm::mat4 model = posMat * rotMat * scaleMat;
    
    return model;
  }
};

#endif
