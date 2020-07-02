#include "CEAnimatableModel.h"
#include "C2CarFile.h"

#include "CEGeometry.h"
#include "CEAnimation.h"
#include "transform.h"

#include <cmath>
#include <iostream>

// C Forward declares
//#warning Deprecation: please remove these old references and bake code into ultilities class
int CheckPlaceCollision2(Vector3d &v, int wc);

CEAnimatableModel::CEAnimatableModel(const std::shared_ptr<C2CarFile>& car_file)
  : m_car_file(car_file)
{
  this->m_frame_time = 0;

  this->m_position.x = 0.f;
  this->m_position.z = 0.f;
  this->m_position.y = 0.f;

  this->m_current_animation_name = "";
  this->m_previous_animation_name = "";
}

CEAnimatableModel::~CEAnimatableModel()
{
}

//#warning Using the old code for movement. Need to fix collision detection to use world resource
// This should be done in Character, which simply setsPos after transition. An animatableModel doesnt care
// about collisions.
void CEAnimatableModel::moveTo(float x, float z, bool blocked_by_water, bool blocked_by_models)
{
  Vector3d p = this->m_position;

  if (CheckPlaceCollision2(p, (int)blocked_by_water)) {
    this->m_position.x += x / 2;
    this->m_position.z += z / 2;
    return;
  }

  p.x+=x;
  p.z+=z;

  if (!CheckPlaceCollision2(p, (int)blocked_by_water)) {
    this->m_position = p;
    return;
  }

  p = this->m_position;
  p.x += x / 2;
  p.z += z / 2;
  if (!CheckPlaceCollision2(p, (int)blocked_by_water)) this->m_position = p;
  p = this->m_position;

  p.x+=x/4;
  p.z+=z/4;
  this->m_position = p;
}

void CEAnimatableModel::setScale(float scale)
{
  this->m_scale = scale;
}

float CEAnimatableModel::getScale()
{
  return this->m_scale;
}

void CEAnimatableModel::setPosition(float x, float z, float y)
{
  this->m_position.x = x;
  this->m_position.z = z;
  this->m_position.y = y;
}

Vector3d CEAnimatableModel::getCurrentPosition()
{
  return this->m_position;
}

void CEAnimatableModel::setAlpha(float alpha)
{
  this->m_alpha = alpha;
}

float CEAnimatableModel::getCurrentAlpha()
{
  return this->m_alpha;
}

void CEAnimatableModel::setBeta(float beta)
{
  this->m_beta = beta;
}

float CEAnimatableModel::getBeta()
{
  return this->m_beta;
}

void CEAnimatableModel::setGamma(float gamma)
{
  this->m_gamma = gamma;
}

float CEAnimatableModel::getGamma()
{
  return this->m_gamma;
}

/*
 * Keep playing the current animation + sound, and process loop behavior.
 * TODO: Actually process loop behavior. Currently the AI controls this, and it shouldn't (?).
 */
void CEAnimatableModel::animate(const int dtime)
{
  if (this->m_current_animation_name.empty()) {
    return;
  }
  this->m_frame_time += dtime;
  std::weak_ptr<CEAnimation> animation = this->m_car_file->getAnimationByName(this->m_current_animation_name);

  if (this->m_frame_time >= (animation.lock())->m_total_time)
  {
    this->m_frame_time %= (animation.lock())->m_total_time;
    this->m_did_animation_finish = true;
//#warning Incomplete integration: notify something that the animation did finish, and play the audio again
  }
}

bool CEAnimatableModel::didAnimationFinish()
{
  return this->m_did_animation_finish;
}

void CEAnimatableModel::setAnimation(std::string animation_name)
{
  this->m_did_animation_finish = false;

  this->m_previous_animation_name = this->m_current_animation_name;
  this->m_previous_animation_frame_time = this->m_frame_time;
  this->m_previous_animation_to_new_morph_time = 0.f;

  this->m_frame_time = 0;
  this->m_current_animation_name = animation_name;
//#warning Play sound
  //ActivateCharacterFx(cptr);
}

void CEAnimatableModel::Update(Camera& camera)
{
    // TODO: convert old data types to glm
  glm::vec3 pos(this->getCurrentPosition().x, this->getCurrentPosition().z, this->getCurrentPosition().y);
  Transform t(pos, glm::vec3(0, 0, 0), glm::vec3(2, 2, 2));
  std::weak_ptr<CEGeometry> geo = this->getCurrentModelForRender();

  geo.lock()->Update(t, camera);
}

