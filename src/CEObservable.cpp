#include "CEObservable.hpp"

// I don't yet know why such an obscenely high value is needed here to avoid z fighting... but it is
// I'll defer to folks smarter than I or just wait to understand it at some point.
// Tested with ice age maps (e.g., area #4) to verify.
CEObservable::CEObservable() : m_camera(glm::vec3(0.f), glm::radians(80.f), AspectRatio::Standard, 1.f, 536870912.0f)
{
}

glm::vec3 CEObservable::getPosition()
{
    return getCamera()->GetCurrentPos();
}

Camera* CEObservable::getCamera()
{
    return &this->m_camera;
}
