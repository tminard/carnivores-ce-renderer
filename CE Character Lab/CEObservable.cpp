//
//  CEObservable.cpp
//  CE Character Lab
//
//  Created by Minard, Tyler on 6/8/18.
//  Copyright © 2018 Tyler Minard. All rights reserved.
//

#include "CEObservable.hpp"

CEObservable::CEObservable() : m_camera(glm::vec3(0.f), 45.f, (21.f/9.f), 128.f, MAX_VIEW_R)
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