/*
 *
 */
void CEAnimatableModel::render()
{
  // update matrix
  // update textures and shader if needed (pass in current shader so we know?)
  // call draw
  std::weak_ptr<CEGeometry> geo = this->getCurrentModelForRender();
  geo.lock()->Draw();
}

std::weak_ptr<CEGeometry> CEAnimatableModel::getCurrentModelForRender()
{
  std::weak_ptr<CEGeometry> m = this->m_car_file->getGeometry();
  if (this->m_current_animation_name == "") {
    return m;
  }

  if (this->m_previous_animation_name != this->m_current_animation_name && this->m_previous_animation_name != "") {
//#warning gamma, beta, and bend not integrated
    return this->getBetweenMorphedModel(this->m_previous_animation_name, this->m_current_animation_name, this->m_previous_animation_frame_time, this->m_frame_time, this->m_previous_animation_to_new_morph_time, this->m_scale, 0, 0, 0);
  } else {
    return this->getMorphedModel(this->m_current_animation_name, this->m_frame_time, this->m_scale);
  }
}

std::weak_ptr<CEGeometry> CEAnimatableModel::getMorphedModel(std::string animation_name, int at_time, float scale)
{
//  #warning TODO: This modifies the original shared model. Fix this. Also check for nulls
  std::weak_ptr<CEGeometry> sharedGeo = this->m_car_file->getGeometry();
  //CEAnimation* animation = this->m_car_file->getAnimationByName(animation_name);

  /*int currentFrame = ((animation->m_number_of_frames-1) * at_time * 256) / animation->m_total_time;
  int splineDelta = currentFrame & 0xFF;
  currentFrame = currentFrame >> 8;

  float k2 = (float)(splineDelta) / 256.f;
  float k1 = 1.0f - k2;
  k1 = k1/8.f;
  k2 = k2/8.f;

  std::vector<TPoint3d> m_vertices = sharedGeo->getVertices();
  int vcount = (int)m_vertices.size();
  int aniOffset = currentFrame*vcount*3;

  for (int v=0; v<vcount; v++) {
	  m_vertices[v].x = (float)animation->m_animation_data[(aniOffset)+(v * 3 + 0)] * k1 + (float)animation->m_animation_data[(aniOffset) + ((v + vcount) * 3 + 0)] * k2;
	  m_vertices[v].y = (float)animation->m_animation_data[(aniOffset)+(v * 3 + 1)] * k1 + (float)animation->m_animation_data[(aniOffset) + ((v + vcount) * 3 + 1)] * k2;
	  m_vertices[v].z = -((float)animation->m_animation_data[(aniOffset)+(v * 3 + 2)] * k1 + (float)animation->m_animation_data[(aniOffset) + ((v + vcount) * 3 + 2)] * k2);
  }

  sharedGeo->setVertices(m_vertices);
  sharedGeo->syncOldVerticeData(); //support the old way. TODO: Eventually remove them when renderer can be refactord*/
  return sharedGeo;
}

