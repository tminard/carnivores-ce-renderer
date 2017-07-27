//
//  C2MapFile.cpp
//  CE Character Lab
//
//  Created by Tyler Minard on 8/14/15.
//  Copyright (c) 2015 Tyler Minard. All rights reserved.
//

#include "C2MapFile.h"
#include <iostream>
#include <fstream>

void Console_PrintLogString(std::string log_msg);

C2MapFile::C2MapFile(const std::string& map_file_name)
{
  this->load(map_file_name);
}

C2MapFile::~C2MapFile()
{

}

const float C2MapFile::getWidth()
{
  return SIZE;
}

const float C2MapFile::getHeight()
{
  return SIZE;
}

const float C2MapFile::getTileLength()
{
  return 256.f;
}

const int C2MapFile::getTextureIDAt(int xy)
{
  return int(this->m_texture_A_index_data.at(xy));
}

const uint16_t C2MapFile::getFlagsAt(int xy)
{
  return this->m_flags_data.at(xy);
}

const float C2MapFile::getHeightAt(int xy)
{
  if (xy <0 || xy >= this->m_heightmap_data.size()) {
    return 0;
  }

  float scaled_height = this->m_heightmap_data.at(xy) * HEIGHT_SCALE;

  return (scaled_height);
}

const int C2MapFile::getObjectAt(int xy)
{
  if (xy <0 || xy >= this->m_object_index_data.size()) {
    return 255;
  }

  return int(this->m_object_index_data.at(xy));
}

const float C2MapFile::getHeightmapScale()
{
  return HEIGHT_SCALE;
}

const float C2MapFile::getObjectHeightAt(int xy)
{
  if (xy < 0 || xy >= this->m_heightmap_data.size()) {
    return 0;
  }

  float object_height = this->m_object_heightmap_data.at(xy);

  float scaled_height = object_height * HEIGHT_SCALE;

  return (scaled_height);
}

void C2MapFile::load(const std::string &file_name)
{
  std::ifstream infile;
  infile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

  try {
    infile.open(file_name.c_str(), std::ios::binary | std::ios::in);

    infile.read(reinterpret_cast<char *>(this->m_heightmap_data.data()), 1024*1024);
    infile.read(reinterpret_cast<char *>(this->m_texture_A_index_data.data()), 1024*1024*2);
    infile.read(reinterpret_cast<char *>(this->m_texture_B_index_data.data()), 1024*1024*2);
    infile.read(reinterpret_cast<char *>(this->m_object_index_data.data()), 1024*1024);
    infile.read(reinterpret_cast<char *>(this->m_flags_data.data()), 1024*1024*2);

    infile.read(reinterpret_cast<char *>(this->m_dawn_brightness_data.data()), 1024*1024);
    infile.read(reinterpret_cast<char *>(this->m_day_brightness_data.data()), 1024*1024);
    infile.read(reinterpret_cast<char *>(this->m_night_brightness_data.data()), 1024*1024);

    infile.read(reinterpret_cast<char *>(this->m_watermap_data.data()), 1024*1024);
    infile.read(reinterpret_cast<char *>(this->m_object_heightmap_data.data()), 1024*1024);
    infile.read(reinterpret_cast<char *>(this->m_fog_data.data()), 512*512);
    infile.read(reinterpret_cast<char *>(this->m_soundfx_data.data()), 512*512);

    infile.close();
  } catch (std::ifstream::failure e) {
    Console_PrintLogString("Failed to load " + file_name + ": " + strerror(errno));
    std::cerr << "Exception opening/reading/closing file\n";
  }
}
