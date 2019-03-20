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
    }
    
    inline glm::mat4 GetVM()
    {
        return glm::lookAt(pos, pos + forward, up);
    }
    
    inline glm::mat4 GetViewProjection() const
    {
        return projection * glm::lookAt(pos, pos + forward, up);
    }
    
    inline float GetViewDistance() const
    {
        return this->m_view_distance;
    }
    
    void SetPos(const glm::vec3& new_pos)
    {
        this->pos = new_pos;
    }
    
    const glm::vec3& GetCurrentPos()
    {
        return this->pos;
    }
    
    void MoveForward(float amt)
    {
        pos += forward * amt;
    }
    
    void MoveUp(float amt)
    {
        pos += up * amt;
    }
    
    void MoveRight(float amt)
    {
        pos += glm::cross(up, forward) * amt;
    }
    
    void SetHeight(float height) {
        pos.y = height;
    }
    
    //void Pitch(float angle)
    //{
    //	glm::vec3 right = glm::normalize(glm::cross(up, forward));
    
    //	forward = glm::vec3(glm::normalize(glm::rotate(angle, right) * glm::vec4(forward, 0.0)));
    //	up = glm::normalize(glm::cross(forward, right));
    //}
    
    void SetLookAt(glm::vec3 fw) {
        this->forward = fw;
    }
    
    void RotateY(float angle)
    {
        static const glm::vec3 UP(0.0f, 1.0f, 0.0f);
        
        glm::mat4 rotation = glm::rotate(angle, UP);

        this->forward = glm::vec3(rotation * glm::vec4(this->forward, 0.0));
        this->up = glm::vec3(rotation * glm::vec4(this->up, 0.0));
    }
    
protected:
private:
    float m_view_distance;
    glm::mat4 projection;
    glm::vec3 pos;
    glm::vec3 forward;
    glm::vec3 up;
};

#endif
