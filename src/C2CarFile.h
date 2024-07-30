//
//  C2CarFile.h
//  CE Character Lab
//
//  Created by Tyler Minard on 8/6/15.
//  Copyright (c) 2015 Tyler Minard. All rights reserved.
//

/*
 * This class is responsible for loading a C2/Ice Age car
 * file.
 *
 * Stores primitive types with the contents of the file.
 */

#ifndef CE_Character_Lab_C2CarFile_h
#define CE_Character_Lab_C2CarFile_h

#include <memory>
#include <string>
#include <cstdint>
#include <map>
#include <vector>

#include "g_shared.h"

#include "dependency/libAF/af2-sound.h"

#include <glm/glm.hpp>

using libAF2::Sound;

class CEGeometry;
class CEAnimation;
class CEAudioSource;
class Vertex;

class C2CarFile {
    
public:
    C2CarFile(std::string file_name);
    ~C2CarFile();
    void load_file(std::string file_name);
    
    std::shared_ptr<CEGeometry> getGeometry();
    std::weak_ptr<CEAnimation> getAnimationByName(std::string animation_name);
    std::weak_ptr<CEAnimation> getFirstAnimation();
  
    void playAudio(int i, glm::vec3 pos);
    
private:
    std::map<std::string, std::shared_ptr<CEAnimation> > m_animations;
    std::shared_ptr<CEGeometry> m_geometry;
    std::vector<std::shared_ptr<Sound>> m_animation_sounds;
    std::vector<std::shared_ptr<CEAudioSource>> m_animation_audio_sources;
};

#endif
