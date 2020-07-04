/*
* This object is responsible for representing a CEGeometry by combining the geometry
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
// TODO: implement keyframe animation using this approach instead of below: https://www.khronos.org/opengl/wiki/Keyframe_Animation
#include "g_shared.h"

class CEGeometry;

class Camera;
class C2CarFile;

class CEAnimatableModel {
private:
  std::shared_ptr<C2CarFile> m_car_file;

  Vector3d m_position;
  double m_scale;
  double m_alpha; // "direction I am currently pointed": TODO: which axis??! Come on
  double m_beta;
  double m_gamma;
  
  // Animatable
  std::string m_current_animation_name;
  std::string m_previous_animation_name;
  int m_frame_time;
  int m_previous_animation_frame_time;
  int m_previous_animation_to_new_morph_time;
  
  bool m_did_animation_finish;

public:
  CEAnimatableModel(const std::shared_ptr<C2CarFile>& car_file);
  ~CEAnimatableModel();
  
  /* Positioning details */
  void Update(Camera& camera);
  Vector3d getCurrentPosition();
  void setPosition(float x, float z, float y);
  void moveTo(float x, float z, bool blocked_by_water, bool blocked_by_models);
  
//  #warning Should not set alpha, beta, gamma. Instead, use rotation functions.
  float getCurrentAlpha();
  void setAlpha(double alpha);
  
  float getBeta();
  void setBeta(double beta);
  
  float getGamma();
  void setGamma(double gamma);
  
  /* sizing */
  float getScale();
  void setScale(double scale);
  
  /* Animating */
  bool didAnimationFinish();
  std::weak_ptr<CEGeometry> getCurrentModelForRender();
  void setAnimation(std::string animation_name);
  void animate(int time_delta);
  
  /* Rendering */
  void render();
  
  // Creates a complex morphed representation for between animations.
  std::weak_ptr<CEGeometry> getBetweenMorphedModel(std::string animation_previous_name, std::string animation_target_name, int at_time_previous, int at_time_target, int current_morph_time, float scale, float character_beta, float character_gamma, float character_bend);

  // Creates a single morphed representation for a static animation.
  std::weak_ptr<CEGeometry> getMorphedModel(std::string animation_name, int at_time, float scale);
};
