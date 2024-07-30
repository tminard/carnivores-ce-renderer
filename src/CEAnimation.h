//
//  CEAnimation.h
//  CE Character Lab
//
//  Created by Tyler Minard on 8/6/15.
//  Copyright (c) 2015 Tyler Minard. All rights reserved.
//

#ifndef __CE_Character_Lab__CEAnimation__
#define __CE_Character_Lab__CEAnimation__

#include <memory>
#include <string>
#include <vector>
#include <cstdint>
#include <glad/glad.h>
#include "g_shared.h"

class CEAnimation {
private:

public:
	std::string m_name;
	int m_kps, m_number_of_frames;
	int m_total_time;
	std::vector<short int> m_animation_data;
  std::vector<TFace> m_faces;
  std::vector<TPoint3d> m_original_vertices;

  CEAnimation(const std::string& name, int kps, int total_frames, int total_time_ms); // pass name by const ref, because VS has copy assignment bug
  ~CEAnimation();

  void setAnimationData(std::vector<short int> raw_animation_data, int vcount, std::vector<TFace> faces, std::vector<TPoint3d> original_vertices);
  std::shared_ptr<const std::vector<short int>> GetAnimationData() const;
  std::shared_ptr<const std::vector<TFace>> GetFaces() const;
  std::shared_ptr<const std::vector<TPoint3d>> GetOriginalVertices() const;
};

#endif /* defined(__CE_Character_Lab__CEAnimation__) */
