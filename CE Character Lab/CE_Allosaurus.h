/*
* This class exists only to demonstrate how to initiate a hard-coded dinosaur type.
* This class determines how to respond to common behavior, as dictated by the AI.
*
* About: Allosaurus is moderately aggressive carnivore.
* Has custom 'slide' animations to smooth turning.
*/
#pragma once
#include "C2HuntableCharacter.h"
#include "C2Character.h"
#include <string>

class C2CarFilePreloader;

class CE_Allosaurus :
	public C2Character, public C2HuntableCharacter
{
public:
	CE_Allosaurus(C2CarFilePreloader* preloader, const std::string& car_file_name = "allo.car");

	void spawn();
	void kill();
	void printDebuggingInfo(std::string& output);
  
  void performWalkAction();
  void performRunAction();
  void performIdleAction();
};

