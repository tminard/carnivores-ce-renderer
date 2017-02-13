//
//  CETerrain.cpp
//  CE Character Lab
//
//  Created by Tyler Minard on 2/12/17.
//  Copyright © 2017 Tyler Minard. All rights reserved.
//

#include "CETerrain.h"

#include "CEGeometry.h"

CETerrain::CETerrain(std::unique_ptr<CEGeometry> geometry)
: m_geometry(std::move(geometry))
{
}

void CETerrain::Draw()
{
  this->m_geometry->Draw();
}

CETerrain::~CETerrain()
{
}
