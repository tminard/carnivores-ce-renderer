#include "C2Character.h"

#include "CE_ArtificialIntelligence.h"
#include "Hunt.h"

#include "C2CarFile.h"

C2Character::C2Character(CE_ArtificialIntelligence* intelligence_strategy, const std::shared_ptr<C2CarFile>& car_file)
	: intelligence(intelligence_strategy), CEAnimatableModel(car_file)
{
}

C2Character::~C2Character()
{
}


void C2Character::printDebuggingInfo(std::string& output)
{
	output = "C2Character abstract class debugging info.";
}

void C2Character::performWalkAction()
{
  return;
}

void C2Character::performRunAction()
{
  return;
}

void C2Character::performEatAction()
{
  return;
}

void C2Character::performJumpAction()
{
  return;
}

void C2Character::performSwimAction()
{
  return;
}

void C2Character::performAttackAction()
{
  return;
}

void C2Character::performDeathAction()
{
  return;
}

void C2Character::performIdleAction()
{
  return;
}
