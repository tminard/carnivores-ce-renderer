//
//  C2MapRscFile.cpp
//  CE Character Lab
//
//  Created by Tyler Minard on 8/14/15.
//  Copyright (c) 2015 Tyler Minard. All rights reserved.
//

#include "C2MapRscFile.h"
#include <iostream>
#include <fstream>

#include "CETexture.h"
#include "CEWorldModel.h"
#include "C2Sky.h"
#include "CEWaterEntity.h"

#include "deps/libAF/af2-sound.h"
#include "CEAudioSource.hpp"

#include <stdlib.h>

using libAF2::Sound;

template <typename T, typename... Args>
auto make_unique(Args&&... args) -> std::unique_ptr<T>
{
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
void Console_PrintLogString(std::string log_msg);

C2MapRscFile::C2MapRscFile(const CEMapType type, const std::string& file_name) : m_type(type)
{
  this->load(file_name);
}

C2MapRscFile::~C2MapRscFile()
{
  
}

int C2MapRscFile::getWorldModelCount()
{
  return (int)this->m_models.size();
}

CETexture* C2MapRscFile::getTexture(int i)
{
  return this->m_textures.at(i).get();
}

CEWorldModel* C2MapRscFile::getWorldModel(int i)
{
  if (i < 0 || i >= this->m_models.size()) {
    return nullptr;
  }
  
  return this->m_models.at(i).get();
}

int C2MapRscFile::getTextureAtlasWidth()
{
  return this->m_texture_atlas_width;
}

C2Sky* C2MapRscFile::getDaySky()
{
  return this->m_day_sky.get();
}

void C2MapRscFile::setWaterHeight(int i, int h_unscaled)
{
  CEWaterEntity wd = this->m_waters.at(i);
  wd.water_level = h_unscaled;

  this->m_waters.at(i) = wd;
}

void C2MapRscFile::playRandomAudio(int x, int y, int z)
{
  // play random audio at this point
  int rnd = rand() % (this->m_random_audio_sources.size()-1);
  CEAudioSource* src = this->m_random_audio_sources.at(rnd).get();
  src->setPosition(glm::vec3(x, y, z));
  src->play();
}

void C2MapRscFile::playAmbientAudio(int i)
{
  CEAudioSource* src = this->m_ambient_audio_sources.at(i).get();
  src->play();
}

void C2MapRscFile::load(const std::string &file_name)
{
  std::ifstream infile;
  infile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  
  try {
    infile.open(file_name.c_str(), std::ios::binary | std::ios::in);
    
    int texture_count, model_count;
    int SOURCE_SQUARE_SIZE = 128; // each texture in C2 is 128x128
    int TEXTURE_SQUARE_SIZE = 130; // we add 2 px to each side for clamping, for 130x130
    
    infile.read(reinterpret_cast<char *>(&texture_count), 4);
    infile.read(reinterpret_cast<char *>(&model_count), 4);

    if (m_type == C2) {
      infile.read(reinterpret_cast<char *>(this->m_fade_rgb), 4*3*3);
      infile.read(reinterpret_cast<char *>(this->m_trans_rgb), 4*3*3);
    } else {
      infile.read(reinterpret_cast<char *>(this->m_fade_rgb), 4*3);
      infile.read(reinterpret_cast<char *>(this->m_trans_rgb), 4*3);
    }
    
    std::vector<uint16_t> raw_texture_data; //rgba5551
    raw_texture_data.resize(SOURCE_SQUARE_SIZE*SOURCE_SQUARE_SIZE*texture_count);
    infile.read(reinterpret_cast<char *>(raw_texture_data.data()), SOURCE_SQUARE_SIZE*SOURCE_SQUARE_SIZE*sizeof(uint16_t)*texture_count);
    
    // Combine all the textures into a single texture for ease of opengl use.
    // Add a buffer around images to mimic clamping with GL_LINEAR; see: https://stackoverflow.com/questions/19611745/opengl-black-lines-in-between-tiles
    std::vector<uint16_t> combined_texture_data; // rgba5551
    
    int squared_texture_rows = static_cast<int>(sqrt(static_cast<float>(texture_count)) + .99f); // number of rows and columns needed to fit the data
    
    uint16_t pad_color;
    std::vector<uint16_t> tx_filler(TEXTURE_SQUARE_SIZE*squared_texture_rows);
    
    for (int line = 0; line < (squared_texture_rows*SOURCE_SQUARE_SIZE); line++) { // Total vertical rows in image file (128 * number of images; current = pointer to vertical pixel location in file that represents the location of a new row of horizontal blocks)
      
      if (line % SOURCE_SQUARE_SIZE == 0) {
        pad_color = _PadTypeColor::Red;
        tx_filler.clear();
        tx_filler.assign(TEXTURE_SQUARE_SIZE*squared_texture_rows, pad_color);
        
        for (int x=0; x < squared_texture_rows; x++) {
          tx_filler.at(x*TEXTURE_SQUARE_SIZE) = _PadTypeColor::Blue;
          tx_filler.at((x*TEXTURE_SQUARE_SIZE)+(TEXTURE_SQUARE_SIZE-1)) = _PadTypeColor::Green;
        }
        combined_texture_data.insert(combined_texture_data.end(), tx_filler.begin(), tx_filler.end());
      }
      
      for (int col = 0; col < squared_texture_rows; col++) { // Blocks (number of images; current = current block's image)
        int lin_to_row = static_cast<int>(line/SOURCE_SQUARE_SIZE);
        int tx_c = ((lin_to_row)*squared_texture_rows)+col;
        int tex_start = line % SOURCE_SQUARE_SIZE;
        
        pad_color = _PadTypeColor::Blue;
        combined_texture_data.insert(combined_texture_data.end(), pad_color);
        
        if (tx_c >= texture_count) {
          std::vector<uint16_t> tx_filler(SOURCE_SQUARE_SIZE);
          combined_texture_data.insert(combined_texture_data.end(), tx_filler.begin(), tx_filler.end());
        } else {
          std::vector<uint16_t>::const_iterator first = raw_texture_data.begin() + (tx_c*SOURCE_SQUARE_SIZE*SOURCE_SQUARE_SIZE) + (tex_start*SOURCE_SQUARE_SIZE); // Get start pos of a single line of pixels
          std::vector<uint16_t>::const_iterator last = raw_texture_data.begin() + (tx_c*SOURCE_SQUARE_SIZE*SOURCE_SQUARE_SIZE) + (tex_start*SOURCE_SQUARE_SIZE) + SOURCE_SQUARE_SIZE; // Get end pos, which is the start + 128 pixels
          combined_texture_data.insert(combined_texture_data.end(), first, last);
        }
        
        pad_color = _PadTypeColor::Green;
        combined_texture_data.insert(combined_texture_data.end(), pad_color);
      }
      
      if ((line+1) % SOURCE_SQUARE_SIZE == 0) {
        pad_color = _PadTypeColor::Yellow;
        tx_filler.clear();
        tx_filler.assign(TEXTURE_SQUARE_SIZE*squared_texture_rows, pad_color);
        
        for (int x=0; x < squared_texture_rows; x++) {
          tx_filler.at(x*TEXTURE_SQUARE_SIZE) = _PadTypeColor::Blue;
          tx_filler.at((x*TEXTURE_SQUARE_SIZE)+(TEXTURE_SQUARE_SIZE-1)) = _PadTypeColor::Green;
        }
        
        combined_texture_data.insert(combined_texture_data.end(), tx_filler.begin(), tx_filler.end());
      }
    }
    
    int missing_bits = (squared_texture_rows*TEXTURE_SQUARE_SIZE*squared_texture_rows*TEXTURE_SQUARE_SIZE) - static_cast<int>(combined_texture_data.size());
    
    if (missing_bits > 0) {
      std::vector<uint16_t> tx_filler(missing_bits);
      combined_texture_data.insert(combined_texture_data.end(), tx_filler.begin(), tx_filler.end());
    }

      // TODO: we can use a frame buffer instead of this nastiness
    std::vector<uint16_t> final_texture_data; // Now replace colors with correct entries
    for (int i=0; i < 2; i++) {
      for (int row = 0; row < (TEXTURE_SQUARE_SIZE*squared_texture_rows); row++) {
        for (int col = 0; col < (TEXTURE_SQUARE_SIZE*squared_texture_rows); col++) {
          int id_x = col;
          int id_y = row*TEXTURE_SQUARE_SIZE*squared_texture_rows;
          uint16_t color;
          
          if (i == 1) {
            color = final_texture_data.at(id_y+id_x);
          } else {
            color = combined_texture_data.at(id_y+id_x);
          }
          
          switch (color) {
            case _AtlasPadType::Left:
              color = final_texture_data.at(id_y+(id_x-1));
              break;
            case _AtlasPadType::Right:
              if (i == 1) {
                color = final_texture_data.at(id_y+(id_x+1));
              } else {
                color = combined_texture_data.at(id_y+(id_x+1));
              }
              break;
            case _AtlasPadType::Above:
              color = final_texture_data.at(((row-1)*TEXTURE_SQUARE_SIZE*squared_texture_rows)+id_x);
              break;
            case _AtlasPadType::Below:
              if (i == 1) {
                color = final_texture_data.at(((row+1)*TEXTURE_SQUARE_SIZE*squared_texture_rows)+id_x);
              } else {
                color = combined_texture_data.at(((row+1)*TEXTURE_SQUARE_SIZE*squared_texture_rows)+id_x);
              }
              break;
            default:
              break;
          }
          if (i == 1) {
            final_texture_data.at(id_y+id_x) = color;
          } else {
            final_texture_data.insert(final_texture_data.end(), color);
          }
        }
      }
    }
    
    std::unique_ptr<CETexture> cTexture = std::unique_ptr<CETexture>(new CETexture(final_texture_data, squared_texture_rows*TEXTURE_SQUARE_SIZE*squared_texture_rows*TEXTURE_SQUARE_SIZE, squared_texture_rows*TEXTURE_SQUARE_SIZE, squared_texture_rows*TEXTURE_SQUARE_SIZE));
    this->m_texture_atlas_width = squared_texture_rows;
    this->m_texture_count = texture_count;
    this->m_textures.push_back(std::move(cTexture));
    
    
    // Load 3d models
    for (int m=0; m < model_count; m++) {
      std::unique_ptr<CEWorldModel> cModel = std::unique_ptr<CEWorldModel>(new CEWorldModel(m_type, infile));
      this->m_models.push_back(std::move(cModel));
    }
    
    // Load sky bitmap and map overlay (dawn, day, night)
    if (m_type == C2) {
      this->m_dawn_sky = std::unique_ptr<C2Sky>(new C2Sky(infile));
      this->m_dawn_sky->setRGBA(glm::vec4(m_fade_rgb[0][0], m_fade_rgb[0][1], m_fade_rgb[0][2], 1.f));
    }

    this->m_day_sky = std::unique_ptr<C2Sky>(new C2Sky(infile));

    if (m_type == C2) {
      this->m_day_sky->setRGBA(glm::vec4(m_fade_rgb[1][0], m_fade_rgb[1][1], m_fade_rgb[1][2], 1.f));
    } else {
      this->m_day_sky->setRGBA(glm::vec4(m_fade_rgb[0][0], m_fade_rgb[0][1], m_fade_rgb[0][2], 1.f));
    }

    if (m_type == C2) {
      this->m_night_sky = std::unique_ptr<C2Sky>(new C2Sky(infile));
      this->m_night_sky->setRGBA(glm::vec4(m_fade_rgb[2][0], m_fade_rgb[2][1], m_fade_rgb[2][2], 1.f));
    }
    
    this->m_shadow_map.resize(128*128);
    infile.read(reinterpret_cast<char *>(this->m_shadow_map.data()), 128*128);
    
    /*
     * byte: 1 byte, 0..255
     * word: 2 bytes, LSB->MSB, 0..65535
     * short: 2 bytes, LSB->MSB, -32768..32767
     * long: 4 bytes, LSB->MSB, -2147483648..2147483647
     * single: 4 bytes, LSB->MSB (IEEE single-precision floating point format) */
    // Load fog data
    struct FogData {
      int32_t rgb;
      float altitude;
      int32_t danger;
      float transparency;
      float hlimit;
    };
    
    int fog_count;
    FogData fd[64];
    infile.read(reinterpret_cast<char *>(&fog_count), 4);
    infile.read(reinterpret_cast<char *>(&fd), sizeof(FogData)*fog_count);

    // Load rnd sounds
    infile.read(reinterpret_cast<char *>(&this->m_random_sounds_count), 4);
    for (int i = 0; i < this->m_random_sounds_count; i++)
    {
      std::shared_ptr<Sound> snd(new Sound());
      uint32_t length = 0;
      int8_t* snd_data = nullptr;

      infile.read((char*)&length, sizeof(uint32_t));
      snd_data = new int8_t [ length ];
      infile.read((char*)snd_data, length);

      snd->setName("RndAudio");
      snd->setWaveData(16, 1, length, 22050, snd_data);

      delete [] snd_data;

      this->m_random_sounds.push_back(snd);

      std::unique_ptr<CEAudioSource> src(new CEAudioSource(snd));

      src->setClampDistance((256*2));
      src->setMaxDistance((256*200));

      this->m_random_audio_sources.push_back(std::move(src));
    }

    // Skip table for now
    infile.read(reinterpret_cast<char *>(&this->m_ambient_sounds_count), 4);
    for (int i = 0; i < this->m_ambient_sounds_count; i++)
    {
      std::shared_ptr<Sound> snd(new Sound());

      int8_t* snd_data = nullptr;
      uint32_t length = 0;
      infile.read((char*)&length, sizeof(uint32_t));
      snd_data = new int8_t [ length ];
      infile.read((char*)snd_data, length);

      snd->setName("AmbientAudio");
      snd->setWaveData(16, 1, length, 22050, snd_data);

      delete [] snd_data;

      this->m_ambient_sounds.push_back(snd);
      std::unique_ptr<CEAudioSource> src(new CEAudioSource(snd));

      src->setClampDistance((256*1));
      src->setMaxDistance((256*2));
      src->setNoDistance(0.25);
      src->setLooped(true);

      this->m_ambient_audio_sources.push_back(std::move(src));

      // TODO
      infile.seekg((16*16) + 8, std::ios_base::cur);

      // read data first, then a 16x16 table
      // For each of these ambient sounds, read a table 16 entries long that maps to the random sounds, and describes which random sounds play in the ambient area and how frequent, etc
    }

    if (m_type == C1) {
      CEWaterEntity wd;
      wd.texture_id = 0;
      //wd.water_level = 0; // actually height map height - use magic number to calc on the fly
      wd.transparency = 0.8f;

      this->m_waters.push_back(wd);
      return;
    }

    infile.read(reinterpret_cast<char *>(&this->m_num_waters), 4);
    for (int i = 0; i < this->m_num_waters; i++)
    {
      CEWaterEntity wd;

      infile.read((char*)&wd, sizeof(CEWaterEntity));

      this->m_waters.push_back(wd);
    }

    
    infile.close();
  } catch (std::ifstream::failure e) {
    Console_PrintLogString("Failed to load " + file_name + ": " + strerror(errno));
    std::cerr << "Exception opening/reading/closing file\n";
  }
}

int C2MapRscFile::getWaterCount()
{
  return (int)this->m_waters.size();
}

const CEWaterEntity& C2MapRscFile::getWater(int i) const
{
  return this->m_waters.at(i);
}
