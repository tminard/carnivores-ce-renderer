//
//  CEObservable.cpp
//  CE Character Lab
//
//  Created by Minard, Tyler on 6/8/18.
//  Copyright © 2018 Tyler Minard. All rights reserved.
//

#include "CEObservable.hpp"

CEObservable::CEObservable() : m_camera(glm::vec3(165098.718750f,6119.780762f,72035.828125f), 45.f, (21.f/9.f), 1.f, VIEW_R)
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
