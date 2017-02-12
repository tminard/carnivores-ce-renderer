//
//  C2Sky.cpp
//  CE Character Lab
//
//  Created by Tyler Minard on 8/18/15.
//  Copyright (c) 2015 Tyler Minard. All rights reserved.
//

#include "C2Sky.h"

#include "CETexture.h"

C2Sky::C2Sky(std::ifstream& instream)
{
  std::vector<uint16_t> raw_sky_texture_data;
  raw_sky_texture_data.resize(256*256);
  instream.read(reinterpret_cast<char *>(raw_sky_texture_data.data()), 256*256*sizeof(uint16_t));
  
  this->m_texture = std::unique_ptr<CETexture>(new CETexture(raw_sky_texture_data, 256*256, 256, 256));
}

C2Sky::C2Sky(std::unique_ptr<CETexture> sky_texture)
: m_texture(std::move(sky_texture))
{
  
}

C2Sky::~C2Sky()
{
  
}

void C2Sky::saveTextureAsBMP(const std::string &file_name)
{
  this->m_texture->saveToBMPFile(file_name);
}
