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
#include <algorithm>

  // TODO: DRY this up and remove interdeps
#include "C2MapRscFile.h"
#include "CEWaterEntity.h"

#include <math.h>

C2MapFile::C2MapFile(const CEMapType type, const std::string& map_file_name, std::weak_ptr<C2MapRscFile> rsc) : m_type(type)
{
  if (!std::filesystem::exists(map_file_name)) {
      throw std::runtime_error("File not found: " + map_file_name);
  } else {
    std::cout << "Found " << map_file_name << "; OK. Loading..." << std::endl;
  }
  
  switch (m_type) {
  case CEMapType::C2:
      this->load(map_file_name, rsc);
      break;
  case CEMapType::C1:
      this->load_c1(map_file_name, rsc);
  }

}

C2MapFile::~C2MapFile()
{
}

// Take tile x,y and return actual x,y,z marking the center of the tile
// at world coordinates and at ground or water level
glm::vec3 C2MapFile::getPositionAtCenterTile(glm::vec2 pos)
{
  int x = pos.x;
  int y = pos.y;

  if (x < 0 || x > getWidth() || y < 0 || y > getHeight()) {
    return glm::vec3(0.f);
  }

  bool hasWater = hasWaterAt(x, y);

  float height = hasWater ? getWaterHeightAt(x, y) : getPlaceGroundHeight(x, y);

  float tile = getTileLength();
  float halfTile = tile / 2.f;

  return glm::vec3((static_cast<float>(x) * tile) + halfTile, height, (static_cast<float>(y) * tile) + halfTile);
}

glm::vec2 C2MapFile::getWorldTilePosition(glm::vec3 pos)
{
  return glm::vec2(int(floorf(pos.x / getTileLength())), int(floorf(pos.z / getTileLength())));
}

int C2MapFile::getAmbientAudioIDAt(int x, int y)
{
  // Covers a 4x4 zone area
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
  if (m_type == CEMapType::C2) {
    return -1;
  }

  // In c1, half water tiles have anchor vertex (at xy) in the ground, and thus calculate a different height. Fix this here
  float lowest_height = this->getLowestHeight(x, y, true);

  float scaled_height = (lowest_height + 48.f) * this->getHeightmapScale();

  return (scaled_height);
}

int C2MapFile::XY(glm::vec2 tilePos)
{
  int xy = (tilePos.y * this->getWidth()) + tilePos.x;

  return xy;
}

float C2MapFile::GetLandHeightAt(glm::vec3 position)
{
  int tileLength = static_cast<int>(getTileLength());
  float tileLengthf = getTileLength();
  bool isC2Style = m_type == CEMapType::C2;

  glm::vec2 tilePosition = glm::vec2(static_cast<int>(position.x) / tileLength, static_cast<int>(position.z) / tileLength);
  glm::vec2 quadOffset = glm::vec2(static_cast<int>(position.x) % tileLength, static_cast<int>(position.z) % tileLength);
  
  std::array<int, 4> unscaledHeights = {
    isC2Style ? m_heightmap_data.at(XY(tilePosition)) : m_watermap_data.at(XY(tilePosition)),
    isC2Style ? m_heightmap_data.at(XY(tilePosition + glm::vec2(1, 0))) : m_watermap_data.at(XY(tilePosition + glm::vec2(1, 0))),
    isC2Style ? m_heightmap_data.at(XY(tilePosition + glm::vec2(1, 1))) : m_watermap_data.at(XY(tilePosition + glm::vec2(1, 1))),
    isC2Style ? m_heightmap_data.at(XY(tilePosition + glm::vec2(0, 1))) : m_watermap_data.at(XY(tilePosition + glm::vec2(0, 1)))
  };
  
  if (isQuadRotatedAt(XY(tilePosition))) {
    if (tileLength - quadOffset.x > quadOffset.y) {
      unscaledHeights.at(2) = unscaledHeights.at(1) + unscaledHeights.at(3) - unscaledHeights.at(0);
    } else {
      unscaledHeights.at(0) = unscaledHeights.at(1) + unscaledHeights.at(3) - unscaledHeights.at(2);
    }
  } else {
    if (quadOffset.x > quadOffset.y) {
      unscaledHeights.at(3) = unscaledHeights.at(0) + unscaledHeights.at(2) - unscaledHeights.at(1);
    } else {
      unscaledHeights.at(1) = unscaledHeights.at(0) + unscaledHeights.at(2) - unscaledHeights.at(3);
    }
  }
  
  float unscaledHeight = (float)
       (unscaledHeights.at(0)   * (256-quadOffset.x) + unscaledHeights.at(1) * quadOffset.x) * (256-quadOffset.y) +
       (unscaledHeights.at(3)   * (256-quadOffset.x) + unscaledHeights.at(2) * quadOffset.x) * quadOffset.y;
  
  if (isC2Style) {
    return (unscaledHeight / tileLengthf / tileLengthf) * getHeightmapScale();
  }
  
  return (unscaledHeight / tileLengthf / tileLengthf - 48.f) * getHeightmapScale();
}

