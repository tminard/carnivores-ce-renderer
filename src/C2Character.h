#pragma once

/*
* Responsible for movement and animations.
* AI is responsible for telling us how to act.
*
* This is basically an OOP wrapper to TCharacter in hunt.h.
*/
#include <time.h>
#include <memory>
#include <string>
#include "CEAnimatableModel.h"

class CE_ArtificialIntelligence;
class C2CarFile;

class C2Character: public CEAnimatableModel {
  public:
    CE_ArtificialIntelligence* intelligence; // dont let go out of scope... need to fix this

    /* (De)Constructor */
	C2Character(CE_ArtificialIntelligence* intelligence_strategy, const std::shared_ptr<C2CarFile>& car_file);
	C2Character();
    ~C2Character();

    // Spawn and killed behavior should be defined in derived class
  	virtual void spawn() = 0;
  	virtual void kill() = 0;

    // Debug info should be extended in derived class
    virtual void printDebuggingInfo(std::string& output);

    /* Game Animations hooks. Note that only a few of these are provided in the general
    * implementation, such as walk, run, and die. Derived class should implement others.
    * These commands can be triggered on the character from the command line,
    * internally within the Character class, or by AI.
    */
    virtual void performWalkAction();
    virtual void performRunAction();
    virtual void performEatAction();
    virtual void performJumpAction();
    virtual void performSwimAction();
    virtual void performAttackAction();
    virtual void performDeathAction();
    virtual void performIdleAction();
};
