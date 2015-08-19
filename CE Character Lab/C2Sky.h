//
//  C2Sky.h
//  CE Character Lab
//
//  Created by Tyler Minard on 8/18/15.
//  Copyright (c) 2015 Tyler Minard. All rights reserved.
//

#ifndef __CE_Character_Lab__C2Sky__
#define __CE_Character_Lab__C2Sky__

#include <stdio.h>
#include <memory>
#include <vector>

#include <iostream>
#include <fstream>

class C2Texture;

class C2Sky
{
private:
  std::unique_ptr<C2Texture> m_texture;

public:
  C2Sky(std::ifstream& instream);
  C2Sky(std::unique_ptr<C2Texture> sky_texture);
  ~C2Sky();
  
  void saveTextureAsBMP(const std::string& file_name );
};

#endif /* defined(__CE_Character_Lab__C2Sky__) */
