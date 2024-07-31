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

#include <math.h>

C2MapFile::C2MapFile(const CEMapType type, const std::string& map_file_name, C2MapRscFile* crsc_weak) : m_type(type)
{
  if (!std::filesystem::exists(map_file_name)) {
      throw std::runtime_error("File not found: " + map_file_name);
  } else {
    std::cout << "Found " << map_file_name << "; OK. Loading..." << std::endl;
  }
  
  switch (m_type) {
  case CEMapType::C2:
      this->load(map_file_name, crsc_weak);
      break;
  case CEMapType::C1:
      this->load_c1(map_file_name, crsc_weak);
  }

}

C2MapFile::~C2MapFile()
{

}

int C2MapFile::getAmbientAudioIDAt(int x, int y)
{
  x = x>>1;
  y = y>>1;
  int w = (int)getWidth()>>1;
  
  int xy = (y * w) + x;

  return this->m_soundfx_data.at(xy);
}

int C2MapFile::getWidth()
{
  if (m_type == CEMapType::C2) {
    return SIZE;
  } else {
    return SIZE_C1;
  }
}

int C2MapFile::getHeight()
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

int C2MapFile::getWaterTextureIDAt(int xy, int water_texture_id)
{
  if (m_type == CEMapType::C2) {
    return water_texture_id;
  } else {
    int water_texture;

    uint32_t texture_a = this->m_texturec1_A_index_data.at(xy);
    uint32_t texture_b = this->m_texturec1_B_index_data.at(xy);

    if (!texture_a || !texture_b) {
      water_texture = 0;
    } else {
      // TODO: need to figure out which one is the correct liquid texture for split lava and pond tiles
      water_texture = texture_a;
    }

    return water_texture;
  }
}

int C2MapFile::getTextureIDAt(int xy)
{
  if (m_type == CEMapType::C2) {
    return int(this->m_texture_A_index_data.at(xy));
  } else {
    int id = int(this->m_texturec1_A_index_data.at(xy));
    if (this->hasWaterAt(xy)) {
      if (!id) {
        return 1;
      }
      return id;
    } else {
      return id;
    }
  }
}

int C2MapFile::getSecondaryTextureIDAt(int xy)
{
  if (m_type == CEMapType::C2) {
    // C2 doesn't use secondary textures - texture map B is used for alternative purposes
    // For now, just return the primary texture
    return int(this->m_texture_A_index_data.at(xy));
  } else {
    int id = int(this->m_texturec1_B_index_data.at(xy));
    if (this->hasWaterAt(xy)) {
      if (!id) {
        return 1;
      }
      return id;
    } else {
      return id;
    }
  }
}

float C2MapFile::getBrightnessAt(int x, int y)
{
  int xy = (y*this->getWidth()) + x;

  return getBrightnessAt(xy);
}

float C2MapFile::getBrightnessAt(int xy)
{
  int brightness = this->m_day_brightness_data.at(xy); // uint8 (max 255)

  if (m_type == CEMapType::C1) {
    float p = float(55.f - brightness) / 55.f;
    brightness = 255.f * p;
  }

  return (float)brightness;
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
    if (xy <0 || xy >= this->m_c1_flags_data.size()) {
      return 0;
    }
    return this->m_c1_flags_data.at(xy);
  }
}

bool C2MapFile::isQuadRotatedAt(int xy)
{
  if (m_type == CEMapType::C2) {
    return (getFlagsAt(xy) & 0x0010);
  } else {
    return (getFlagsAt(xy) & 0x40);
  }
}

/*
 * Given that this location is a water tile, return the water's height for the whole quad.
 * xy is the "anchor" vertex in the quad
 */
float C2MapFile::getWaterHeightAt(int x, int y)
{
  int xy = (y * this->getWidth()) + x;
  if (xy < 0 || xy >= this->m_heightmap_data.size() || m_type == CEMapType::C2) {
    return 0;
  }

  // In c1, half water tiles have anchor vertex (at xy) in the ground, and thus calculate a different height. Fix this here
  float lowest_height = this->getLowestHeight(x, y);

  float scaled_height = (lowest_height + (48.f)) * this->getHeightmapScale();

  return (scaled_height);
}

float C2MapFile::getHeightAt(int xy)
{
  float scaled_height;
  
  if (xy < 0 || xy >= m_heightmap_data.size()) {
    std::cerr << "OOB height requested for heightmap! Returning -1" << std::endl;
    return -1.f;
  }

  if (m_type == CEMapType::C2) {
    scaled_height = this->m_heightmap_data.at(xy) * this->getHeightmapScale();
  } else {
    scaled_height = this->m_watermap_data.at(xy) * this->getHeightmapScale();
  }

  return (scaled_height);
}

void C2MapFile::setGroundLevelAt(int x, int y, float level)
{
  int xy = (y * this->getWidth()) + x;

  m_ground_levels[xy] = level;
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
    uint8_t flags = this->getFlagsAt(xy);
    if (flags & 0x80) return true;
    if (this->m_watermap_data.at(xy) != this->m_heightmap_data.at(xy) + 48) return true;

    int id = int(this->m_texturec1_A_index_data.at(xy));
    int id_second = int(this->m_texturec1_B_index_data.at(xy));
    if (!id || !id_second) {
      // Has a water texture, but not set to water in flag. Original engine did the same check.
      return true;
    }

    return false;
  }
}

