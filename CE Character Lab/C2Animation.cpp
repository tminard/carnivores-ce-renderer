//
//  C2Animation.cpp
//  CE Character Lab
//
//  Created by Tyler Minard on 8/6/15.
//  Copyright (c) 2015 Tyler Minard. All rights reserved.
//

#include "C2Animation.h"

C2Animation::C2Animation(const std::string& ani_name, int kps, int total_frames, int total_time_ms) :
m_name(ani_name), m_kps(kps), m_number_of_frames(total_frames), m_total_time(total_time_ms)
{
}

C2Animation::~C2Animation()
{
  
}

void C2Animation::setAnimationData(std::vector<short int> raw_animation_data)
{
	this->m_animation_data = raw_animation_data;
}
