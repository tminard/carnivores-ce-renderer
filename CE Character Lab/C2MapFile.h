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

class C2MapFile
{
private:
  std::array<uint8_t, 1024*1024> m_heightmap_data;
  std::array<uint16_t, 1024*1024> m_texture_A_index_data;
  std::array<uint16_t, 1024*1024> m_texture_B_index_data;
  std::array<uint8_t, 1024*1024> m_object_index_data;
  std::array<uint16_t, 1024*1024> m_flags_data;
  std::array<uint8_t, 1024*1024> m_dawn_brightness_data;
  std::array<uint8_t, 1024*1024> m_day_brightness_data;
  std::array<uint8_t, 1024*1024> m_night_brightness_data;
  std::array<uint8_t, 1024*1024> m_watermap_data;
  std::array<uint8_t, 1024*1024> m_object_heightmap_data;
  std::array<uint8_t, 512*512> m_fog_data;
  std::array<uint8_t, 512*512> m_soundfx_data;

public:
  C2MapFile(const std::string& map_file_name);
  ~C2MapFile();
  
  void load(const std::string& file_name);
};

#endif /* defined(__CE_Character_Lab__C2MapFile__) */