bool C2MapFile::hasWaterAt(int x, int y)
{
  int xy = (y * this->getWidth()) + x;

  return this->hasWaterAt(xy);
}

// true if the tile has water as specified in the original file flags
bool C2MapFile::hasOriginalWaterAt(int xy)
{
  if (m_type == CEMapType::C2) {
    uint16_t flags = this->getFlagsAt(xy);
    if (flags & 0x0080) return true;

    return false;
  } else {
    throw 1; // Not supported
  }
}

bool C2MapFile::hasOriginalWaterAt(int x, int y)
{
  int xy = (y * this->getWidth()) + x;
  return this->hasOriginalWaterAt(xy);
}

// true if the tile has water but it was marked by the engine
bool C2MapFile::hasDynamicWaterAt(int xy)
{
  if (m_type == CEMapType::C2) {
    uint16_t flags = this->getFlagsAt(xy);
    if (flags & 0x8000) return true;

    return false;
  } else {
    uint8_t flags = this->getFlagsAt(xy);
    if (flags & 0x8000) return true;

    return false;
  }
}
bool C2MapFile::hasDynamicWaterAt(int x, int y)
{
  int xy = (y * this->getWidth()) + x;
  return this->hasDynamicWaterAt(xy);
}

void C2MapFile::setWaterAt(int x, int y)
{
  if (m_type == CEMapType::C2) {
    this->setWaterAt(x, y, -1);
  } else {
    throw 1; // Only C2 uses flags for water data
  }
}

void C2MapFile::setWaterAt(int x, int y, int water_height)
{
  int xy = (y * this->getWidth()) + x;

  if (m_type == CEMapType::C2) {
    this->m_flags_data.at(xy) |= 0x8000;
  } else {
    // meh?
    this->m_c1_flags_data[xy] |= 0x80;
    this->m_watermap_data[xy] = this->m_heightmap_data.at(xy) + 48;
  }
}

int C2MapFile::getWaterAt(int xy)
{
  if (m_type == CEMapType::C2) {
    return this->m_watermap_data.at(xy);
  } else {
    return 0; // Only 1 core water type in C1 (but also need to handle lava and ponds)
  }
}

float C2MapFile::getLowestHeight(int x, int y)
{
  int width = (int)this->getWidth();

  std::array<int, 4> quad_locations = {
    (y * width) + x,
    ((y+1) * width) + x,
    (y * width) + (x + 1),
    ((y+1) * width) + (x + 1),
  };

  uint8_t lowest = this->m_heightmap_data.at(quad_locations[0]);

  for (int e=1; e < quad_locations.size(); e++) {
    uint8_t h = this->m_heightmap_data.at(quad_locations[e]);

    if (h < lowest) lowest = h;
  }

  return lowest;
}

float C2MapFile::getObjectHeightAt(int xy)
{
  float object_height = this->m_object_heightmap_data.at(xy);

  float scaled_height = object_height * this->getHeightmapScale();

  return (scaled_height);
}

float C2MapFile::interpolateHeight(float x, float z)
{
  int x0 = static_cast<int>(floor(x));
  int z0 = static_cast<int>(floor(z));
  int x1 = x0 + 1;
  int z1 = z0 + 1;

  float Q11 = this->getHeightAt((this->getWidth() * z0) + x0);
  float Q21 = this->getHeightAt((this->getWidth() * z0) + x1);
  float Q12 = this->getHeightAt((this->getWidth() * z1) + x0);
  float Q22 = this->getHeightAt((this->getWidth() * z1) + x1);

  float xRatio = x - x0;
  float zRatio = z - z0;

  float R1 = Q11 * (1 - xRatio) + Q21 * xRatio;
  float R2 = Q12 * (1 - xRatio) + Q22 * xRatio;

  float interpolatedHeight = R1 * (1 - zRatio) + R2 * zRatio;

  return interpolatedHeight;
}

