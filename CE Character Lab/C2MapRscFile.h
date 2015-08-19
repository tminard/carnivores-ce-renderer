//
//  C2MapRscFile.h
//  CE Character Lab
//
//  Created by Tyler Minard on 8/14/15.
//  Copyright (c) 2015 Tyler Minard. All rights reserved.
//

#ifndef __CE_Character_Lab__C2MapRscFile__
#define __CE_Character_Lab__C2MapRscFile__

#include <stdio.h>
#include <string>

#include <vector>
#include <memory>

class C2Texture;
class C2WorldModel;
class C2Sky;

class C2MapRscFile
{
private:
  int m_fade_rgb[3][3]; // calculated sky RGB base values
  int m_trans_rgb[3][3]; // calculated sky transparency values
  
  std::vector< std::unique_ptr<C2Texture> > m_textures; // control access to the textures
  std::vector< std::unique_ptr<C2WorldModel> > m_models;
  
  std::unique_ptr<C2Sky> m_dawn_sky;
  std::unique_ptr<C2Sky> m_day_sky;
  std::unique_ptr<C2Sky> m_night_sky;
  
  std::vector<uint8_t> m_shadow_map;

  void load(const std::string& file_name);
public:
  C2MapRscFile(const std::string& file_name);
  ~C2MapRscFile();
};

#endif /* defined(__CE_Character_Lab__C2MapRscFile__) */
