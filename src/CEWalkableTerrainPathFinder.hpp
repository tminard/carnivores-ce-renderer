#pragma once

#include <memory>

class C2MapFile;
class C2MapRscFile;

struct CEWalkableTerrainPathFinder
{  
  bool operator() (unsigned x, unsigned y) const;

  std::shared_ptr<C2MapFile> map;
  std::shared_ptr<C2MapRscFile> rsc;
};

