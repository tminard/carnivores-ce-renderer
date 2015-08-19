//
//  C2CarFile.h
//  CE Character Lab
//
//  Created by Tyler Minard on 8/6/15.
//  Copyright (c) 2015 Tyler Minard. All rights reserved.
//

/*
 * This class is responsible for loading a C2/Ice Age car
 * file.
 *
 * Stores primitive types with the contents of the file.
 */

#ifndef CE_Character_Lab_C2CarFile_h
#define CE_Character_Lab_C2CarFile_h

#include <memory>
#include <string>
#include <cstdint>
#include <map>
#include <vector>

#include "g_shared.h"

class C2Geometry;
class C2Animation;
//class C2SoundFX;

class C2CarFile {

public:
  C2CarFile(std::string file_name);
  ~C2CarFile();
  void load_file(std::string file_name);

  C2Geometry* getGeometry();
  C2Animation* getAnimationByName(std::string animation_name);
  //std::vector<C2SoundFX*> getSoundsByAnimationName(std::string animation_name);

private:
  std::unique_ptr<C2Geometry> m_geometry;
  
  std::map<std::string, std::unique_ptr<C2Animation>> m_animations;
  //std::multimap<std::string, std::unique_ptr<C2SoundFX> > m_animation_sounds;
};

#endif
