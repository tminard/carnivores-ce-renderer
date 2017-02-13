//
//  CETerrainVertexBuffer.h
//  CE Character Lab
//
//  Created by Tyler Minard on 2/12/17.
//  Copyright © 2017 Tyler Minard. All rights reserved.
//

#pragma once

#include "CEVertexBuffer.h"

class Vertex;

class CETerrainVertexBuffer : public CEVertexBuffer
{
public:
  void uploadToGPU(const Vertex* vertex_data, int vertex_data_size, const unsigned int* indice_data, int indice_data_size);
};
