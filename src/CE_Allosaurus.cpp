#include "CE_Allosaurus.h"

#include "C2CarFile.h"
#include "C2CarFilePreloader.h"

//#include "Hunt.h"


CE_Allosaurus::CE_Allosaurus(C2CarFilePreloader* preloader, const std::string& car_file_name)
	: C2Character(preloader->fetch(car_file_name))
{
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