std::weak_ptr<CEGeometry> CEAnimatableModel::getBetweenMorphedModel(std::string animation_previous_name, std::string animation_target_name, int at_time_previous, int at_time_target, int current_morph_time, float scale, float character_beta, float character_gamma, float character_bend)
{

  std::weak_ptr<CEGeometry> sharedGeo = this->m_car_file->getGeometry();
/*
  CEAnimation* previous_animation = m_car_file->getAnimationByName(animation_previous_name);
  CEAnimation* target_animation = m_car_file->getAnimationByName(animation_target_name);

  int previousFrame = 0, previousSplineDelta = 0;

  int currentFrame = ((target_animation->m_number_of_frames-1) * at_time_target * 256) / target_animation->m_total_time;
  int splineDelta = currentFrame & 0xFF;
  currentFrame = currentFrame >> 8;

  // Should I even try to morph?
  const int MAXMORPHTIME = 256;
  bool ShouldMorphPrevious = (animation_previous_name != animation_target_name) && (current_morph_time < MAXMORPHTIME);

  if (ShouldMorphPrevious) {
    previousFrame = ((previous_animation->m_number_of_frames-1) * at_time_previous * 256) / previous_animation->m_total_time;
    previousSplineDelta = previousFrame & 0xFF;
    previousFrame = previousFrame >> 8;
  }

  float k2 = (float)(splineDelta) / 256.f;
  float k1 = 1.0f - k2;
  k1 /= 8.f;
  k2 /= 8.f;

  float previous_k1 = 0.f, previous_k2 = 0.f, pmk1 = 0.f, pmk2 = 0.f;

  if (ShouldMorphPrevious) {
    previous_k2 = (float)(previousSplineDelta) / 256.f;
    previous_k1 = 1.0f - previous_k2;

    previous_k1 /= 8.f;
    previous_k2 /= 8.f;
    pmk1 = (float)current_morph_time / MAXMORPHTIME;
    pmk2 = 1.f - pmk1;
  }

  std::vector<TPoint3d> m_vertices = sharedGeo->getVertices();
  int vcount = m_vertices.size();
  float sb = (float)std::sin(character_beta) * scale;
  float cb = (float)std::cos(character_beta) * scale;
  float sg = (float)std::sin(character_gamma);
  float cg = (float)std::cos(character_gamma);

  float x, xx, y, yy, z, zz, px, py, pz;

  for (int v=0; v<vcount; v++) {
    x = target_animation->m_animation_data[v*3+0] * k1 + target_animation->m_animation_data[(v+vcount)*3+0] * k2;
    y = target_animation->m_animation_data[v*3+1] * k1 + target_animation->m_animation_data[(v+vcount)*3+1] * k2;
    z = -(target_animation->m_animation_data[v*3+2] * k1 + target_animation->m_animation_data[(v+vcount)*3+2] * k2);

    if (ShouldMorphPrevious) {
      px = previous_animation->m_animation_data[v*3+0] * previous_k1 + previous_animation->m_animation_data[(v+vcount)*3+0] * previous_k2;
      py = previous_animation->m_animation_data[v*3+1] * previous_k1 + previous_animation->m_animation_data[(v+vcount)*3+1] * previous_k2;
      pz = -(previous_animation->m_animation_data[v*3+2] * previous_k1 + previous_animation->m_animation_data[(v+vcount)*3+2] * previous_k2);

      x = x*pmk1 + px * pmk2;
      y = y*pmk1 + py * pmk2;
      z = z*pmk1 + pz * pmk2;
    }

    // Calculate 'FI' (used to determine whether or not to slow down the animation morph)

    zz = z;
    xx = cg * x - sg * y;
    yy = cg * y + sg * x;

    float fi = 0.f;
    if (z > 0) {
      fi = z / 240.f;
      if (fi > 1.f) fi = 1.f;
    } else {
      fi = z / 380.f;
      if (fi < -1.f) fi = - 1.f;
    }

    // FI value will be in range -1.0...+1.0
 */
    /*
     STANDARD:
     Y represents forward/back.
     Z is height.
     X is shift right/left

     beta is front-back rotation off X-0
     gamma is side-side rotation off Y-0
     alpha is facing direction off Z-0

     NOTE: In carnivores, Y axis acts as the Z axis and vice versa (per RexHunter99, 30-July 2008):

     C2:
     Z represents forward/back.
     Y is height.
     X is shift right/left

     beta is front-back rotation off X-0
     gamma is side-side rotation off Z-0
     alpha is facing direction off Y-0
     */

    // character bend represents how sharp of an angle the character is currently turning towards
    // his target alpha (where PI speed means complete 180). Taking this into account here
    // allows us to speed up/slow down the animation process based on bend (the animation effectively slows down when character makes a sharp turn).
    /*fi *= character_bend;

    // bend will be max of 0.628
    float bendc = (float)std::cos(fi); // right/left bend
    float bends = (float)std::sin(fi); // forward/back bend
    float bx = bendc * xx - bends * zz;
    float bz = bendc * zz + bends * xx;
    zz = bz;
    xx = bx;

    // finally done...
	m_vertices[v].x = xx * scale;
	m_vertices[v].y = cb * yy - sb * zz;
	m_vertices[v].z = cb * zz + sb * yy;
  }

  sharedGeo->setVertices(m_vertices);
  sharedGeo->syncOldVerticeData();*/
  return sharedGeo;
}
