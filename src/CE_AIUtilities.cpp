//
//  CE_AIUtilities.cpp
//  CE Character Lab
//
//  Created by Tyler Minard on 8/11/15.
//  Copyright (c) 2015 Tyler Minard. All rights reserved.
//

#include "CE_AIUtilities.h"
#include <math.h>

#include <iostream>

float CE_AIUtilities::distance_to_point(Vector3d base_point, Vector3d target_point)
{
  return (float)sqrt( (target_point.x - base_point.x) * (target_point.x - base_point.x) + (target_point.z - base_point.z) * (target_point.z - base_point.z));
}

