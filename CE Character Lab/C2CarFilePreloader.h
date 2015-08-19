//
//  C2CarFilePreloader.h
//  CE Character Lab
//
//  Created by Tyler Minard on 8/7/15.
//  Copyright (c) 2015 Tyler Minard. All rights reserved.
//

#ifndef __CE_Character_Lab__C2CarFilePreloader__
#define __CE_Character_Lab__C2CarFilePreloader__

#include <memory>
#include <map>
#include <string>

class C2CarFile;

class C2CarFilePreloader
{
  std::map<std::string, std::shared_ptr<C2CarFile>> _files;
  
public:
	const std::shared_ptr<C2CarFile>& fetch(std::string file_name);
};

#endif /* defined(__CE_Character_Lab__C2CarFilePreloader__) */
