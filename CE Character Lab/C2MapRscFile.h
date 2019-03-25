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
#include <array>

#include "deps/libAF/af2-sound.h"

#include "g_shared.h"

using libAF2::Sound;

class CETexture;
class CEWorldModel;
class C2Sky;
class CEWaterEntity;

class C2MapRscFile
{
private:
  enum _PadTypeColor { // rgba5551
    Red = 0b111110000000000,
    Yellow = 0b111111111000000,
    Green = 0b1111100000,
    Blue = 0b11111
  };
  enum _AtlasPadType {
    Below = _PadTypeColor::Red,
    Above = _PadTypeColor::Yellow,
    Left = _PadTypeColor::Green,
    Right = _PadTypeColor::Blue
  };
  int m_fade_rgb[3][3]; // calculated sky RGB base values
  int m_trans_rgb[3][3]; // calculated sky transparency values
  
  std::vector< std::unique_ptr<CETexture> > m_textures; // control access to the textures
  int m_texture_atlas_width;
  int m_texture_count;
  
  int m_random_sounds_count, m_ambient_sounds_count;
  std::vector<Sound> m_random_sounds;
  
  int m_num_waters;
  std::vector<CEWaterEntity> m_waters;
  
  std::unique_ptr<C2Sky> m_dawn_sky;
  std::unique_ptr<C2Sky> m_day_sky;
  std::unique_ptr<C2Sky> m_night_sky;
  
  std::vector<uint8_t> m_shadow_map;
  
  void load(const std::string& file_name);
public:
  std::vector< std::unique_ptr<CEWorldModel> > m_models;
  
  C2MapRscFile(const std::string& file_name);
  ~C2MapRscFile();
  
    int getTextureAtlasWidth();
    int getWorldModelCount();
  int getWaterCount();

  const CEWaterEntity& getWater(int i) const;
  CEWorldModel* getWorldModel(int i);
  CETexture* getTexture(int i);
  C2Sky* getDaySky();
};

#endif /* defined(__CE_Character_Lab__C2MapRscFile__) */
