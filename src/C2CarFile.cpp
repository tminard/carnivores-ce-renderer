  //
  //  C2CarFile.cpp
  //  CE Character Lab
  //
  //  Created by Tyler Minard on 8/6/15.
  //  Copyright (c) 2015 Tyler Minard. All rights reserved.
  //

#include "C2CarFile.h"

#include "CEGeometry.h"
#include "CEAnimation.h"
#include "CETexture.h"

#include <iostream>
#include <fstream>

#include "g_shared.h"

#include "IndexedMeshLoader.h"

void Console_PrintLogString(std::string log_msg);

C2CarFile::C2CarFile(std::string file_name)
{
  Console_PrintLogString("Loading file " + file_name);
  this->load_file(file_name);
}

C2CarFile::~C2CarFile()
{
}

std::shared_ptr<CEGeometry> C2CarFile::getGeometry()
{
  return this->m_geometry;
}

std::weak_ptr<CEAnimation> C2CarFile::getAnimationByName(std::string animation_name)
{
  return this->m_animations[animation_name];
}

void C2CarFile::load_file(std::string file_name)
{
  std::ifstream infile;
  TCharacterInfo c_char_info;
  std::unique_ptr<CETexture> m_texture;

  int _vcount, _fcount, _texture_size, _texture_height;
  std::vector<TPoint3d> _vertices;
  std::vector<TFace> _faces;
  std::vector<uint16_t> _texture_data;

  infile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

  try {
    infile.open(file_name.c_str(), std::ios::binary | std::ios::in);

    infile.read(c_char_info.ModelName, 32);

    infile.read(reinterpret_cast<char *>(&c_char_info.AniCount), 4);
    infile.read(reinterpret_cast<char *>(&c_char_info.SfxCount), 4);

    infile.read(reinterpret_cast<char *>(&_vcount), 4);
    infile.read(reinterpret_cast<char *>(&_fcount), 4);
    infile.read(reinterpret_cast<char *>(&_texture_size), 4);

    _faces.resize(_fcount);
    _vertices.resize(_vcount);
    infile.read(reinterpret_cast<char *>(_faces.data()), _fcount<<6);
    infile.read(reinterpret_cast<char *>(_vertices.data()), _vcount<<4);

    int tsize = _texture_size;
    _texture_height = tsize>>9;

    _texture_size = _texture_height*512;

    _texture_data.resize(tsize);
    infile.read(reinterpret_cast<char *>(_texture_data.data()), tsize);
    m_texture = std::unique_ptr<CETexture>(new CETexture(_texture_data, 256*256, 256, 256));

    for (int ani = 0; ani < c_char_info.AniCount; ani++) {
      int _ani_kps, _frames_count;
      float animation_length;
      std::vector<short int> _aniData;
      char _aniName[32];

      infile.read(_aniName, 32);
      infile.read(reinterpret_cast<char *>(&_ani_kps), 4);
      infile.read(reinterpret_cast<char *>(&_frames_count), 4);

      animation_length = (float)(_frames_count * 1000) / (float)_ani_kps;
      _aniData.resize(_frames_count*_vcount*6);
      infile.read(reinterpret_cast<char *>(_aniData.data()), _vcount*_frames_count*6);

      std::string animation_name(_aniName);
      std::shared_ptr<CEAnimation> chAni( new CEAnimation(animation_name, _ani_kps, _frames_count, (int)animation_length));
      chAni->setAnimationData(_aniData);

      this->m_animations.insert(std::make_pair(animation_name, std::move(chAni)));
    }

      // TODO: Sounds and Animations

      // brighten texture
    infile.close();
  }
  catch (std::ifstream::failure e) {
    Console_PrintLogString("Failed to load " + file_name + ": " + strerror(errno));
    std::cerr << "Exception opening/reading/closing file\n";
  }

  Console_PrintLogString("Loaded " + file_name);

    // data correction
  for (int v=0; v < _vcount; v++) {
    _vertices[v].z *= -1.0f; // Original models need to be inverted across z axis
  }

    // load instance
  std::unique_ptr<IndexedMeshLoader> m_loader(new IndexedMeshLoader(_vertices, _faces));
  this->m_geometry = std::unique_ptr<CEGeometry>(new CEGeometry(m_loader->getVertices(), m_loader->getIndices(), std::move(m_texture)));
}
