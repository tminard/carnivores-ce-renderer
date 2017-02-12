//
//  CEAnimation.cpp
//  CE Character Lab
//
//  Created by Tyler Minard on 8/6/15.
//  Copyright (c) 2015 Tyler Minard. All rights reserved.
//

#include "CEAnimation.h"

CEAnimation::CEAnimation(const std::string& ani_name, int kps, int total_frames, int total_time_ms) :
m_name(ani_name), m_kps(kps), m_number_of_frames(total_frames), m_total_time(total_time_ms)
{
}

CEAnimation::~CEAnimation()
{
  
}

void CEAnimation::setAnimationData(std::vector<short int> raw_animation_data)
{
	this->m_animation_data = raw_animation_data;
}
