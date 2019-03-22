#pragma once
#include <time.h>
#include "g_shared.h"

/*
* This class is responsible for processing a characters' surroundings and triggering certain behavior.
* Exact behavior must be implemented by derived class.
*
* Shared 'awareness' methods (such as 'iCanSee?()') can be added here to make available to all
* derived classes.
*/

class C2Character;

class CE_ArtificialIntelligence
{
protected:
  // Use old attributes for now to ease integration
//#warning TODO: convert this to modern structure and move into their respective classes
  int Phase = 0;
  int State = 0; // simple way to track movement from one state to another
  int StateF = 0;
  int FTime = 0;
  int AfriadTime = 0; // character is blindly afriad until m_afriad_time <= 0
  float TargetAlpha = 0.f;
  float tgx = 0.f, tgz = 0.f;
  int tgtime = 0;
  int NoFindCnt = 0;
  int NoWayCnt = 0;
  float rspeed = 0.f, bend = 0.f;
  float vspeed = 0.f;
  float slidex = 0.f, slidez = 0.f;
  int Slide = 0;
  float lookx = 0.f, lookz = 0.f;

//  #warning this is a copy of the existing code. Move to a proper location
  void flush_previous_state(); // aka ProcessPrevPhase
  void look_for_a_way_to_target(bool wc, bool mc);
  int check_possible_path(bool wc, bool mc);
  virtual void setNewTarget(float range);

public:
  C2Character* character; // weak, but accessed directly. Should be shared_ptr, probably (and private). Note that Character currently has raw pointer link to AI (but character controls alloc and delete)

	CE_ArtificialIntelligence();
	~CE_ArtificialIntelligence();

	virtual void think(int time_delta) = 0;
  Vector3d getTargetPosition();
};


