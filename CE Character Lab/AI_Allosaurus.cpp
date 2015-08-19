//
//  AI_Allosaurus.cpp
//  CE Character Lab
//
//  Created by Tyler Minard on 8/10/15.
//  Copyright (c) 2015 Tyler Minard. All rights reserved.
//

#include "AI_Allosaurus.h"
#include <stdio.h>

#include "CE_AIUtilities.h"
#include "C2Character.h"
#include "g_shared.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>

// Forward declarations from old code base
//#warning Move this into the World manager
float GetLandUpH(float x, float y);
float GetLandH(float x, float y);
float CorrectedAlpha(float a, float b);
float FindVectorAlpha(float, float);
int rRand(int r);
void DeltaFunc(float &a, float b, float d);
float SGN(float);
float AngleDifference(float a, float b);

/*
 * Allo behavior characteristics:
 * - Wanders around randomly.
 * - Upon finding player, moderately aggressive.
 * - Easy to scare away, but forgets quickly.
 *
 * TODO: Using the old AI for ease of integration.
 * Can: swim, jump
 */
void AI_Allosaurus::think(int time_delta)
{
//#warning TODO: convert this code to modern method. Also, this does not include player (enemy) tracking.
  this->tgtime += time_delta;
  if (this->tgtime > 30*1000) {
    this->setNewTarget(2048);
  }

  bool NewPhase = false;
  int _Phase = this->Phase;
  
TBEGIN:
  float targetx = this->tgx;
  float targetz = this->tgz;
  float targetdx = targetx - this->character->getCurrentPosition().x;
  float targetdz = targetz - this->character->getCurrentPosition().z;
  
  float tdist = (float)sqrt( targetdx * targetdx + targetdz * targetdz );

  if (this->State==2) {
    if (this->Phase != RAP_JUMP) {
      NewPhase=true;
      this->State=1;
    }
  }
  
  
  if (GetLandUpH(this->character->getCurrentPosition().x, this->character->getCurrentPosition().z) - GetLandH(this->character->getCurrentPosition().x, this->character->getCurrentPosition().z) > 180 * this->character->getScale()) {
	  this->StateF |= csONWATER;
  }
  else {
	  this->StateF &= (!csONWATER);
  }
       
  
  if (this->Phase == RAP_EAT) goto NOTHINK;

  if (!this->State) {
    if (tdist<456) {
      this->setNewTarget(8048.f);
      goto TBEGIN; }
  }
  
NOTHINK:
  if (this->NoFindCnt) {
    this->NoFindCnt--;
  } else {
    this->TargetAlpha = CorrectedAlpha(FindVectorAlpha(targetdx, targetdz), this->character->getCurrentAlpha());
  }
  
  this->look_for_a_way_to_target(false, true);
  if (this->NoWayCnt>12) { this->NoWayCnt=0; this->NoFindCnt = 16 + rRand(20); }
  
  
  if (this->TargetAlpha < 0) this->TargetAlpha+=2*M_PI;
  if (this->TargetAlpha > 2*M_PI) this->TargetAlpha-=2*M_PI;
  
  //===============================================//
  
//#warning move this to the animation loop (tracks previous state morphing)
  this->flush_previous_state();
  //======== select new phase =======================//
  NewPhase = this->character->didAnimationFinish();
  
  if (this->Phase==RAP_EAT)  goto ENDPSELECT;
  if (NewPhase && _Phase==RAP_JUMP) { this->Phase = RAP_RUN; goto ENDPSELECT; }

  if (this->Phase== RAP_JUMP) goto ENDPSELECT;
  
  if (!this->State) {
    this->Phase=RAP_WALK;
  } else {
    if (fabs(this->TargetAlpha - this->character->getCurrentAlpha())<1.0 || fabs(this->TargetAlpha - this->character->getCurrentAlpha())>2*M_PI-1.0) {
      this->Phase = RAP_RUN;
    } else {
      this->Phase=RAP_WALK;
    }
  }
  
  if (this->StateF & csONWATER) this->Phase = RAP_SWIM;
  if (this->Slide>40) this->Phase = RAP_SLIDE;
  
  
ENDPSELECT:
  
  //====== process phase changing ===========//
  // animation did change in the p select? If so, prepare morphing variables
  /*if (_Phase != this->Phase) {
    //==== set proportional FTime for better morphing =//
    if (MORPHP)
      if (_Phase<=3 && cptr->Phase<=3)
        cptr->FTime = _FTime * cptr->pinfo->Animation[cptr->Phase].AniTime / cptr->pinfo->Animation[_Phase].AniTime + 64;
      else if (!NewPhase) cptr->FTime = 0;
    
    if (cptr->PPMorphTime>128) {
      cptr->PrevPhase = _Phase;
      cptr->PrevPFTime  = _FTime;
      cptr->PPMorphTime = 0; }
  }*/
  
  //cptr->FTime %= cptr->pinfo->Animation[cptr->Phase].AniTime;
  
  
  
  //========== rotation to tgalpha ===================//
  
  float rspd, currspeed, tgbend, new_alpha;
  float dalpha = (float)fabs(this->TargetAlpha - this->character->getCurrentAlpha());
  float drspd = dalpha;
  if (drspd>M_PI) drspd = 2*M_PI - drspd;
  
  if (this->Phase==RAP_JUMP || this->Phase==RAP_EAT) {
    goto SKIPROT;
  }
  
  if (drspd > 0.02)
    if (this->TargetAlpha > this->character->getCurrentAlpha()) currspeed = 0.6f + drspd*1.2f;
    else currspeed =-0.6f - drspd*1.2f;
    else currspeed = 0;
  if (this->AfriadTime) currspeed*=2.5;
  
  if (dalpha > M_PI) currspeed*=-1;
  if ((this->StateF & csONWATER) || this->Phase==RAP_WALK) currspeed/=1.4f;
  
  if (this->AfriadTime) DeltaFunc(this->rspeed, currspeed, (float)time_delta / 160.f);
  else DeltaFunc(this->rspeed, currspeed, (float)time_delta / 180.f);
  
  tgbend = drspd/2;
  if (tgbend>M_PI/5) tgbend = M_PI/5;
  
  tgbend*= SGN(currspeed);
  if (fabs(tgbend) > fabs(this->bend)) DeltaFunc(this->bend, tgbend, (float)time_delta / 800.f);
  else DeltaFunc(this->bend, tgbend, (float)time_delta / 600.f);
  
  
  rspd=this->rspeed * time_delta / 1024.f;
  if (drspd < fabs(rspd)) this->character->setAlpha(this->TargetAlpha);
  else this->character->setAlpha(this->character->getCurrentAlpha() + rspd);
  
  new_alpha = this->character->getCurrentAlpha();
  if (new_alpha > M_PI * 2) new_alpha-= M_PI * 2;
  if (new_alpha < 0     ) new_alpha+= M_PI * 2;
  this->character->setAlpha(new_alpha);
  
SKIPROT:
  
  //======= set slide mode ===========//
  if (!this->Slide && this->vspeed>0.6 && this->Phase!=RAP_JUMP)
    if (AngleDifference(this->TargetAlpha, this->character->getCurrentAlpha())>M_PI*2/3.f) {
      this->Slide = (int)(this->vspeed*700.f);
      this->slidex = this->lookx;
      this->slidez = this->lookz;
      this->vspeed = 0;
    }
  
  
  
  //========== movement ==============================//
//#warning move these into the animatable model class and make auto-generated with setAlpha
  this->lookx = (float)cos(this->character->getCurrentAlpha());
  this->lookz = (float)sin(this->character->getCurrentAlpha());
  
  float curspeed = 0;
  if (this->Phase == RAP_RUN ) curspeed = 1.2f;
  if (this->Phase == RAP_JUMP) curspeed = 1.1f;
  if (this->Phase == RAP_WALK) curspeed = 0.428f;
  if (this->Phase == RAP_SWIM) curspeed = 0.4f;
  if (this->Phase == RAP_EAT) curspeed = 0.0f;
  
  if (this->Phase == RAP_RUN && this->Slide) {
    curspeed /= 8;
    if (drspd > M_PI / 2.f) curspeed=0; else
      if (drspd > M_PI / 4.f) curspeed*=2.f - 4.f*drspd / M_PI;
  } else
    if (drspd > M_PI / 2.f) curspeed*=2.f - 2.f*drspd / M_PI;
  
  //========== process speed =============//
  
  DeltaFunc(this->vspeed, curspeed, time_delta / 500.f);
  
  if (this->Phase==RAP_JUMP) this->vspeed = 1.1f;
  
  this->character->moveTo(this->lookx * this->vspeed * time_delta * this->character->getScale(), this->lookz * this->vspeed * time_delta * this->character->getScale(), false, true);
  
  //========== slide ==============//
  if (this->Slide) {
    this->character->moveTo(this->slidex * this->Slide / 600.f * time_delta * this->character->getScale(), this->slidez * this->Slide / 600.f * time_delta * this->character->getScale(), false, true);
    
    this->Slide -= time_delta;
    if (this->Slide<0) this->Slide=0;
  }
  
  
  //============ Y movement =================//
  if (this->StateF & csONWATER) {
    float py = GetLandUpH(this->character->getCurrentPosition().x, this->character->getCurrentPosition().z) - 200 * this->character->getScale();
    this->character->setPosition(this->character->getCurrentPosition().x, this->character->getCurrentPosition().z, py);
    
    this->character->setBeta(this->character->getBeta() / 2);
    this->character->setGamma(0);
  } else {
//#warning TODO ASAP: need to calculate gamma beta!
    //ThinkY_Beta_Gamma(cptr, 64, 32, 0.5f, 0.4f);
  }
  
  //=== process to tggamma ===//
  /*if (cptr->Phase==RAP_WALK) cptr->tggamma+= cptr->rspeed / 10.0f;
  else cptr->tggamma+= cptr->rspeed / 8.0f;
  if (cptr->Phase==RAP_JUMP) cptr->tggamma=0;
  
  DeltaFunc(cptr->gamma, cptr->tggamma, TimeDt / 1624.f);*/
  
  //=== sync animation ===//
  if (_Phase != this->Phase || NewPhase) {
    switch (this->Phase) {
      case RAP_RUN:
        this->character->performRunAction();
        break;
      case RAP_WALK:
        this->character->performWalkAction();
        break;
      default:
        this->character->performIdleAction();
        break;
    }
  }

  return;
}


























