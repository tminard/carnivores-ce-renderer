/*
* This object is responsible for representing a C2Geometry by combining the geometry
* with animations, sounds, and position.
*
* Allows the developer to animate geometry along with sounds.
*
* Equivalent to SceneKit's SCNNode.
*/
#pragma once

#include <string>
#include <memory>
#include <map>

//#warning Bad C integration: remove hunt.h references
#include "g_shared.h"

class C2Geometry;
class C2CarFile;

class C2AnimatableModel {
private:
  std::shared_ptr<C2CarFile> m_car_file;
  
//#warning Do these belong in the C2Geometry object? (and set for Characters in the m_modified geo member)
  Vector3d m_position;
  float m_scale;
  float m_alpha; // direction I am currently pointed
  float m_beta;
  float m_gamma;
  
  // Animatable
  std::string m_current_animation_name;
  std::string m_previous_animation_name;
  int m_frame_time;
  int m_previous_animation_frame_time;
  int m_previous_animation_to_new_morph_time;
  
  bool m_did_animation_finish;

public:
  C2AnimatableModel(const std::shared_ptr<C2CarFile>& car_file);
  ~C2AnimatableModel();
  
  /* Positioning details */
  Vector3d getCurrentPosition();
  void setPosition(float x, float z, float y);
  void moveTo(float x, float z, bool blocked_by_water, bool blocked_by_models);
  
//  #warning Should not set alpha, beta, gamma. Instead, use rotation functions.
  float getCurrentAlpha();
  void setAlpha(float alpha);
  
  float getBeta();
  void setBeta(float beta);
  
  float getGamma();
  void setGamma(float gamma);
  
  /* sizing */
  float getScale();
  void setScale(float scale);
  
  /* Animating */
  bool didAnimationFinish();
  C2Geometry* getCurrentModelForRender();
  void setAnimation(std::string animation_name);
  void animate(int time_delta);
  
  // Creates a complex morphed representation for between animations.
  C2Geometry* getBetweenMorphedModel(std::string animation_previous_name, std::string animation_target_name, int at_time_previous, int at_time_target, int current_morph_time, float scale, float character_beta, float character_gamma, float character_bend);

  // Creates a single morphed representation for a static animation.
  C2Geometry* getMorphedModel(std::string animation_name, int at_time, float scale);
};
