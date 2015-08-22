/*
 * Stores C2 style textures (and mipmap).
 * Use this class to access and convert the TGA textures used in the car and rsc files.
 * (16-bit TGA-style encoding in ARGB1555 format with the first bit indicating alpha, always 256 pixels wide)
 */

#ifndef __CE_Character_Lab__C2Texture__
#define __CE_Character_Lab__C2Texture__

#include <stdio.h>

#include "g_shared.h"

#include <memory>
#include <array>
#include <cstdint>

#include <vector>
#include <string>

#include <glfw/glfw3.h>

class Shader;
class CBitmap;
class C2Texture
{
private:
  std::vector<uint16_t> m_raw_data; //argb1555 (ifA1R5G5B5)
  GLuint m_texture_id;
  int m_height;
  int m_width;

  void loadTextureIntoHardwareMemory();
public:
  C2Texture(const std::vector<uint16_t>& raw_texture_data, int texture_size = 128*128*2, int texture_height = 128, int texture_width = 128);
  ~C2Texture();

  void saveToBMPFile(std::string file_name);
  void Use();
  std::unique_ptr<CBitmap> getCBitmap();
};

#endif /* defined(__CE_Character_Lab__C2Texture__) */