float C2MapFile::getPlaceGroundHeight(int x, int y) {
  // Original Carnivores implementation just returns the lowest height of the approximate quad but we memoize actual vertices during terrain build
  int xy = (y * this->getWidth()) + x;
  return m_ground_levels.at(xy) + 12.f;
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

void C2MapFile::fillWater(int x, int y, int source_x, int source_y)
{
  if (m_type == CEMapType::C1) {
    int srcxy = (source_y * this->getWidth()) + source_x;
    int h = this->m_heightmap_data.at(srcxy);
    this->setWaterAt(x, y, h);
  } else {
    this->setWaterAt(x, y);
    this->copyWaterMap(x, y, source_x, source_y);
  }
}

void C2MapFile::copyWaterMap(int x, int y, int src_x, int src_y)
{
  int xy = (y * this->getWidth()) + x;
  int srcxy = (src_y * this->getWidth()) + src_x;

  if (m_type == CEMapType::C2) {
    this->m_watermap_data.at(xy) = this->m_watermap_data.at(srcxy);
  }
}

glm::vec2 C2MapFile::getXYAtWorldPosition(glm::vec2 pos)
{
  glm::vec2 xy(floorf(pos.x / this->getTileLength()), floorf(pos.y / this->getTileLength()));

  return xy;
}

float C2MapFile::getAngleBetweenPoints(glm::vec3 a, glm::vec3 b)
{
  glm::vec3 da=glm::normalize(a);
  glm::vec3 db=glm::normalize(b);
  return glm::acos(glm::dot(da, db));
}

glm::vec3 C2MapFile::getRandomLanding()
{
  int num_landings = (int)m_landings.size();
  glm::vec2 landing;

  if (num_landings > 0) {
    int r_landing = rand() % num_landings;
    landing = m_landings.at(r_landing);
  } else {
    landing = glm::vec2((int)getWidth() / 2, (int)getHeight() / 2);
  }

  int xy = (landing.y * getHeight()) + landing.x;
  return glm::vec3(
    (landing.x * getTileLength()) + (getTileLength() / 2),
    getHeightAt(xy) + 1024.f,
    (landing.y * getTileLength()) + (getTileLength() / 2)
  );
}

/*
 * fix flags, etc
 */
void C2MapFile::postProcess(C2MapRscFile* crsc_weak)
{
  int w = (int)this->getWidth();
  int h = (int)this->getHeight();
  
  for (int y = 1; y < w-1; y++)
    for (int x = 1; x < h-1; x++) {
      int xy = (y * w) + x;

      if (this->getObjectAt(xy) == 254) {
        m_landings.push_back(glm::vec2(x, y));
      }

      if (m_type == CEMapType::C1 || crsc_weak->getWaterCount() < 1) continue;

      if (!this->hasOriginalWaterAt(xy)) {
        if (this->hasOriginalWaterAt(x+1, y)) this->fillWater(x, y, x+1, y);
        if (this->hasOriginalWaterAt(x, y+1)) this->fillWater(x, y, x, y+1);
        if (this->hasOriginalWaterAt(x-1, y)) this->fillWater(x, y, x-1, y);
        if (this->hasOriginalWaterAt(x, y-1)) this->fillWater(x, y, x, y-1);
        if (this->hasOriginalWaterAt(x-1, y-1)) this->fillWater(x, y, x-1, y-1);
        if (this->hasOriginalWaterAt(x+1, y-1)) this->fillWater(x, y, x+1, y-1);
        if (this->hasOriginalWaterAt(x-1, y+1)) this->fillWater(x, y, x-1, y+1);
        if (this->hasOriginalWaterAt(x+1, y+1)) this->fillWater(x, y, x+1, y+1);

        if (this->hasDynamicWaterAt(xy)) {
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

    // This is used in the original engine to calculate object "darkness" on a given tile - we don't really need it anymore
    infile.read(reinterpret_cast<char *>(this->m_day_brightness_data.data()), 512*512);

    infile.read(reinterpret_cast<char *>(this->m_watermap_data.data()), 512*512); // C1 has only 1 water - these are the heights of the underwater surface
    infile.read(reinterpret_cast<char *>(this->m_object_heightmap_data.data()), 512*512);
    infile.read(reinterpret_cast<char *>(this->m_fog_data.data()), 256*256);
    infile.read(reinterpret_cast<char *>(this->m_soundfx_data.data()), 256*256);

    infile.close();
    this->postProcess(crsc_weak);
  } catch (const std::ios_base::failure& e) {
    std::cerr << "I/O error: " << e.what() << std::endl;
    throw;
  } catch (const std::runtime_error& e) {
      std::cerr << "Runtime error: " << e.what() << std::endl;
    throw;
  } catch (...) {
      std::cerr << "An unknown error occurred." << std::endl;
    
    throw;
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
    infile.read(reinterpret_cast<char *>(this->m_flags_data.data()), 1024*1024*2);

    infile.read(reinterpret_cast<char *>(this->m_dawn_brightness_data.data()), 1024*1024);
    infile.read(reinterpret_cast<char *>(this->m_day_brightness_data.data()), 1024*1024);
    infile.read(reinterpret_cast<char *>(this->m_night_brightness_data.data()), 1024*1024);

    infile.read(reinterpret_cast<char *>(this->m_watermap_data.data()), 1024*1024);
    infile.read(reinterpret_cast<char *>(this->m_object_heightmap_data.data()), 1024*1024);
    infile.read(reinterpret_cast<char *>(this->m_fog_data.data()), 512*512);
    infile.read(reinterpret_cast<char *>(this->m_soundfx_data.data()), 512*512);

    infile.close();
    
    this->postProcess(crsc_weak);
  } catch (const std::ios_base::failure& e) {
    std::cerr << "I/O error: " << e.what() << std::endl;
    throw;
  } catch (const std::runtime_error& e) {
      std::cerr << "Runtime error: " << e.what() << std::endl;
    throw;
  } catch (...) {
      std::cerr << "An unknown error occurred." << std::endl;
    
    throw;
  }
}
