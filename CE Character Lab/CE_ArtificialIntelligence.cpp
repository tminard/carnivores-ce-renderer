#include "CE_ArtificialIntelligence.h"
#include "C2Character.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <iostream>

//#warning forward declare old code base function
int CheckPlaceCollision(Vector3d &v, int wc, int mc);
float GetLandUpH(float x, float y);
float GetLandH(float x, float y);
int siRand(int R);
int CheckPlaceCollisionP(Vector3d &v);

CE_ArtificialIntelligence::CE_ArtificialIntelligence()
{
}

CE_ArtificialIntelligence::~CE_ArtificialIntelligence()
{
}

/*
 * Pick a new random target area.
 */
//#warning This is a direct port of the old code and should be updated.
void CE_ArtificialIntelligence::setNewTarget(float range)
{
  Vector3d cur_pos = this->character->getCurrentPosition();
  Vector3d new_pos;
  int tryi = 0;
replace:
  new_pos.x = cur_pos.x + siRand((int)range); if (new_pos.x<512) new_pos.x = 512; if (new_pos.x>1018*256) new_pos.x = 1018*256;
  new_pos.z = cur_pos.z + siRand((int)range); if (new_pos.z<512) new_pos.z = 512; if (new_pos.z>1018*256) new_pos.z = 1018*256;
  new_pos.y = GetLandH(new_pos.x, new_pos.z);
  tryi++;
  
  if (tryi < 128)
    if ( fabs(new_pos.x - cur_pos.x) + fabs(new_pos.z - cur_pos.z) < range / 2.f) goto replace;

  range += 512;
  
  if (tryi < 256)
    if (CheckPlaceCollisionP(new_pos)) goto replace;
  
  this->tgtime = 0;
  this->tgx = new_pos.x;
  this->tgz = new_pos.z;
}

//#warning This is a copy of the old code. Replace this with a modern implementation in a better location. Move to animation loop
void CE_ArtificialIntelligence::flush_previous_state()
{
  /*cptr->PPMorphTime+=TimeDt;
  if (cptr->PPMorphTime > PMORPHTIME) cptr->PrevPhase = cptr->Phase;
  
  cptr->PrevPFTime+=TimeDt;
  cptr->PrevPFTime %= cptr->pinfo->Animation[cptr->PrevPhase].AniTime;*/
}

Vector3d CE_ArtificialIntelligence::getTargetPosition()
{
  Vector3d t;
  t.x = this->tgx;
  t.z = this->tgz;
  t.y = 0;

  return t;
}

//#warning This is a copy of the old code. Replace this with a modern implementation in a better location.
void CE_ArtificialIntelligence::look_for_a_way_to_target(bool wc, bool mc)
{
  float alpha = this->TargetAlpha;
  float dalpha = 15.f;
  float afound = alpha;
  int maxp = 16;
  int curp;
  
  if (!this->check_possible_path(wc, mc)) { this->NoWayCnt=0; return; } // got lucky. No collisions; go this route.
  
  this->NoWayCnt++; // this way is blocked... let's find another.
  for (int i=0; i<12; i++) { // allow up to 180 degree turn to find a route (to my right)
    this->TargetAlpha = alpha+dalpha*M_PI/180.f;
    curp=this->check_possible_path(wc, mc) + (i>>1);
    if (!curp) return; // found the perfect route
    if (curp<maxp) { // found one with less than 16 collisions. Go with it and keep trying to find a better route.
      maxp = curp;
      afound = this->TargetAlpha;
    }
    
    this->TargetAlpha = alpha-dalpha*M_PI/180.f; // check the other 180 degree turn (to my left)
    curp=this->check_possible_path(wc, mc) + (i>>1);
    if (!curp) return; // found the perfect route. Go with it.
    if (curp<maxp) { // see if it beats going right
      maxp = curp;
      afound = this->TargetAlpha;
    }
    
    dalpha+=15.f; // try the next 15 degrees
  }
  
  this->TargetAlpha = afound; // go with the best option. Worst case, I ALWAYS go 15 degrees to my right
}

//#warning deprecate this and move inline
int CE_ArtificialIntelligence::check_possible_path(bool wc, bool mc)
{
  Vector3d p = this->character->getCurrentPosition();
  float lookx = (float)cos(this->TargetAlpha);
  float lookz = (float)sin(this->TargetAlpha);
  int c=0;
  // check 20 segments ahead for a path and count collisions
  for (int t=0; t<20; t++) {
    p.x+=lookx * 64.f;
    p.z+=lookz * 64.f;
    if (CheckPlaceCollision(p, (int)wc, (int)mc)) c++;
  }
  return c;
}


