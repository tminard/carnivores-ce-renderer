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

const std::shared_ptr<C2CarFile>& C2CarFilePreloader::fetch(std::string file_name) {
  std::map<std::string, std::shared_ptr<C2CarFile>>::iterator it;
  
  it = _files.find(file_name);

  if (it != _files.end()) {
    return it->second;
  } else {
    _files[file_name] = std::shared_ptr<C2CarFile>(new C2CarFile(file_name));

    return _files[file_name];
  }
}