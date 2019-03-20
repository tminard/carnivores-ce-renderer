#pragma once

#include "CEObservable.hpp"
#include <glm/glm.hpp>

#include <memory>

class C2MapFile;

class CEPlayer :
    public CEObservable
{
private:
    std::shared_ptr<C2MapFile> m_map;
public:
    CEPlayer(std::shared_ptr<C2MapFile> map);
    glm::vec2 getWorldPosition();
};
