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

#include <glm/glm.hpp>

class C2MapRscFile;
struct _Water;

class C2MapFile
{
private:
  std::array<uint8_t, 1024 * 1024> m_heightmap_data = {};
  std::array<uint8_t, 512 * 512> m_texturec1_A_index_data = {};
  std::array<uint8_t, 512*512> m_texturec1_B_index_data = {};
  std::array<uint16_t, 1024*1024> m_texture_A_index_data = {};
  std::array<uint16_t, 1024*1024> m_texture_B_index_data = {};
  std::array<uint8_t, 1024*1024> m_object_index_data = {};
  std::array<uint16_t, 1024*1024> m_flags_data = {};
  std::array<uint8_t, 512*512> m_c1_flags_data = {};
  std::array<uint8_t, 1024*1024> m_dawn_brightness_data = {};
  std::array<uint8_t, 1024*1024> m_day_brightness_data = {};
  std::array<uint8_t, 1024*1024> m_night_brightness_data = {};
  std::array<uint8_t, 1024*1024> m_watermap_data = {};
  std::array<uint8_t, 1024*1024> m_object_heightmap_data = {};
  std::array<uint8_t, 512*512> m_fog_data = {};
  std::array<uint8_t, 512*512> m_soundfx_data = {};
  std::vector<glm::vec2> m_landings;
  
  // Store computed ground level heights
  std::array<float, 1024 * 1024> m_ground_levels = {};
  std::array<float, 1024 * 1024> m_ground_angles = {};
  std::array<uint16_t, 1024*1024> m_walkable_flags_data = {};

  constexpr static const int SIZE = 1024;
  constexpr static const int SIZE_C1 = 512;
  constexpr static const float HEIGHT_SCALE = 64.f;
  constexpr static const float HEIGHT_SCALE_C1 = 32.f;
  
  void postProcess(std::weak_ptr<C2MapRscFile> rsc);
  void fillWater(int x, int y, int src_x, int src_y);
  void copyWaterMap(int x, int y, int src_x, int src_y);
  
  int XY(glm::vec2 tilePos);

public:
  const CEMapType m_type;

  C2MapFile(const CEMapType map_type, const std::string& map_file_name, std::weak_ptr<C2MapRscFile> rsc);
  ~C2MapFile();

  int getWaterAt(int xy);
  float getHeightAt(int xy);
  float getLowestHeight(int x, int y, bool waterOnly);
  float getObjectHeightAt(int xy);
  float GetLandHeightAt(glm::vec3 position);
  float getPlaceGroundHeight(int x, int y);
  float getGroundAngleAt(int x, int y);
  
  // Enhanced height sampling with sub-tile precision
  float getInterpolatedGroundHeight(float world_x, float world_z);
  float getHeightAtWorldPosition(const glm::vec3& worldPos);
  float getWaterHeightAt(int x, int y);
  int getObjectHeightForRadius(int x, int y, int R);
  int getObjectAt(int xy);
  
  // Get terrain height (excluding water) for depth calculations
  float getTerrainHeightAt(int xy);

  int getTextureIDAt(int xy);
  int getSecondaryTextureIDAt(int xy);

  int getWaterTextureIDAt(int xy, int water_texture_id);
  int getAmbientAudioIDAt(int x, int y);

  bool isQuadRotatedAt(int xy);
  uint16_t getFlagsAt(int xy);
  uint16_t getFlagsAt(int x, int y);
  uint16_t getWalkableFlagsAt(glm::vec2 tile);
  void setWalkableFlagsAt(glm::vec2 tile, uint16_t flags);

  glm::vec2 getXYAtWorldPosition(glm::vec2 pos);
  glm::vec3 getPositionAtCenterTile(glm::vec2 pos);
  glm::vec2 getWorldTilePosition(glm::vec3 pos);
  
  glm::vec3 getRandomLanding();

  float getAngleBetweenPoints(glm::vec3 a, glm::vec3 b);

  int getHeight();
  int getWidth();
  float getTileLength();
  float getHeightmapScale();
  float getBrightnessAt(int xy);
  float getBrightnessAt(int x, int y);

  bool hasWaterAt(int xy);
  bool hasWaterAt(int x, int y);

  bool hasOriginalWaterAt(int xy);
  bool hasOriginalWaterAt(int x, int y);
  bool hasDynamicWaterAt(int xy);
  bool hasDynamicWaterAt(int x, int y);
  bool hasDangerTileAt(std::shared_ptr<C2MapRscFile> rsc, glm::vec2 tile);

  void setWaterAt(int x, int y); // for C2
  void setWaterAt(int x, int y, int water_height);// c1
  
  void setGroundLevelAt(int x, int y, float level, float angle);

  void load(const std::string& file_name, std::weak_ptr<C2MapRscFile> rsc);
  void load_c1(const std::string& file_name, std::weak_ptr<C2MapRscFile> rsc);
};

#endif /* defined(__CE_Character_Lab__C2MapFile__) */
