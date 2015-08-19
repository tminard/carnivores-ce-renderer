//
//  C2Animation.h
//  CE Character Lab
//
//  Created by Tyler Minard on 8/6/15.
//  Copyright (c) 2015 Tyler Minard. All rights reserved.
//

#ifndef __CE_Character_Lab__C2Animation__
#define __CE_Character_Lab__C2Animation__

#include <memory>
#include <string>
#include <vector>
#include <cstdint>

class C2Animation {
private:
public:
	std::string m_name; //char[32];
	int m_kps, m_number_of_frames;
	int m_total_time;
	std::vector<short int> m_animation_data;

  C2Animation(const std::string& name, int kps, int total_frames, int total_time_ms); // pass name by const ref, because VS has copy assignment bug
  ~C2Animation();

  void setAnimationData(std::vector<short int> raw_animation_data);
};

#endif /* defined(__CE_Character_Lab__C2Animation__) */
