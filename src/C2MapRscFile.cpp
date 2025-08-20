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
#include "CEGeometry.h"
#include "vertex.h"
#include "transform.h"

#include "dependency/libAF/af2-sound.h"
#include "CEAudioSource.hpp"

#include <stdlib.h>

using libAF2::Sound;

template <typename T, typename... Args>
auto make_unique(Args&&... args) -> std::unique_ptr<T>
{
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

C2MapRscFile::C2MapRscFile(const CEMapType type, const std::string& file_name, std::filesystem::path basePath) : m_type(type)
{
  this->load(file_name, basePath);
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

/*
 Register a water entity for custom use.
 Used by C1 to build on the fly since water is not pre-registered in the RSC.
 
 We consider water the same body if the height and texture are the same.
 */
void C2MapRscFile::registerDynamicWater(const CEWaterEntity& water)
{
  if (findMatchingWater(water) >= 0) return;
  
  m_waters.push_back(water);
  m_num_waters = (int)m_waters.size();
}

int C2MapRscFile::findMatchingWater(const CEWaterEntity& water) const {
  for (int i = 0; i < m_num_waters; i++) {
    if(m_waters[i].texture_id == water.texture_id && m_waters[i].water_level == water.water_level) {
      return i;
    }

    // HACK: Group "close enough" waters to eliminate oddities
    // Note some maps (like area 4) in C1 have hacks to hide map issues
    // like painting water over spots that are too high for water.
    // This helps eliminate most of them.
    if(m_waters[i].texture_id == water.texture_id) {
      if (abs(m_waters[i].water_level - water.water_level) < 8) {
        return i;
      }
    }
  }
  
  return -1;
}

float getRandomFloat(float min, float max) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(min, max);
    return dis(gen);
}

std::shared_ptr<CEAudioSource> C2MapRscFile::getRandomAudio(int x, int y, int z)
{
  if (this->m_random_audio_sources.empty()) {
    return std::shared_ptr<CEAudioSource>();
  }

  int rnd = rand() % (this->m_random_audio_sources.size()-1);
  std::shared_ptr<CEAudioSource> src = this->m_random_audio_sources.at(rnd);
    
    float min = 256.0f * -32.f;
    float max = 256.0f * 32.f;

    float randomX = x + getRandomFloat(min, max);
    float randomY = y + getRandomFloat(min, max);
  src->setPosition(glm::vec3(randomX, randomY, z));

  return src;
}

void C2MapRscFile::playRandomAudio(int x, int y, int z)
{
  // play random audio at this point
  int rnd = rand() % (this->m_random_audio_sources.size()-1);
  CEAudioSource* src = this->m_random_audio_sources.at(rnd).get();
  src->setPosition(glm::vec3(x, y, z));
  src->play();
}

std::shared_ptr<CEAudioSource> C2MapRscFile::getAmbientAudio(int i)
{
  return this->m_ambient_audio_sources.at(i);
}

void C2MapRscFile::playAmbientAudio(int i)
{
  CEAudioSource* src = this->m_ambient_audio_sources.at(i).get();
  src->play();
}

int C2MapRscFile::getAtlasTileWidth()
{
  return 128;
}

int C2MapRscFile::getAtlasTilePadding()
{
  return 32;
}

glm::vec4 C2MapRscFile::getFadeColor()
{
    if (m_type == CEMapType::C2) {
      return glm::vec4(m_fade_rgb[1][0], m_fade_rgb[1][1], m_fade_rgb[1][2], 1.f);
    } else {
      return glm::vec4(m_fade_rgb[0][0], m_fade_rgb[0][1], m_fade_rgb[0][2], 1.f);
    }
}

void C2MapRscFile::load(const std::string &file_name, std::filesystem::path basePath)
{
  std::ifstream infile;
  infile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

  if (!std::filesystem::exists(file_name)) {
      throw std::runtime_error("File not found: " + file_name);
  } else {
    std::cout << "Found " << file_name << "; OK. Loading..." << std::endl;
  }
  
  try {
    infile.open(file_name.c_str(), std::ios::binary | std::ios::in);

    int tile_padding = getAtlasTilePadding();
    int tile_width = getAtlasTileWidth();

    int texture_count, model_count;
    int SOURCE_SQUARE_SIZE = tile_width; // each texture in C2 is 128x128
    int TEXTURE_SQUARE_SIZE = SOURCE_SQUARE_SIZE + (tile_padding * 2); // we add padding to each side to prevent bleeding and mipmap artifacts
    
    infile.read(reinterpret_cast<char *>(&texture_count), 4);
    infile.read(reinterpret_cast<char *>(&model_count), 4);

    if (m_type == CEMapType::C2) {
      infile.read(reinterpret_cast<char *>(this->m_fade_rgb), 4*3*3);
      infile.read(reinterpret_cast<char *>(this->m_trans_rgb), 4*3*3);
    } else {
      infile.read(reinterpret_cast<char *>(this->m_fade_rgb), 4*3);
      infile.read(reinterpret_cast<char *>(this->m_trans_rgb), 4*3);
    }
    
    std::vector<uint16_t> raw_texture_data; //rgba5551
    raw_texture_data.resize((size_t)SOURCE_SQUARE_SIZE*SOURCE_SQUARE_SIZE*texture_count);
    infile.read(reinterpret_cast<char *>(raw_texture_data.data()), (size_t)SOURCE_SQUARE_SIZE*SOURCE_SQUARE_SIZE*sizeof(uint16_t)*texture_count);
    
    // Combine all the textures into a single texture for ease of opengl use.
    // Add a buffer around images to mimic clamping with GL_LINEAR; see: https://stackoverflow.com/questions/19611745/opengl-black-lines-in-between-tiles
    std::vector<uint16_t> combined_texture_data; // rgba5551
    
    int squared_texture_rows = static_cast<int>(sqrt(static_cast<float>(texture_count)) + .99f); // number of rows and columns needed to fit the data
    
    uint16_t pad_color;
    std::vector<uint16_t> tx_filler((size_t)TEXTURE_SQUARE_SIZE*squared_texture_rows);
    
    for (int line = 0; line < (squared_texture_rows*SOURCE_SQUARE_SIZE); line++) { // Total vertical rows in image file (128 * number of images; current = pointer to vertical pixel location in file that represents the location of a new row of horizontal blocks)
      
      if (line % SOURCE_SQUARE_SIZE == 0) {
        pad_color = _PadTypeColor::Red;
        tx_filler.clear();
        tx_filler.assign(((long long)TEXTURE_SQUARE_SIZE*squared_texture_rows*tile_padding), pad_color);
        
        for (int x=0; x < squared_texture_rows; x++) {
          for (int p = 0; p < tile_padding; p++)
            tx_filler.at((long long)x*TEXTURE_SQUARE_SIZE + p) = _PadTypeColor::Blue;

          for (int p = 1; p <= tile_padding; p++)
            tx_filler.at(((long long)x*TEXTURE_SQUARE_SIZE)+((long long)TEXTURE_SQUARE_SIZE-p)) = _PadTypeColor::Green;
        }
        combined_texture_data.insert(combined_texture_data.end(), tx_filler.begin(), tx_filler.end());
      }
      
      for (int col = 0; col < squared_texture_rows; col++) { // Blocks (number of images; current = current block's image)
        int lin_to_row = static_cast<int>(line/SOURCE_SQUARE_SIZE);
        int tx_c = ((lin_to_row)*squared_texture_rows)+col;
        int tex_start = line % SOURCE_SQUARE_SIZE;
        
        pad_color = _PadTypeColor::Blue;
        for (int p = 0; p < tile_padding; p++)
          combined_texture_data.insert(combined_texture_data.end(), pad_color);
        
        if (tx_c >= texture_count) {
          std::vector<uint16_t> tx_filler(SOURCE_SQUARE_SIZE);
          combined_texture_data.insert(combined_texture_data.end(), tx_filler.begin(), tx_filler.end());
        } else {
          std::vector<uint16_t>::const_iterator first = raw_texture_data.begin() + ((long long)tx_c*SOURCE_SQUARE_SIZE*SOURCE_SQUARE_SIZE) + ((long long)tex_start*SOURCE_SQUARE_SIZE); // Get start pos of a single line of pixels
          std::vector<uint16_t>::const_iterator last = raw_texture_data.begin() + ((long long)tx_c*SOURCE_SQUARE_SIZE*SOURCE_SQUARE_SIZE) + ((long long)tex_start*SOURCE_SQUARE_SIZE) + SOURCE_SQUARE_SIZE; // Get end pos, which is the start + 128 pixels
          combined_texture_data.insert(combined_texture_data.end(), first, last);
        }
        
        pad_color = _PadTypeColor::Green;
        for (int p = 0; p < tile_padding; p++)
          combined_texture_data.insert(combined_texture_data.end(), pad_color);
      }
      
      if ((line+1) % SOURCE_SQUARE_SIZE == 0) {
        pad_color = _PadTypeColor::Yellow;
        tx_filler.clear();
        tx_filler.assign((long long)TEXTURE_SQUARE_SIZE*squared_texture_rows*tile_padding, pad_color);
        
        for (int x=0; x < squared_texture_rows; x++) {
          for (int p = 0; p < tile_padding; p++)
            tx_filler.at((long long)x*TEXTURE_SQUARE_SIZE+p) = _PadTypeColor::Blue;

          for (int p = 1; p <= tile_padding; p++)
            tx_filler.at(((long long)x*TEXTURE_SQUARE_SIZE)+((long long)TEXTURE_SQUARE_SIZE-p)) = _PadTypeColor::Green;
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
    for (int i=0; i < tile_padding; i++) {
      for (int row = 0; row < (TEXTURE_SQUARE_SIZE*squared_texture_rows); row++) {
        for (int col = 0; col < (TEXTURE_SQUARE_SIZE*squared_texture_rows); col++) {
          int id_x = col;
          int id_y = row*TEXTURE_SQUARE_SIZE*squared_texture_rows;
          uint16_t color;
          
          if (i >= 1) {
            color = final_texture_data.at((long long)id_y+id_x);
          } else {
            color = combined_texture_data.at((long long)id_y+id_x);
          }
          
          switch (color) {
            case _AtlasPadType::Left:
              color = final_texture_data.at((long long)id_y+((long long)id_x-1));
              break;
            case _AtlasPadType::Right:
              if (i >= 1) {
                color = final_texture_data.at((long long)id_y+((long long)id_x+1));
              } else {
                color = combined_texture_data.at((long long)id_y+((long long)id_x+1));
              }
              break;
            case _AtlasPadType::Above:
              color = final_texture_data.at((((long long)row-1)*TEXTURE_SQUARE_SIZE*squared_texture_rows)+id_x);
              break;
            case _AtlasPadType::Below:
              if (i >= 1) {
                color = final_texture_data.at((((long long)row+1)*TEXTURE_SQUARE_SIZE*squared_texture_rows)+id_x);
              } else {
                color = combined_texture_data.at((((long long)row+1)*TEXTURE_SQUARE_SIZE*squared_texture_rows)+id_x);
              }
              break;
            default:
              break;
          }
          if (i >= 1) {
            final_texture_data.at((long long)id_y+id_x) = color;
          } else {
            final_texture_data.insert(final_texture_data.end(), color);
          }
        }
      }
    }
    
    std::unique_ptr<CETexture> cTexture = std::unique_ptr<CETexture>(new CETexture(final_texture_data, squared_texture_rows*TEXTURE_SQUARE_SIZE*squared_texture_rows*TEXTURE_SQUARE_SIZE, squared_texture_rows*TEXTURE_SQUARE_SIZE, squared_texture_rows*TEXTURE_SQUARE_SIZE));
    //cTexture->saveToBMPFile("/tmp/atlas.bmp");

    this->m_texture_atlas_width = squared_texture_rows;
    this->m_texture_count = texture_count;
    this->m_textures.push_back(std::move(cTexture));

    for (int t=0; t < m_texture_count; t++) {
      std::vector<uint16_t> texture_data;
      std::vector<uint16_t>::const_iterator first = raw_texture_data.begin() + ((long long)t * SOURCE_SQUARE_SIZE * SOURCE_SQUARE_SIZE);
      std::vector<uint16_t>::const_iterator last = first + (long long)SOURCE_SQUARE_SIZE*SOURCE_SQUARE_SIZE;
      texture_data.insert(texture_data.end(), first, last);

      std::unique_ptr<CETexture> c_texture = std::unique_ptr<CETexture>(new CETexture(texture_data, SOURCE_SQUARE_SIZE*SOURCE_SQUARE_SIZE, SOURCE_SQUARE_SIZE, SOURCE_SQUARE_SIZE));
      //std::stringstream ss;
      //ss << "/tmp/" << "map_texture_" << t << ".bmp";

      //c_texture->saveToBMPFile(ss.str());
      this->m_textures.push_back(std::move(c_texture));
    }
    
    
    // Load 3d models
    for (int m=0; m < model_count; m++) {
      std::unique_ptr<CEWorldModel> cModel = std::unique_ptr<CEWorldModel>(new CEWorldModel(m_type, infile));
      this->m_models.push_back(std::move(cModel));
    }
    
    // Load sky bitmap and map overlay (dawn, day, night)
    if (m_type == CEMapType::C2) {
      this->m_dawn_sky = std::unique_ptr<C2Sky>(new C2Sky(infile, basePath / "shaders"));
      this->m_dawn_sky->setRGBA(glm::vec4(m_fade_rgb[0][0], m_fade_rgb[0][1], m_fade_rgb[0][2], 1.f));
    }

    this->m_day_sky = std::unique_ptr<C2Sky>(new C2Sky(infile, basePath / "shaders"));
    this->m_day_sky->setRGBA(this->getFadeColor());

    if (m_type == CEMapType::C2) {
      this->m_night_sky = std::unique_ptr<C2Sky>(new C2Sky(infile, basePath / "shaders"));
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
    
    int fog_count;
    infile.read(reinterpret_cast<char *>(&fog_count), 4);
    
    m_fogs.resize(fog_count);
    infile.read(reinterpret_cast<char*>(m_fogs.data()), sizeof(FogData)*fog_count);

    // Load rnd sounds
    infile.read(reinterpret_cast<char *>(&this->m_random_sounds_count), 4);
    for (int i = 0; i < this->m_random_sounds_count; i++)
    {
      std::shared_ptr<Sound> snd(new Sound());
      std::vector<int16_t> snd_data;
      uint32_t length = 0;

      infile.read((char*)&length, sizeof(uint32_t));

      snd_data.resize(length / sizeof(uint16_t));
      infile.read(reinterpret_cast<char*>(snd_data.data()), length);

      snd->setName("RndAudio");
      snd->setWaveData(16, 1, length, 22050, snd_data);

      this->m_random_sounds.push_back(snd);

      std::unique_ptr<CEAudioSource> src(new CEAudioSource(snd));

      src->setClampDistance((16*2)); // Scaled down 16x (was 256*2)
      src->setMaxDistance((16*200)); // Scaled down 16x (was 256*200)
      src->setGain(1.f);
      src->setLooped(false);

      this->m_random_audio_sources.push_back(std::move(src));
    }

    // Load ambient sounds and their associated area data
    // In the original engine, each ambient sound has embedded area data (TAmbient structure)
    infile.read(reinterpret_cast<char *>(&this->m_ambient_sounds_count), 4);
    for (int i = 0; i < this->m_ambient_sounds_count; i++)
    {
      // Load the sound data first (matches original TSFX loading)
      std::shared_ptr<Sound> snd(new Sound());
      std::vector<int16_t> snd_data;
      uint32_t length = 0;
      infile.read((char*)&length, sizeof(uint32_t));
      
      snd_data.resize(length / sizeof(uint16_t));
      infile.read(reinterpret_cast<char*>(snd_data.data()), length);

      snd->setName("AmbientAudio");
      snd->setWaveData(16, 1, length, 22050, snd_data);

      this->m_ambient_sounds.push_back(snd);
      std::unique_ptr<CEAudioSource> src(new CEAudioSource(snd));

      src->setNoDistance(0.75f);
      src->setLooped(false);

      this->m_ambient_audio_sources.push_back(std::move(src));

      // Load area-specific random sound data immediately after each sound (matches TAmbient structure)
      AmbientArea area;
      
      // Read the random sound data array (16 entries)
      infile.read(reinterpret_cast<char*>(area.rdata), sizeof(RandomSoundData) * 16);
      
      // Read RSFXCount and AVolume
      infile.read(reinterpret_cast<char*>(&area.RSFXCount), 4);
      infile.read(reinterpret_cast<char*>(&area.AVolume), 4);
      
      // Initialize random timer like original engine
      if (area.RSFXCount > 0) {
        int baseFreq = area.rdata[0].RFreq;
        area.RndTime = (baseFreq / 2 + (rand() % baseFreq)) * 1000;
      } else {
        area.RndTime = 0;
      }
      
      this->m_ambient_areas.push_back(area);
    }

    if (m_type == CEMapType::C1) {
      this->m_waters.resize(24);
      // C1 we are done here
      infile.close();

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

void C2MapRscFile::generateTextureAtlas()
{
    // Generate and bind the framebuffer
    unsigned int fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Generate and bind the texture
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Define the texture with common, compatible parameters
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error after glTexImage2D for generateTextureAtlas: " << error << std::endl;
        // Handle the error appropriately here
        // Cleanup before returning to avoid leaking resources
        glDeleteTextures(1, &texture);
        glDeleteFramebuffers(1, &fbo);
        return;
    }

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Attach the texture to the framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    // Check framebuffer status
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Failed to complete FRAMEBUFFER" << std::endl;
        // Cleanup before returning to avoid leaking resources
        glDeleteTextures(1, &texture);
        glDeleteFramebuffers(1, &fbo);
        return;
    }

    // Clean up bindings
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Do not delete the texture here if you intend to use it later.
    // Keep the texture ID for later use in rendering.

    // Only delete the framebuffer
    glDeleteFramebuffers(1, &fbo);
}


int C2MapRscFile::getWaterCount()
{
  return (int)this->m_waters.size();
}

const CEWaterEntity& C2MapRscFile::getWater(int i) const
{
  if (i > this->m_waters.size()) {
    printf("ERROR! Attempted to access water index %i, which is invalid\n", i);

    i = 0;
  }
  return this->m_waters.at(i);
}

const FogData& C2MapRscFile::getFog(int i) const
{
  if (i > this->m_fogs.size()) {
    printf("ERROR! Attempted to access fog index %i, which is invalid\n", i);

    i = 0;
  }
  return this->m_fogs.at(i);
}

std::shared_ptr<CEAudioSource> C2MapRscFile::getRandomAudioForArea(int ambientAreaId, int x, int y, int z)
{
  if (ambientAreaId < 0 || ambientAreaId >= m_ambient_areas.size()) {
    return nullptr;
  }
  
  const AmbientArea& area = m_ambient_areas[ambientAreaId];
  if (area.RSFXCount == 0) {
    return nullptr;
  }
  
  // Pick a random sound from this area's available sounds (matches original engine logic)
  int rr = rand() % area.RSFXCount;
  int soundIndex = area.rdata[rr].RNumber;
  
  // Validate sound index
  if (soundIndex < 0 || soundIndex >= m_random_audio_sources.size()) {
    return nullptr;
  }
  
  std::shared_ptr<CEAudioSource> src = m_random_audio_sources[soundIndex];
  
  // Set random position around player (matches original engine)
  float min = -4096.0f;
  float max = 4096.0f;
  float randomX = x + getRandomFloat(min, max);
  float randomY = y + getRandomFloat(min, 256.0f);
  float randomZ = z + getRandomFloat(min, max);
  
  src->setPosition(glm::vec3(randomX, randomY, randomZ));
  
  // Set volume from area data
  float volume = area.rdata[rr].RVolume / 256.0f; // Normalize to 0-1 range
  src->setGain(volume);
  
  return src;
}

bool C2MapRscFile::shouldPlayRandomSound(int ambientAreaId, double currentTime)
{
  if (ambientAreaId < 0 || ambientAreaId >= m_ambient_areas.size()) {
    return false;
  }
  
  const AmbientArea& area = m_ambient_areas[ambientAreaId];
  return area.RSFXCount > 0 && area.RndTime <= 0;
}

void C2MapRscFile::updateRandomSoundTimer(int ambientAreaId, double timeDelta)
{
  if (ambientAreaId < 0 || ambientAreaId >= m_ambient_areas.size()) {
    return;
  }
  
  AmbientArea& area = m_ambient_areas[ambientAreaId];

  // If timer expired, reset it (matches original engine logic)
  // IMPORTANT: check for reset BEFORE reducing RndTime so render loop has a chance to
  // catch the reset.
  if (area.RndTime <= 0) {
    int baseFreq = area.rdata[0].RFreq;
    area.RndTime = (baseFreq / 2 + (rand() % baseFreq)) * 1000;
  }

  if (area.RSFXCount > 0) {
    area.RndTime -= (int)(timeDelta * 1000); // Convert to milliseconds like original engine
  }
}
