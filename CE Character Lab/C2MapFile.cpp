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

  // TODO: DRY this up and remove interdeps
#include "C2MapRscFile.h"
#include "CEWaterEntity.h"

void Console_PrintLogString(std::string log_msg);

C2MapFile::C2MapFile(const CEMapType type, const std::string& map_file_name, C2MapRscFile* crsc_weak) : m_type(type)
{
  switch (m_type) {
    case C2:
      this->load(map_file_name, crsc_weak);
      break;
    case C1:
      this->load_c1(map_file_name, crsc_weak);
  }

}

C2MapFile::~C2MapFile()
{

}

float C2MapFile::getWidth()
{
  if (m_type == CEMapType::C2) {
    return SIZE;
  } else {
    return SIZE_C1;
  }
}

float C2MapFile::getHeight()
{
  if (m_type == CEMapType::C2) {
    return SIZE;
  } else {
    return SIZE_C1;
  }
}

float C2MapFile::getTileLength()
{
  return 256.f;
}

  // TODO: handle split textures
int C2MapFile::getTextureIDAt(int xy)
{
  if (m_type == CEMapType::C2) {
    return int(this->m_texture_A_index_data.at(xy));
  } else {
    if (this->hasWaterAt(xy)) {
      return 1;
    } else {
      return int(this->m_texturec1_A_index_data.at(xy));
    }
  }
}

uint16_t C2MapFile::getFlagsAt(int x, int y)
{
  int xy = (y * (this->getWidth())) + x;

  return this->getFlagsAt(xy);
}

uint16_t C2MapFile::getFlagsAt(int xy)
{
  if (m_type == CEMapType::C2) {
    if (xy <0 || xy >= this->m_flags_data.size()) {
      return 0;
    }

    return this->m_flags_data.at(xy);
  } else {
    return this->m_c1_flags_data.at(xy);
  }
}

float C2MapFile::getWaterHeightAt(int xy)
{
  if (xy <0 || xy >= this->m_heightmap_data.size() || m_type == C2) {
    return 0;
  }

  float scaled_height = this->m_heightmap_data.at(xy) * this->getHeightmapScale();

  return (scaled_height);
}

float C2MapFile::getHeightAt(int xy)
{
  if (xy <0 || xy >= this->m_heightmap_data.size()) {
    return 0;
  }

  float scaled_height;

  if (m_type == C2) {
    scaled_height = this->m_heightmap_data.at(xy) * this->getHeightmapScale();
  } else {
    if (this->hasWaterAt(xy)) {
      scaled_height = (this->m_watermap_data.at(xy) - 48) * this->getHeightmapScale();
    } else {
      scaled_height = this->m_heightmap_data.at(xy) * this->getHeightmapScale();
    }
  }

  return (scaled_height);
}

int C2MapFile::getObjectAt(int xy)
{
  if (xy <0 || xy >= this->m_object_index_data.size()) {
    return 255;
  }

  return int(this->m_object_index_data.at(xy));
}

float C2MapFile::getHeightmapScale()
{
  if (m_type == CEMapType::C2) {
    return HEIGHT_SCALE;
  } else {
    return HEIGHT_SCALE_C1;
  }
}

bool C2MapFile::hasWaterAt(int xy)
{
  if (m_type == CEMapType::C2) {
    uint16_t flags = this->getFlagsAt(xy);
    if (flags & 0x0080 || flags & 0x8000) return true;

    return false;
  } else {
    if (abs(this->m_watermap_data.at(xy) - this->m_heightmap_data.at(xy)) != 48) return true;

    return false;
  }
}

int C2MapFile::getWaterAt(int xy)
{
  if (m_type == C2) {
    return this->m_watermap_data.at(xy);
  } else {
    return 0; // Only 1 water type in C1
  }
}

float C2MapFile::getObjectHeightAt(int xy)
{
  if (xy < 0 || xy >= this->m_heightmap_data.size()) {
    return 0;
  }

  float object_height = this->m_object_heightmap_data.at(xy);

  float scaled_height = object_height * HEIGHT_SCALE;

  return (scaled_height);
}

/*
 * find the best/lowest height for a given location
 * Lifted from old source - not yet working properly
*/
int C2MapFile::getObjectHeightForRadius(int x, int y, int R)
{
    x = (x<<8) + 128;
    y = (y<<8) + 128;
    float hr,h;
    hr = getHeightAt((y*getWidth())+(x));
    
    h = getHeightAt( ((y)*getWidth()) + (x+R) ); if (h < hr && h != 0) hr = h;
    h = getHeightAt( ((y)*getWidth()) + (x-R) ); if (h < hr && h != 0) hr = h;
    h = getHeightAt( ((y+R)*getWidth()) + (x) ); if (h < hr && h != 0) hr = h;
    h = getHeightAt( ((y-R)*getWidth()) + (x) ); if (h < hr && h != 0) hr = h;
    return (int)hr;
}

/*
 * fix flags, etc
 */
