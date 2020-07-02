#pragma once
#include "CE_ArtificialIntelligence.h"

//#warning move these DEFINES into the character model class
#define RAP_RUN    0
#define RAP_WALK   1
#define RAP_SWIM   2
#define RAP_SLIDE  3
#define RAP_JUMP   4
#define RAP_DIE    5
#define RAP_EAT    6
#define RAP_SLP    7

class AI_Allosaurus: public CE_ArtificialIntelligence {
  public:
    void think(int time_delta);
};
