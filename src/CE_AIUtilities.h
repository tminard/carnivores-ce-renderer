//
//  CE_AIUtilities.h
//  CE Character Lab
//
//  Created by Tyler Minard on 8/11/15.
//  Copyright (c) 2015 Tyler Minard. All rights reserved.
//

#ifndef __CE_Character_Lab__CE_AIUtilities__
#define __CE_Character_Lab__CE_AIUtilities__

#include <stdio.h>
#include "g_shared.h"

//#warning Move this into the World manager
float GetLandUpH(float x, float y);
float GetLandH(float x, float y);
int siRand(int R);
int CheckPlaceCollisionP(Vector3d &v);

class CE_AIUtilities
{
public:
  static float distance_to_point(Vector3d base_point, Vector3d target_point);
};

#endif /* defined(__CE_Character_Lab__CE_AIUtilities__) */
