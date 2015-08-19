//
//  vertex.h
//  CE Character Lab
//
//  Created by Tyler Minard on 8/18/15.
//  Copyright (c) 2015 Tyler Minard. All rights reserved.
//

#ifndef __CE_Character_Lab__vertex__
#define __CE_Character_Lab__vertex__

#include <stdio.h>
#include <glm/vec3.hpp>

class Vertex
{
private:
  glm::vec3 m_position;

public:
  Vertex(const glm::vec3& pos);
};

#endif /* defined(__CE_Character_Lab__vertex__) */
