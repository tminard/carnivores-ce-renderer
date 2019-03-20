//
//  CEPlayer.cpp
//  CE Character Lab
//
//  Created by Minard, Tyler on 6/8/18.
//  Copyright © 2018 Tyler Minard. All rights reserved.
//

#include "CEPlayer.hpp"

#include "C2MapFile.h"
#include "TerrainRenderer.h"

CEPlayer::CEPlayer(std::shared_ptr<C2MapFile> map) : m_map(map)
{
}

glm::vec2 CEPlayer::getWorldPosition()
{
    int x, y;
    glm::vec3 pos = this->getPosition();

    x = int(floorf(pos.x / m_map->getTileLength()));
    y = int(floorf(pos.z / m_map->getTileLength()));
    return glm::vec2(x, y);
}
