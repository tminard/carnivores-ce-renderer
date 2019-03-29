  //
  //  C2MapFile.h
  //  CE Character Lab
  //
  //  Created by Tyler Minard on 8/14/15.
  //  Copyright (c) 2015 Tyler Minard. All rights reserved.
  //

#ifndef __CE_Character_Lab__C2MapFile__
#define __CE_Character_Lab__C2MapFile__

#include <stdio.h>
#include <string>
#include <memory>
#include <vector>

#include <cstdint>
#include <array>

#include "g_shared.h"

class C2MapRscFile;
struct _Water;

class C2MapFile
{
private:
  std::array<uint8_t, 1024*1024> m_heightmap_data;
  std::array<uint8_t, 512*512> m_texturec1_A_index_data;
  std::array<uint8_t, 512*512> m_texturec1_B_index_data;
  std::array<uint16_t, 1024*1024> m_texture_A_index_data;
  std::array<uint16_t, 1024*1024> m_texture_B_index_data;
  std::array<uint8_t, 1024*1024> m_object_index_data;
  std::array<uint16_t, 1024*1024> m_flags_data;
  std::array<uint8_t, 512*512> m_c1_flags_data;
  std::array<uint8_t, 1024*1024> m_dawn_brightness_data;
  std::array<uint8_t, 1024*1024> m_day_brightness_data;
  std::array<uint8_t, 1024*1024> m_night_brightness_data;
  std::array<uint8_t, 1024*1024> m_watermap_data;
  std::array<uint8_t, 1024*1024> m_object_heightmap_data;
  std::array<uint8_t, 512*512> m_fog_data;
  std::array<uint8_t, 512*512> m_soundfx_data;

  const CEMapType m_type;
  constexpr static const float SIZE = 1024.f;
  constexpr static const float SIZE_C1 = 512.f;
  constexpr static const float HEIGHT_SCALE = 32.f;
  constexpr static const float HEIGHT_SCALE_C1 = 16.f;
  
  void postProcess(C2MapRscFile* crsc_weak);
  void fillWater(int x, int y, int src_x, int src_y);
  void copyWaterMap(int x, int y, int src_x, int src_y);

public:
  C2MapFile(const CEMapType map_type, const std::string& map_file_name, C2MapRscFile* crsc_weak);
  ~C2MapFile();

  int getWaterAt(int xy);
  float getHeightAt(int xy);
  float getObjectHeightAt(int xy);
  float getWaterHeightAt(int xy); // Used by C1
  int getObjectHeightForRadius(int x, int y, int R);
  int getObjectAt(int xy);
  int getTextureIDAt(int xy);
  int getWaterTextureIDAt(int xy, int water_texture_id);

  uint16_t getFlagsAt(int xy);
  uint16_t getFlagsAt(int x, int y);

  float getHeight();
  float getWidth();
  float getTileLength();
  float getHeightmapScale();
  float getBrightnessAt(int xy);

  bool hasWaterAt(int xy);
  bool hasWaterAt(int x, int y);

  bool hasOriginalWaterAt(int xy);
  bool hasOriginalWaterAt(int x, int y);
  bool hasDynamicWaterAt(int xy);
  bool hasDynamicWaterAt(int x, int y);

  void setWaterAt(int x, int y); // for C2
  void setWaterAt(int x, int y, int water_height);// c1

  void load(const std::string& file_name, C2MapRscFile* crsc_weak);
  void load_c1(const std::string& file_name, C2MapRscFile* crsc_weak);
};

#endif /* defined(__CE_Character_Lab__C2MapFile__) */
