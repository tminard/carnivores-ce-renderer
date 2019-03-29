//
//  CEObservable.cpp
//  CE Character Lab
//
//  Created by Minard, Tyler on 6/8/18.
//  Copyright © 2018 Tyler Minard. All rights reserved.
//

#include "CEObservable.hpp"

CEObservable::CEObservable() : m_camera(glm::vec3(82279.875000f,2638.347168f,33472.246094f), 45.f, (4.f/3.f), 1.f, VIEW_R)
{
}

float CEObservable::getViewRange()
{
    return VIEW_R;
}

glm::vec3 CEObservable::getPosition()
{
    return getCamera()->GetCurrentPos();
}

Camera* CEObservable::getCamera()
{
    return &this->m_camera;
}
