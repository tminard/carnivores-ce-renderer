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

#include "dependency/libAF/af2-sound.h"
#include "CEAudioSource.hpp"

C2CarFile::C2CarFile(std::string file_name)
{
  std::cout << "Loading " << file_name << std::endl;
  this->load_file(file_name);
}

C2CarFile::~C2CarFile()
{
}

std::shared_ptr<CEGeometry> C2CarFile::getGeometry()
{
  return this->m_geometry;
}

std::weak_ptr<CEAnimation> C2CarFile::getFirstAnimation()
{
  if (!m_animations.empty()) {
      std::string firstKey = m_animations.begin()->first;
    return getAnimationByName(firstKey);
  } else {
    return std::weak_ptr<CEAnimation>();
  }
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
    infile.read(reinterpret_cast<char *>(_faces.data()), ((long long)_fcount) << 6);
    infile.read(reinterpret_cast<char *>(_vertices.data()), ((long long)_vcount) << 4);

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
      _aniData.resize((size_t)_frames_count*_vcount*6);
      infile.read(reinterpret_cast<char *>(_aniData.data()), (size_t)_vcount*_frames_count*6);

      std::string animation_name(_aniName);
      std::shared_ptr<CEAnimation> chAni( new CEAnimation(animation_name, _ani_kps, _frames_count, (int)animation_length));
      chAni->setAnimationData(_aniData, _vcount, _faces, _vertices);

      std::cout << "Loaded animation: " << animation_name << std::endl;
      this->m_animations.insert(std::make_pair(animation_name, std::move(chAni)));
    }

    // TODO: we should share sound resources vs loading them over and over...
    // Really we only need top copy geometry, so possible keep the same CAR file and only create new instances of geo
    // Load rnd sounds
    for (int i = 0; i < c_char_info.SfxCount; i++)
    {
      std::shared_ptr<Sound> snd(new Sound());
      std::vector<int16_t> snd_data;
      
      char _sndName[32];

      infile.read(_sndName, 32);
      std::string sndName(_sndName);
      
      uint32_t length = 0;
      infile.read((char*)&length, sizeof(uint32_t));
      snd_data.resize(length / sizeof(uint16_t));
      infile.read(reinterpret_cast<char*>(snd_data.data()), length);

      snd->setName(sndName);
      snd->setWaveData(16, 1, length, 22050, snd_data);

      this->m_animation_sounds.push_back(snd);

      std::unique_ptr<CEAudioSource> src(new CEAudioSource(snd));

      // TODO: use tile length
      //src->setClampDistance((256*2));
      //src->setMaxDistance((256*200));
      src->setLooped(true);
      
      std::cout << "Loaded audio " << sndName << std::endl;

      this->m_animation_audio_sources.push_back(std::move(src));
    }

    // Original game brightened the texture here too but we wont do that
    infile.close();
  }
  catch (std::ifstream::failure e) {
    std::cerr << "Failed to load " + file_name + ": " + strerror(errno) << e.what() << std::endl;

    throw e;
  }

  std::cout << "Loaded OK " << file_name << std::endl;

    // data correction
  for (int v=0; v < _vcount; v++) {
    _vertices[v].z *= -2.0f; // Original models need to be inverted across z axis
    _vertices[v].y *= 2.0f;
    _vertices[v].x *= 2.0f;
  }

    // load instance
  std::unique_ptr<IndexedMeshLoader> m_loader(new IndexedMeshLoader(_vertices, _faces));
  
  // Note we transfer ownership of the texture
  this->m_geometry = std::unique_ptr<CEGeometry>(new CEGeometry(m_loader->getVertices(), m_loader->getIndices(), std::move(m_texture), "dinosaur"));
}

void C2CarFile::playAudio(int i, glm::vec3 pos)
{
  CEAudioSource* src = this->m_animation_audio_sources.at(i).get();
  src->setPosition(pos);
  src->play();
}
