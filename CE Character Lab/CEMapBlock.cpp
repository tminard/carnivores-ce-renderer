//
//  CEMapBlock.cpp
//  CE Character Lab
//
//  Created by Tyler Minard on 2/13/17.
//  Copyright © 2017 Tyler Minard. All rights reserved.
//


#include "CEMapBlock.h"

#include "C2MapFile.h"
#include "C2MapRscFile.h"
#include "C2WorldModel.h"
#include "CETerrain.h"

#include "shader.h"
#include "camera.h"
#include "transform.h"

#include <iostream>

CEMapBlock::CEMapBlock(glm::vec2 position, int start_tile_x, int start_tile_y, C2MapFile* cmap, C2MapRscFile* crsc, std::unique_ptr<CETerrain> terrain)
: m_start_position(position), m_tile_x(start_tile_x), m_tile_y(start_tile_y), m_cmap_weak(cmap), m_crsc_weak(crsc), m_terrain(std::move(terrain))
{
  for (int y = m_tile_y; y < m_tile_y + BLOCK_SIZE; y++) {
    for (int x = m_tile_x; x < m_tile_x + BLOCK_SIZE; x++) {
      if (x > m_cmap_weak->getWidth() || y > m_cmap_weak->getHeight()) {
        continue;
      }

      int base_index = (y * m_cmap_weak->getHeight()) + x;
      int obj_id = cmap->getObjectAt(base_index);
      float obj_height = m_cmap_weak->getHeightAt(base_index);

      if (obj_height == 0.0f) {
        obj_height = m_cmap_weak->getObjectHeightAt(base_index);
      }

      if (obj_id != 255 && obj_id != 254) {
        MObjectPlacementInfo placementInfo;

        placementInfo.model_weak = m_crsc_weak->getWorldModel(obj_id);
        placementInfo.tile_x = x;
        placementInfo.tile_y = y;
        placementInfo.position = glm::vec3(x*m_cmap_weak->getTileLength(),obj_height,y*m_cmap_weak->getTileLength());

        this->m_model_placements.push_back(placementInfo);
      }
    }
  }
}

CEMapBlock::~CEMapBlock()
{

}

void CEMapBlock::DrawTerrain(Shader* shader)
{
  shader->Bind();
  this->m_terrain->Draw();
}

void CEMapBlock::DrawWorldModels(Shader* shader, Camera* camera)
{
  for (int obj_i = 0; obj_i < this->m_model_placements.size(); obj_i++) {
    MObjectPlacementInfo placement = this->m_model_placements.at(obj_i);
    // transform: TODO: Reduce calc on this - consumes decent memory still
    Transform mTrans_c(placement.position, glm::vec3(0,0,0), glm::vec3(2.f, 2.f, 2.f));

    // GLSL
    shader->Bind();
    shader->Update(mTrans_c, *camera);

    // Render
    placement.model_weak->render();
  }
}