// C1 code for land height at exact position
//float GetLandH(float x, float y)
//{
//   int CX = (int)x / 256;
//   int CY = (int)y / 256;
//   
//   int dx = (int)x % 256;
//   int dy = (int)y % 256;
//
//   int h1 = HMap2[CY][CX];
//   int h2 = HMap2[CY][CX+1];
//   int h3 = HMap2[CY+1][CX+1];
//   int h4 = HMap2[CY+1][CX];
//
//
//   if (FMap[CY][CX] & fmReverse) {
//     if (256-dx>dy) h3 = h2+h4-h1;
//               else h1 = h2+h4-h3;
//   } else {
//     if (dx>dy) h4 = h1+h3-h2;
//           else h2 = h1+h3-h4;
//   }
//
//   float h = (float)
//     (h1   * (256-dx) + h2 * dx) * (256-dy) +
//     (h4   * (256-dx) + h3 * dx) * dy;
//
//   return  (h / 256.f / 256.f - 48) * ctHScale;
//}
//
//// C2 code for land height at exact position
//float GetLandH(float x, float y)
//{
//   int CX = (int)x / 256;
//   int CY = (int)y / 256;
//   
//   int dx = (int)x % 256;
//   int dy = (int)y % 256;
//
//   int h1 = HMap[CY][CX];
//   int h2 = HMap[CY][CX+1];
//   int h3 = HMap[CY+1][CX+1];
//   int h4 = HMap[CY+1][CX];
//
//
//   if (FMap[CY][CX] & fmReverse) {
//     if (256-dx>dy) h3 = h2+h4-h1;
//               else h1 = h2+h4-h3;
//   } else {
//     if (dx>dy) h4 = h1+h3-h2;
//           else h2 = h1+h3-h4;
//   }
//
//   float h = (float)
//     (h1   * (256-dx) + h2 * dx) * (256-dy) +
//     (h4   * (256-dx) + h3 * dx) * dy;
//
//   return  (h / 256.f / 256.f) * ctHScale;
//}

float C2MapFile::getHeightAt(int xy)
{
  float scaled_height;
  auto isUnderwater = 0x80;
  
  if (xy < 0 || xy >= m_heightmap_data.size()) {
    std::cerr << "OOB height requested for heightmap! Returning -1" << std::endl;
    return -1.f;
  }

  if (m_type == CEMapType::C2) {
    scaled_height = this->m_heightmap_data.at(xy) * this->getHeightmapScale();
  } else {
      auto c1WaterHeight = this->m_watermap_data.at(xy);
      auto c1LandHeight = this->m_heightmap_data.at(xy);
      uint8_t flags = this->getFlagsAt(xy);
      if (flags & isUnderwater) {
          // If water is here, use the water height
          scaled_height = (c1WaterHeight) * this->getHeightmapScale();
      }
      else {
          // Otherwise, land height
          scaled_height = (c1LandHeight + 48) * this->getHeightmapScale();
      }
  }

  return (scaled_height);
}

float C2MapFile::getTerrainHeightAt(int xy)
{
  if (xy < 0 || xy >= m_heightmap_data.size()) {
    std::cerr << "OOB terrain height requested! Returning -1" << std::endl;
    return -1.f;
  }

  float scaled_height;
  
  if (m_type == CEMapType::C2) {
    // For C2, heightmap contains terrain height directly
    scaled_height = this->m_heightmap_data.at(xy) * this->getHeightmapScale();
  } else {
    // For C1, we need the actual ground height for depth calculations
    // m_watermap contains the ground level, m_heightmap contains surface level
    // For terrain height (ground), use watermap value directly
    auto c1GroundHeight = this->m_watermap_data.at(xy);
    scaled_height = c1GroundHeight * this->getHeightmapScale();
  }

  return scaled_height;
}

