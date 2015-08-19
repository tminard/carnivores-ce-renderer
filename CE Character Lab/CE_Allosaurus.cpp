#include "CE_Allosaurus.h"
#include "AI_Allosaurus.h"

#include "C2CarFile.h"
#include "C2CarFilePreloader.h"

//#include "Hunt.h"


CE_Allosaurus::CE_Allosaurus(C2CarFilePreloader* preloader, const std::string& car_file_name)
	: C2Character(new AI_Allosaurus(), preloader->fetch(car_file_name))
{
//#warning Blah. Refactor this.
	this->intelligence->character = this;
}

void CE_Allosaurus::spawn()
{
	C2HuntableCharacter::spawn();
}

void CE_Allosaurus::kill()
{
	C2HuntableCharacter::kill();
}

void CE_Allosaurus::printDebuggingInfo(std::string& output)
{
	output = "Class Name: CE_Allosaurus\n\n"
		"AI Class: AI_Allo\n:::";
}

void CE_Allosaurus::performWalkAction()
{
  this->setAnimation("Rap_wlk");
}

void CE_Allosaurus::performRunAction()
{
 this->setAnimation("Rap_run1");
}

void CE_Allosaurus::performIdleAction()
{
  // igore this command. Not supported by model.
}