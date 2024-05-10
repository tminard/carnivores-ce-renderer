#include "CEObservable.hpp"

CEObservable::CEObservable() : m_camera(glm::vec3(0.f), 45.f, Standard, 0.1f, MAX_VIEW_R)
{
}

float CEObservable::getMaxViewRange()
{
    return MAX_VIEW_R;
}

glm::vec3 CEObservable::getPosition()
{
    return getCamera()->GetCurrentPos();
}

Camera* CEObservable::getCamera()
{
    return &this->m_camera;
}
