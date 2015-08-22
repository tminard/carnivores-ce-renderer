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

#include "C2Texture.h"
#include "C2WorldModel.h"
#include "C2Sky.h"

template <typename T, typename... Args>
auto make_unique(Args&&... args) -> std::unique_ptr<T>
{
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
void Console_PrintLogString(std::string log_msg);

C2MapRscFile::C2MapRscFile(const std::string& file_name)
{
  this->load(file_name);
}

C2MapRscFile::~C2MapRscFile()
{
  
}

C2WorldModel* C2MapRscFile::getWorldModel(int i)
{
  return this->m_models.at(i).get();
}

void C2MapRscFile::load(const std::string &file_name)
{
  std::ifstream infile;
  infile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  
  try {
    infile.open(file_name.c_str(), std::ios::binary | std::ios::in);
    
    // load file
    int texture_count, model_count;
  
    infile.read(reinterpret_cast<char *>(&texture_count), 4);
    infile.read(reinterpret_cast<char *>(&model_count), 4);
    
    infile.read(reinterpret_cast<char *>(this->m_fade_rgb), 4*3*3);
    infile.read(reinterpret_cast<char *>(this->m_trans_rgb), 4*3*3);
    
    for (int tx = 0; tx < texture_count; tx++) {
      std::vector<uint16_t> raw_texture_data; //rgba5551
      raw_texture_data.resize(128*128);
      infile.read(reinterpret_cast<char *>(raw_texture_data.data()), 128*128*sizeof(uint16_t));
      
      std::unique_ptr<C2Texture> cTexture = std::unique_ptr<C2Texture>(new C2Texture(raw_texture_data, 128*128, 128, 128));
      this->m_textures.push_back(std::move(cTexture));
    }
    
    // Load 3d models
    for (int m=0; m < model_count; m++) {
      std::unique_ptr<C2WorldModel> cModel = std::unique_ptr<C2WorldModel>(new C2WorldModel(infile));
      this->m_models.push_back(std::move(cModel));
    }

    // Load sky bitmap and map overlay (dawn, day, night)
    this->m_dawn_sky = std::unique_ptr<C2Sky>(new C2Sky(infile));
    this->m_day_sky = std::unique_ptr<C2Sky>(new C2Sky(infile));
    this->m_night_sky = std::unique_ptr<C2Sky>(new C2Sky(infile));
    
    this->m_shadow_map.resize(128*128);
    infile.read(reinterpret_cast<char *>(this->m_shadow_map.data()), 128*128);

    // Load fog data
    // Load sounds
    // load water data
    
    infile.close();
  } catch (std::ifstream::failure e) {
    Console_PrintLogString("Failed to load " + file_name + ": " + strerror(errno));
    std::cerr << "Exception opening/reading/closing file\n";
  }
}