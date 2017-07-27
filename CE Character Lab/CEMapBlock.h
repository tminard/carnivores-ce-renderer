//
//  CEMapBlock.h
//  CE Character Lab
//
//  Created by Tyler Minard on 2/12/17.
//  Copyright © 2017 Tyler Minard. All rights reserved.
//

#pragma once

#include <stdio.h>
#include <exception>
#include <memory>
#include <map>
#include <vector>

#include <glm/glm.hpp>

class CETerrain;
class C2WorldModel;
class C2MapRscFile;
class C2MapFile;
class Shader;
class Camera;

class CEMapBlock
{
private:
  struct MObjectPlacementInfo {
    glm::vec3 position;
    C2WorldModel* model_weak;
    int tile_x;
    int tile_y;
  };

  std::unique_ptr<CETerrain> m_terrain;
  std::vector< MObjectPlacementInfo > m_model_placements;

  glm::vec2 m_start_position;
  int m_tile_x;
  int m_tile_y;

  C2MapFile* m_cmap_weak;
  C2MapRscFile* m_crsc_weak;

public:
  constexpr static const int BLOCK_SIZE = 16;

  CEMapBlock(glm::vec2 position, int tile_x, int tile_y, C2MapFile* cmap, C2MapRscFile* crsc, std::unique_ptr<CETerrain> terrain);
  ~CEMapBlock();

  void DrawTerrain(Shader* shader);
  void DrawWorldModels(Shader* shader, Camera* camera);
  glm::vec2 getPosition();
};