/*
void AnimateRaptor(TCharacter *cptr)
{
  NewPhase = FALSE;
  int _Phase = cptr->Phase;
  int _FTime = cptr->FTime;
  float _tgalpha = cptr->tgalpha;
  
  
TBEGIN:
  float targetx = cptr->tgx;
  float targetz = cptr->tgz;
  float targetdx = targetx - cptr->pos.x;
  float targetdz = targetz - cptr->pos.z;
  
  float tdist = (float)sqrt( targetdx * targetdx + targetdz * targetdz );
  
  float playerdx = PlayerX - cptr->pos.x - cptr->lookx * 100 * cptr->scale;
  float playerdz = PlayerZ - cptr->pos.z - cptr->lookz * 100 * cptr->scale;
  float pdist = (float)sqrt( playerdx * playerdx + playerdz * playerdz );
  if (ANIMAL_ATTACK_MODE && pdist > 1500)
	   pdist = 1500;
  
  if (cptr->State==2) { if (cptr->Phase!=RAP_JUMP) NewPhase=TRUE; cptr->State=1; }
  
  
  if (GetLandUpH(cptr->pos.x, cptr->pos.z) - GetLandH(cptr->pos.x, cptr->pos.z) > 180 * cptr->scale)
	   cptr->StateF |= csONWATER; else
       cptr->StateF &= (!csONWATER);
  
  if (cptr->Phase == RAP_EAT) goto NOTHINK;
  
  //============================================//
  //if (!MyHealth) cptr->State = 0;
  if (cptr->State) {
	   if (pdist > ctViewR*128+OptAgres/4 && !ANIMAL_ATTACK_MODE) {
       nv.x = playerdx; nv.z = playerdz; nv.y = 0;
       NormVector(nv, 2048.f);
       cptr->tgx = cptr->pos.x - nv.x;
       cptr->tgz = cptr->pos.z - nv.z;
       cptr->tgtime = 0;
       cptr->AfraidTime-=TimeDt;
       if (cptr->AfraidTime<=0) {
         cptr->AfraidTime=0; cptr->State = 0;
       }
       
     } else {
       cptr->tgx = PlayerX;
       cptr->tgz = PlayerZ;
       cptr->tgtime = 0;
     }
    
    if (!(cptr->StateF & csONWATER))
      if (pdist<1324 * cptr->scale && pdist>900 * cptr->scale)
        if (AngleDifference(cptr->alpha, FindVectorAlpha(playerdx, playerdz)) < 0.2f)
          cptr->Phase = RAP_JUMP;
    
    if (pdist<256)
      if (fabs(PlayerY - cptr->pos.y - 160) < 256)
        if (!(cptr->StateF & csONWATER)) {
          if (MyHealth) {
            ProcessAdvancedHealthSystem(cptr, 6500);
          } else if (pdist<64) {
            //Join the feast
            cptr->vspeed/= 8.0f;
            cptr->State = 1;
            cptr->Phase = RAP_EAT;
          }
        }
  }
  
  //-> Check if dinosaur is in dangerous fog...
  BOOL animalInFog;
  int fogID;
  fogID = FogsMap [((int)cptr->pos.z)>>9][((int)cptr->pos.x)>>9];
  if (FogsList[fogID].YBegin*ctHScale> cptr->pos.y)
    animalInFog = (fogID>0);
  else
    animalInFog = FALSE;
  
  if (animalInFog)
    if (cptr->Health)
      if (FogsList[fogID].Mortal) {
        cptr->Health-=1; //Dies really quickly...
        if (cptr->Health < 0)
          cptr->Health = 0;
      }
  
  if (!cptr->State) {
    if (tdist<456) {
      SetNewTargetPlace(cptr, 8048.f);
      goto TBEGIN; }
  }
  
NOTHINK:
  if (pdist<2048) cptr->NoFindCnt = 0;
  if (cptr->NoFindCnt) cptr->NoFindCnt--; else
  {
    cptr->tgalpha = CorrectedAlpha(FindVectorAlpha(targetdx, targetdz), cptr->alpha);//FindVectorAlpha(targetdx, targetdz);
    if (cptr->State && pdist>1648) {
      cptr->tgalpha += (float)sin(RealTime/824.f) / 2.f;
      if (cptr->tgalpha < 0) cptr->tgalpha+=2*pi;
      if (cptr->tgalpha > 2*pi) cptr->tgalpha-=2*pi;
    }
  }
  
  LookForAWay(cptr, FALSE, TRUE);
  if (cptr->NoWayCnt>12) { cptr->NoWayCnt=0; cptr->NoFindCnt = 16 + rRand(20); }
  
  
  if (cptr->tgalpha < 0) cptr->tgalpha+=2*pi;
  if (cptr->tgalpha > 2*pi) cptr->tgalpha-=2*pi;
  
  //===============================================//
  
  ProcessPrevPhase(cptr);
  
  
  //======== select new phase =======================//
  cptr->FTime+=TimeDt;
  
  if (cptr->FTime >= cptr->pinfo->Animation[cptr->Phase].AniTime) {
    cptr->FTime %= cptr->pinfo->Animation[cptr->Phase].AniTime;
    NewPhase = TRUE; }
  
  if (cptr->Phase==RAP_EAT)  goto ENDPSELECT;
  if (NewPhase && _Phase==RAP_JUMP) { cptr->Phase = RAP_RUN; goto ENDPSELECT; }
  
  
  if (cptr->Phase== RAP_JUMP) goto ENDPSELECT;
  
  if (!cptr->State) cptr->Phase=RAP_WALK; else
    if (fabs(cptr->tgalpha - cptr->alpha)<1.0 ||
        fabs(cptr->tgalpha - cptr->alpha)>2*pi-1.0)
      cptr->Phase = RAP_RUN; else cptr->Phase=RAP_WALK;
  
  if (cptr->StateF & csONWATER) cptr->Phase = RAP_SWIM;
  if (cptr->Slide>40) cptr->Phase = RAP_SLIDE;
  
  
ENDPSELECT:
  
  //====== process phase changing ===========//
  if ( (_Phase != cptr->Phase) || NewPhase)
    ActivateCharacterFx(cptr);
  
  if (_Phase != cptr->Phase) {
    //==== set proportional FTime for better morphing =//
    if (MORPHP)
      if (_Phase<=3 && cptr->Phase<=3)
        cptr->FTime = _FTime * cptr->pinfo->Animation[cptr->Phase].AniTime / cptr->pinfo->Animation[_Phase].AniTime + 64;
      else if (!NewPhase) cptr->FTime = 0;
    
    if (cptr->PPMorphTime>128) {
      cptr->PrevPhase = _Phase;
      cptr->PrevPFTime  = _FTime;
      cptr->PPMorphTime = 0; }
  }
  
  cptr->FTime %= cptr->pinfo->Animation[cptr->Phase].AniTime;
  
  
  
  //========== rotation to tgalpha ===================//
  
  float rspd, currspeed, tgbend;
  float dalpha = (float)fabs(cptr->tgalpha - cptr->alpha);
  float drspd = dalpha; if (drspd>pi) drspd = 2*pi - drspd;
  
  if (cptr->Phase==RAP_JUMP || cptr->Phase==RAP_EAT) goto SKIPROT;
  
  if (drspd > 0.02)
    if (cptr->tgalpha > cptr->alpha) currspeed = 0.6f + drspd*1.2f;
    else currspeed =-0.6f - drspd*1.2f;
    else currspeed = 0;
  if (cptr->AfraidTime) currspeed*=2.5;
  
  if (dalpha > pi) currspeed*=-1;
  if ((cptr->StateF & csONWATER) || cptr->Phase==RAP_WALK) currspeed/=1.4f;
  
  if (cptr->AfraidTime) DeltaFunc(cptr->rspeed, currspeed, (float)TimeDt / 160.f);
  else DeltaFunc(cptr->rspeed, currspeed, (float)TimeDt / 180.f);
  
  tgbend = drspd/2;
  if (tgbend>pi/5) tgbend = pi/5;
  
  tgbend*= SGN(currspeed);
  if (fabs(tgbend) > fabs(cptr->bend)) DeltaFunc(cptr->bend, tgbend, (float)TimeDt / 800.f);
  else DeltaFunc(cptr->bend, tgbend, (float)TimeDt / 600.f);
  
  
  rspd=cptr->rspeed * TimeDt / 1024.f;
  if (drspd < fabs(rspd)) cptr->alpha = cptr->tgalpha;
  else cptr->alpha+=rspd;
  
  
  if (cptr->alpha > pi * 2) cptr->alpha-= pi * 2;
  if (cptr->alpha < 0     ) cptr->alpha+= pi * 2;
  
SKIPROT:
  
  //======= set slide mode ===========//
  if (!cptr->Slide && cptr->vspeed>0.6 && cptr->Phase!=RAP_JUMP)
    if (AngleDifference(cptr->tgalpha, cptr->alpha)>pi*2/3.f) {
      cptr->Slide = (int)(cptr->vspeed*700.f);
      cptr->slidex = cptr->lookx;
      cptr->slidez = cptr->lookz;
      cptr->vspeed = 0;
    }
  
  
  
  //========== movement ==============================//
  cptr->lookx = (float)cos(cptr->alpha);
  cptr->lookz = (float)sin(cptr->alpha);
  
  float curspeed = 0;
  if (cptr->Phase == RAP_RUN ) curspeed = 1.2f;
  if (cptr->Phase == RAP_JUMP) curspeed = 1.1f;
  if (cptr->Phase == RAP_WALK) curspeed = 0.428f;
  if (cptr->Phase == RAP_SWIM) curspeed = 0.4f;
  if (cptr->Phase == RAP_EAT) curspeed = 0.0f;
  
  if (cptr->Phase == RAP_RUN && cptr->Slide) {
    curspeed /= 8;
    if (drspd > pi / 2.f) curspeed=0; else
      if (drspd > pi / 4.f) curspeed*=2.f - 4.f*drspd / pi;
  } else
    if (drspd > pi / 2.f) curspeed*=2.f - 2.f*drspd / pi;
  
  //========== Process Human Resistance =========//
  // 	 if (pdist<256)
	 //if (fabs(PlayerY - cptr->pos.y - 160) < 256) {
		// if (VSpeed > 0) {
		//	curspeed -= VSpeed/4; //Strength of human vrs Allosaurus...
		//	cptr->Phase = RAP_SLIDE;
		//	}
		// if (curspeed < 0)
		//	curspeed = 0;
	 //}
  
  //========== process speed =============//
  
  DeltaFunc(cptr->vspeed, curspeed, TimeDt / 500.f);
  
  if (cptr->Phase==RAP_JUMP) cptr->vspeed = 1.1f;
  
  MoveCharacter(cptr, cptr->lookx * cptr->vspeed * TimeDt * cptr->scale,
                cptr->lookz * cptr->vspeed * TimeDt * cptr->scale, FALSE, TRUE);
  
  
  //========== slide ==============//
  if (cptr->Slide) {
    MoveCharacter(cptr, cptr->slidex * cptr->Slide / 600.f * TimeDt * cptr->scale,
                  cptr->slidez * cptr->Slide / 600.f * TimeDt * cptr->scale, FALSE, TRUE);
    
    cptr->Slide-=TimeDt;
    if (cptr->Slide<0) cptr->Slide=0;
  }
  
  
  //============ Y movement =================//
  if (cptr->StateF & csONWATER) {
    cptr->pos.y = GetLandUpH(cptr->pos.x, cptr->pos.z) - 200 * cptr->scale;
    cptr->beta/=2;
    cptr->tggamma=0;
  } else {
    ThinkY_Beta_Gamma(cptr, 64, 32, 0.5f, 0.4f);
  }
  
  //=== process to tggamma ===//
  if (cptr->Phase==RAP_WALK) cptr->tggamma+= cptr->rspeed / 10.0f;
  else cptr->tggamma+= cptr->rspeed / 8.0f;
  if (cptr->Phase==RAP_JUMP) cptr->tggamma=0;
  
  DeltaFunc(cptr->gamma, cptr->tggamma, TimeDt / 1624.f);
  
  
  //==================================================//
  
}*/
