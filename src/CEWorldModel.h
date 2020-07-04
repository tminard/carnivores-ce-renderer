/*
 * This class SHOULD derive from AnimatableModel
 * and stores state information for world objects
 * like trees, grass, and buildings.
 *
 * Roughly analogous to a SCNNode. Has "geometry" and potentially material attached to it.
 *
 * A world model is more advanced than a car file as it carries distant bitmap,
 * bounding box, and a special form of animations and flags.
 *
 * Note that no stock C2 world objects actually have animation data.
 *
 * TODO: Move common code into AnimatableModel, and allow this to derive from it
 */

#pragma once

#include <stdio.h>
#include <memory>
#include <array>
#include <vector>
#include <string>

#include <iostream>
#include <fstream>

#include <glm/glm.hpp>

#include "g_shared.h"

class CETexture;
class CEAnimation;
class CEGeometry;
class CESimpleGeometry;
class Vertex;

struct Transform;
struct Camera;

#define objectPLACEWATER       1
#define objectPLACEGROUND      2
#define objectPLACEUSER        4
#define objectCIRCLE           8
#define objectBOUND            16
#define objectNOBMP            32
#define objectNOLIGHT          64
#define objectDEFLIGHT         128
#define objectGRNDLIGHT        256
#define objectNOSOFT           512
#define objectNOSOFT2          1024
#define objectANIMATED         0x80000000

class CEWorldModel
{
private:
  std::unique_ptr<CEAnimation> m_animation; // apparently, world models only support 1 animation. TODO: allow unbounded animations
  std::unique_ptr<CEGeometry> m_geometry;
  
  std::string m_model_name;

  std::array<TBound, 8> m_bounding_box;

  std::array<Vector3d, 6> m_far_vertices;
  std::unique_ptr<CESimpleGeometry> m_far_geometry;

  std::vector<glm::mat4> m_far_instances;
  std::vector<glm::mat4> m_near_instances;
  
  std::unique_ptr<TObjInfo> m_old_object_info; // Easier to use old object for now
  
  void _generateBoundingBox(std::vector<Vertex>& vertex_data); // old method for bounding box. Move to geo when able
  
public:
  CEWorldModel(const CEMapType type, std::ifstream& instream);
  ~CEWorldModel();

  void addFar(Transform& transform);
  void updateFarInstances();
  void renderFarInstances();

  void addNear(Transform& transform);
  void updateNearInstances();
  void renderNearInstances();
  
  void render(Transform& transform, Camera& camera);
  void renderFar(Transform& transform, Camera& camera);
  
  CEGeometry* getGeometry();
  CESimpleGeometry* getFarGeometry();
  TObjInfo* getObjectInfo(); // TODO: Make this unnecessary
};
