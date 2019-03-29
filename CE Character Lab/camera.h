#ifndef CAMERA_INCLUDED_H
#define CAMERA_INCLUDED_H

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

struct Camera
{
public:
  Camera(const glm::vec3& pos, float fov, float aspect, float zNear, float zFar)
  {
    // Confused? See https://learnopengl.com/#!Getting-started/Coordinate-Systems
    this->m_view_distance = zFar;
    this->pos = pos;
    this->forward = glm::vec3(0.0f, 0.0f, 1.0f);
    this->up = glm::vec3(0.0f, 1.0f, 0.0f);
    this->projection = glm::perspective(fov, aspect, zNear, zFar);
    updateViewProjection();
  }
  
  inline glm::vec3 GetPosition() const
  {
    return this->pos;
  }

  inline glm::vec3 GetUp() const
  {
    return this->up;
  }
  
  inline glm::mat4 GetVM() const
  {
    return glm::lookAt(pos, pos + forward, up);
  }
  
  inline glm::mat4 GetProjection() const
  {
    return projection;
  }
  
  inline glm::mat4 GetViewProjection() const
  {
    return view_projection;
  }
  
  inline float GetViewDistance() const
  {
    return this->m_view_distance;
  }
  
  void SetPos(const glm::vec3& new_pos)
  {
    this->pos = new_pos;
    updateViewProjection();
  }
  
  const glm::vec3& GetCurrentPos()
  {
    return this->pos;
  }
  
  void MoveForward(float amt)
  {
    pos += forward * amt;
    updateViewProjection();
  }
  
  void MoveUp(float amt)
  {
    pos += up * amt;
    updateViewProjection();
  }
  
  void MoveRight(float amt)
  {
    pos += glm::cross(up, forward) * amt;
    updateViewProjection();
  }
  
  void SetHeight(float height) {
    pos.y = height;
  }
  
  void SetLookAt(glm::vec3 fw) {
    this->forward = fw;
    updateViewProjection();
  }
  
  void RotateY(float angle)
  {
    static const glm::vec3 UP(0.0f, 1.0f, 0.0f);
    
    glm::mat4 rotation = glm::rotate(angle, UP);
    
    this->forward = glm::vec3(rotation * glm::vec4(this->forward, 0.0));
    this->up = glm::vec3(rotation * glm::vec4(this->up, 0.0));
  }
  
  inline glm::vec3 GetForward() const
  {
    return this->forward;
  }
  
protected:
private:
  float m_view_distance;
  glm::mat4 projection;
  glm::vec3 pos;
  glm::vec3 forward;
  glm::vec3 up;
  glm::mat4 view_projection;
  
  glm::vec2 world_position;
  
  inline void updateViewProjection()
  {
    this->view_projection = projection * glm::lookAt(pos, pos + forward, up);
  }
};

#endif