void C2MapFile::setGroundLevelAt(int x, int y, float level, float slopeDegrees)
{
  int xy = (y * this->getWidth()) + x;

  m_ground_levels[xy] = level;
  m_ground_angles[xy] = slopeDegrees;
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
    if (flags & 0x0080 || flags & 0x8000) return true;

    int id = int(this->m_texturec1_A_index_data.at(xy));
    int id_second = int(this->m_texturec1_B_index_data.at(xy));
    if (!id || !id_second) {
      // Has a water texture, but not set to water in flag. Original engine did the same check.
      return true;
    }
    
    // Note some mountains in C1 have a slight delta and incorrectly add water....
    if (this->m_watermap_data.at(xy) != this->m_heightmap_data.at(xy) + 48) {
      return true;
    };

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
  uint16_t flags = this->getFlagsAt(xy);

  if (m_type == CEMapType::C2) {
    if (flags & 0x0080) return true;

    return false;
  } else {
    if (flags & 0x0080) return true;
    return (this->m_watermap_data.at(xy) != this->m_heightmap_data.at(xy) + 48);
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
  this->setWaterAt(x, y, -1);
}

void C2MapFile::setWaterAt(int x, int y, int water_height)
{
  int xy = (y * this->getWidth()) + x;

  if (m_type == CEMapType::C2) {
    this->m_flags_data.at(xy) |= 0x8000;
  } else {
    // meh?
    this->m_c1_flags_data[xy] |= 0x80;
  }
}

int C2MapFile::getWaterAt(int xy)
{
  if (m_type == CEMapType::C2) {
    return this->m_watermap_data.at(xy);
  } else {
    // Use a different method for C1
    throw std::runtime_error("Water type unsupported by selected Carnivores engine.");
  }
}

float C2MapFile::getLowestHeight(int x, int y, bool waterOnly)
{
  int width = (int)this->getWidth();
  int height = (int)this->getHeight();
  
  std::array<int, 9> quad_locations = {
    (y * width) + x,
      // Top-left
      (std::max(y - 1, 0) * width) + std::max(x - 1, 0),
      // Top
      (std::max(y - 1, 0) * width) + x,
      // Top-right
      (std::max(y - 1, 0) * width) + std::min(x + 1, width - 1),
      // Left
      (y * width) + std::max(x - 1, 0),
      // Right
      (y * width) + std::min(x + 1, width - 1),
      // Bottom-left
      (std::min(y + 1, height - 1) * width) + std::max(x - 1, 0),
      // Bottom
      (std::min(y + 1, height - 1) * width) + x,
      // Bottom-right
      (std::min(y + 1, height - 1) * width) + std::min(x + 1, width - 1)
  };

  uint8_t lowest = this->m_heightmap_data.at(quad_locations[0]);

  // We have to do this for water since tiles can have HALF water and half land.
  // For these, we want the water portion to be aligned with the neighboring water and not the land.
  for (int e=1; e < quad_locations.size(); e++) {
    uint8_t h = this->m_heightmap_data.at(quad_locations[e]);

    if (waterOnly) {
      if (hasWaterAt(quad_locations[e]) && h < lowest) lowest = h;
    } else {
      if (h < lowest) lowest = h;
    }
  }

  return lowest;
}

float C2MapFile::getObjectHeightAt(int xy)
{
  float object_height = this->m_object_heightmap_data.at(xy);

  float scaled_height = object_height * this->getHeightmapScale();

  return (scaled_height);
}

float C2MapFile::getPlaceGroundHeight(int x, int y) {
  return m_ground_levels.at(XY(glm::vec2(x, y)));
}

float C2MapFile::getGroundAngleAt(int x, int y) {
  int xy = (y * this->getWidth()) + x;
  return m_ground_angles.at(xy);
}

float C2MapFile::getInterpolatedGroundHeight(float world_x, float world_z) {
    // Convert world coordinates to tile coordinates
    float tile_size = getTileLength();
    float tile_x = world_x / tile_size;
    float tile_z = world_z / tile_size;
    
    // Get the integer tile coordinates for the 4 surrounding tiles
    int x0 = (int)floor(tile_x);
    int z0 = (int)floor(tile_z);
    int x1 = x0 + 1;
    int z1 = z0 + 1;
    
    // Clamp to map bounds
    int width = getWidth();
    int height = getHeight();
    x0 = std::max(0, std::min(x0, width - 1));
    x1 = std::max(0, std::min(x1, width - 1));
    z0 = std::max(0, std::min(z0, height - 1));
    z1 = std::max(0, std::min(z1, height - 1));
    
    // Get heights at the 4 corner vertices using the raw heightmap data
    // This accesses the actual vertex heights, not the precomputed center heights
    float h00 = getHeightAt((z0 * width) + x0); // Bottom-left
    float h10 = getHeightAt((z0 * width) + x1); // Bottom-right
    float h01 = getHeightAt((z1 * width) + x0); // Top-left
    float h11 = getHeightAt((z1 * width) + x1); // Top-right
    
    // Calculate interpolation weights
    float fx = tile_x - x0; // Fractional part in X
    float fz = tile_z - z0; // Fractional part in Z
    
    // Bilinear interpolation
    float h0 = h00 * (1.0f - fx) + h10 * fx; // Interpolate bottom edge
    float h1 = h01 * (1.0f - fx) + h11 * fx; // Interpolate top edge
    float interpolatedHeight = h0 * (1.0f - fz) + h1 * fz; // Final interpolation
    
    return interpolatedHeight;
}

float C2MapFile::getHeightAtWorldPosition(const glm::vec3& worldPos) {
    // Use the enhanced interpolated height sampling
    return getInterpolatedGroundHeight(worldPos.x, worldPos.z);
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

  int xy = (landing.y * getWidth()) + landing.x;
  return glm::vec3(
    (landing.x * getTileLength()) + (getTileLength() / 2),
    getHeightAt(xy) + 1024.f,
    (landing.y * getTileLength()) + (getTileLength() / 2)
  );
}

bool C2MapFile::hasDangerTileAt(std::shared_ptr<C2MapRscFile> rsc, glm::vec2 tile)
{
  // Read fog map and check for "lava" as best we can
  // We can kind of conclude that there's lava if there's a mortal
  // fog map at this location AND the tile height is < the mortal Y
  int xy = (((int)tile.y >> 1) * (getWidth() >> 1)) + ((int)tile.x >> 1);
  int fogIndex = m_fog_data.at(xy);
  
  // 0 = no fog
  if (fogIndex < 1) {
    return false;
  }
  
  // index starts at 1
  fogIndex--;

  FogData fog = rsc->getFog(fogIndex);
  if (fog.danger == 1) {
    // Figure out the heights now to determine if standing on this tile would be dangerous
    // Note: Altitude doesnt appear used? At least not for danger calculation.
    float scale = getHeightmapScale();
    float fogHLimit = fog.hlimit * scale;
    float tileH = getLowestHeight(tile.x, tile.y, false) * scale;

    // Hey we're in the danger smog
    // Note we're adjusting the size DOWN a bit since the C1 fogs appear to be oversized
    return tileH < (fogHLimit / 8.f);
  }
  
  return false;
}

void C2MapFile::postProcess(std::weak_ptr<C2MapRscFile> rsc)
{
  std::shared_ptr<C2MapRscFile> m_rsc = rsc.lock();
  if (!m_rsc) {
    std::cerr << "Cannot aquire reference to RSC for postProcess! RSC no longer loaded." << std::endl;

    throw std::runtime_error("Lost reference to RSC file during map build!");
  }

  int w = (int)this->getWidth();
  int h = (int)this->getHeight();
  
  for (int y = 1; y < w-1; y++)
    for (int x = 1; x < h-1; x++) {
      int xy = (y * w) + (x);

      if (this->getObjectAt(xy) == 254) {
        m_landings.push_back(glm::vec2(x, y));
      }
      
      if (m_type == CEMapType::C1) {
        continue;
      }

      // Process water filling to handle gaps
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
          if (this->m_heightmap_data.at(xy) == m_rsc->getWater(this->m_watermap_data.at(xy)).water_level) {
            this->m_heightmap_data.at(xy) += 1;
          }
        }
      }
    }
}

void C2MapFile::load_c1(const std::string &file_name, std::weak_ptr<C2MapRscFile> rsc)
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
    this->postProcess(rsc);
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

void C2MapFile::load(const std::string &file_name, std::weak_ptr<C2MapRscFile> rsc)
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
    
    this->postProcess(rsc);
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

uint16_t C2MapFile::getWalkableFlagsAt(glm::vec2 tile) {
  int xy = (tile.y * getWidth()) + tile.x;
  return m_walkable_flags_data.at(xy);
}


void C2MapFile::setWalkableFlagsAt(glm::vec2 tile, uint16_t flags) {
  int xy = (tile.y * getWidth()) + tile.x;
  m_walkable_flags_data[xy] = flags;
}

