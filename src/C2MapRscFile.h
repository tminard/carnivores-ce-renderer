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
#include <random>

#include "g_shared.h"
#include "dependency/libAF/af2-sound.h"

#include <glm/glm.hpp>

using libAF2::Sound;

class CETexture;
class CEWorldModel;
class C2Sky;
class CEWaterEntity;
class CEAudioSource;

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
  CEMapType m_type;
  int m_fade_rgb[3][3]; // calculated sky RGB base values
  int m_trans_rgb[3][3]; // calculated sky transparency values
  
  std::vector< std::unique_ptr<CETexture> > m_textures;
  int m_texture_atlas_width;
  int m_texture_count;
  
  int m_random_sounds_count, m_ambient_sounds_count;
  std::vector<std::shared_ptr<Sound>> m_random_sounds;
  std::vector<std::shared_ptr<CEAudioSource>> m_random_audio_sources;
  std::vector<std::shared_ptr<Sound>> m_ambient_sounds;
  std::vector<std::shared_ptr<CEAudioSource>> m_ambient_audio_sources;

  int m_num_waters;
  std::vector<CEWaterEntity> m_waters;
  
  std::unique_ptr<C2Sky> m_dawn_sky;
  std::unique_ptr<C2Sky> m_day_sky;
  std::unique_ptr<C2Sky> m_night_sky;
  
  std::vector<uint8_t> m_shadow_map;

  void generateTextureAtlas();
  
  void load(const std::string& file_name);
  void load_c1(const std::string& file_name);
public:
  std::vector< std::unique_ptr<CEWorldModel> > m_models;

  C2MapRscFile(const CEMapType type, const std::string& file_name);
  ~C2MapRscFile();
  
  int getTextureAtlasWidth();
  int getWorldModelCount();
  int getWaterCount();

  int getAtlasTileWidth();
  int getAtlasTilePadding();

  void setWaterHeight(int i, int h_unscaled);

  const CEWaterEntity& getWater(int i) const;
  CEWorldModel* getWorldModel(int i);
  CETexture* getTexture(int i);
  C2Sky* getDaySky();
    
  glm::vec4 getFadeColor();

  std::shared_ptr<CEAudioSource> getAmbientAudio(int i);
  std::shared_ptr<CEAudioSource> getRandomAudio(int x, int y, int z);

  void playRandomAudio(int x, int y, int z);
  void playAmbientAudio(int i);
};

#endif /* defined(__CE_Character_Lab__C2MapRscFile__) */
