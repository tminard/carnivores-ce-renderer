#include "CEObservable.hpp"

// Adjusted far plane for 16x world scale reduction (was 536870912.0f)
// Near plane scaled down proportionally to maintain depth precision
CEObservable::CEObservable() : m_camera(glm::vec3(0.f), glm::radians(80.f), AspectRatio::Standard, 0.0625f, 33554432.0f)
{
}

glm::vec3 CEObservable::getPosition()
{
    return getCamera()->GetPosition();
}

Camera* CEObservable::getCamera()
{
    return &this->m_camera;
}
