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

class C2CarFile;

class C2Character {
  public:
	C2Character(const std::shared_ptr<C2CarFile>& car_file);
	C2Character();
};
