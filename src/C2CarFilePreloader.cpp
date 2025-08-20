//
//  C2CarFilePreloader.cpp
//  CE Character Lab
//
//  Created by Tyler Minard on 8/7/15.
//  Copyright (c) 2015 Tyler Minard. All rights reserved.
//

#include "C2CarFilePreloader.h"

#include "C2CarFile.h"

#include <iostream>

const std::shared_ptr<C2CarFile>& C2CarFilePreloader::fetch(std::filesystem::path file_name, bool pixelPerfectTextures) {
  bool enable = false;
  std::map<std::string, std::shared_ptr<C2CarFile>>::iterator it;
  
  // Create different cache keys based on texture filtering to avoid conflicts
  std::string cache_key = file_name.string() + (pixelPerfectTextures ? "_pixelperfect" : "_linear");
  
  it = _files.find(cache_key);

  if (enable && it != _files.end()) {
    return it->second;
  } else {
    _files[cache_key] = std::shared_ptr<C2CarFile>(new C2CarFile(file_name.string(), pixelPerfectTextures));

    return _files[cache_key];
  }
}
