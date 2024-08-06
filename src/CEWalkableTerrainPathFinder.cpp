#include "CEWalkableTerrainPathFinder.hpp"

#include "C2MapFile.h"
#include "C2MapRscFile.h"
#include "CEWorldModel.h"

#include <glm/vec2.hpp>

bool CEWalkableTerrainPathFinder::operator()(unsigned x, unsigned y) const
{
  int xy = (y * map->getWidth()) + x;
  // Unsigned wraps if < 0
  if(x < map->getHeight() && y < map->getWidth()) {
    if (map->getWalkableFlagsAt(glm::vec2(x, y)) & 0x1) return false;
    
    return true;
  }
  
  return false;
}