void C2MapFile::postProcess(C2MapRscFile* crsc_weak)
{
  if (m_type == C1) {
    crsc_weak->setWaterHeight(0, -1); // magiccccc

    return;
  }

  int w = (int)this->getWidth();
  int h = (int)this->getHeight();
  
  for (int y = 1; y < w-1; y++)
    for (int x = 1; x < h-1; x++) {
      int xy = (y * w) + x;
      if (!(this->getFlagsAt(xy) & 0x0080)) {
        
        if (this->getFlagsAt(x+1, y) & 0x0080) { this->m_flags_data.at((y*w) + x + 1) |= 0x8000; this->m_watermap_data.at(xy) = this->m_watermap_data.at((y*w) + x + 1); }
        if (this->getFlagsAt(x, y+1) & 0x0080) { this->m_flags_data.at(((y+1)*w) + x) |= 0x8000; this->m_watermap_data.at(xy) = this->m_watermap_data.at(((y+1)*w) + x); }
        if (this->getFlagsAt(x-1, y) & 0x0080) { this->m_flags_data.at((y*w) + x - 1) |= 0x8000; this->m_watermap_data.at(xy) = this->m_watermap_data.at((y*w) + x - 1); }
        if (this->getFlagsAt(x, y-1) & 0x0080) { this->m_flags_data.at(((y-1)*w) + x) |= 0x8000; this->m_watermap_data.at(xy) = this->m_watermap_data.at(((y-1)*w) + x); }
        
        if (this->getFlagsAt(x-1, y-1) & 0x0080) { this->m_flags_data.at(((y-1)*w) + x - 1) |= 0x8000; this->m_watermap_data.at(xy) = this->m_watermap_data.at(((y-1)*w) + x - 1); }
        if (this->getFlagsAt(x+1, y-1) & 0x0080) { this->m_flags_data.at(((y-1)*w) + x + 1) |= 0x8000; this->m_watermap_data.at(xy) = this->m_watermap_data.at(((y-1)*w) + x + 1); }
        if (this->getFlagsAt(x-1, y+1) & 0x0080) { this->m_flags_data.at(((y+1)*w) + x - 1) |= 0x8000; this->m_watermap_data.at(xy) = this->m_watermap_data.at(((y+1)*w) + x - 1); }
        if (this->getFlagsAt(x+1, y+1) & 0x0080) { this->m_flags_data.at(((y+1)*w) + x + 1) |= 0x8000; this->m_watermap_data.at(xy) = this->m_watermap_data.at(((y+1)*w) + x + 1); }
        
        if (this->getFlagsAt(xy) & 0x8000) {
          if (this->m_heightmap_data.at(xy) == crsc_weak->getWater(this->m_watermap_data.at(xy)).water_level) {
            this->m_heightmap_data.at(xy) += 1;
          }
        }
      }
    }
}

void C2MapFile::load_c1(const std::string &file_name, C2MapRscFile* crsc_weak)
{
  std::ifstream infile;
  infile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

  try {
    infile.open(file_name.c_str(), std::ios::binary | std::ios::in);

    infile.read(reinterpret_cast<char *>(this->m_heightmap_data.data()), 512*512);
    infile.read(reinterpret_cast<char *>(this->m_texturec1_A_index_data.data()), 512*512);
    infile.read(reinterpret_cast<char *>(this->m_texturec1_B_index_data.data()), 512*512);
    infile.read(reinterpret_cast<char *>(this->m_object_index_data.data()), 512*512);
    infile.read(reinterpret_cast<char *>(this->m_c1_flags_data.data()), 512*512);

    infile.read(reinterpret_cast<char *>(this->m_day_brightness_data.data()), 512*512);

    infile.read(reinterpret_cast<char *>(this->m_watermap_data.data()), 512*512); // C1 has only 1 water - these are the heights of the underwater surface
    infile.read(reinterpret_cast<char *>(this->m_object_heightmap_data.data()), 512*512);
    infile.read(reinterpret_cast<char *>(this->m_fog_data.data()), 256*256);
    infile.read(reinterpret_cast<char *>(this->m_soundfx_data.data()), 256*256);

    infile.close();
    this->postProcess(crsc_weak);
  } catch (std::ifstream::failure e) {
    Console_PrintLogString("Failed to load C1 " + file_name + ": " + strerror(errno));
    std::cerr << "Exception opening/reading/closing file\n";
  }
}

void C2MapFile::load(const std::string &file_name, C2MapRscFile* crsc_weak)
{
  std::ifstream infile;
  infile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

  try {
    infile.open(file_name.c_str(), std::ios::binary | std::ios::in);

    infile.read(reinterpret_cast<char *>(this->m_heightmap_data.data()), 1024*1024);
    infile.read(reinterpret_cast<char *>(this->m_texture_A_index_data.data()), 1024*1024*2);
    infile.read(reinterpret_cast<char *>(this->m_texture_B_index_data.data()), 1024*1024*2);
    infile.read(reinterpret_cast<char *>(this->m_object_index_data.data()), 1024*1024);
    infile.read(reinterpret_cast<char *>(this->m_c1_flags_data.data()), 1024*1024*2);

    infile.read(reinterpret_cast<char *>(this->m_dawn_brightness_data.data()), 1024*1024);
    infile.read(reinterpret_cast<char *>(this->m_day_brightness_data.data()), 1024*1024);
    infile.read(reinterpret_cast<char *>(this->m_night_brightness_data.data()), 1024*1024);

    infile.read(reinterpret_cast<char *>(this->m_watermap_data.data()), 1024*1024);
    infile.read(reinterpret_cast<char *>(this->m_object_heightmap_data.data()), 1024*1024);
    infile.read(reinterpret_cast<char *>(this->m_fog_data.data()), 512*512);
    infile.read(reinterpret_cast<char *>(this->m_soundfx_data.data()), 512*512);

    infile.close();
    
    this->postProcess(crsc_weak);
  } catch (std::ifstream::failure e) {
    Console_PrintLogString("Failed to load " + file_name + ": " + strerror(errno));
    std::cerr << "Exception opening/reading/closing file\n";
  }
}
