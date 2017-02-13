//
//  CETerrain.h
//  CE Character Lab
//
//  Created by Tyler Minard on 2/12/17.
//  Copyright © 2017 Tyler Minard. All rights reserved.
//
// Render a section of terrain. The difference between this an a CEGeometry is that the texture data is shared.

#pragma once
#include <stdio.h>
#include <exception>
#include <memory>

#include <vector>

class CEGeometry;

class CETerrain
{
private:
  std::unique_ptr<CEGeometry> m_geometry;

public:

  CETerrain(std::unique_ptr<CEGeometry> geometry);
  ~CETerrain();

  void Draw();
};
