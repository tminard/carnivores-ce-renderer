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
#include <glm/gtc/matrix_transform.hpp>

#include "g_shared.h"

class CETexture;
class CEAnimation;
class CEGeometry;
class CESimpleGeometry;
class Vertex;

struct Transform;
struct Camera;

struct SquareBoundingBox {
    glm::vec3 corners[4];
};

const int objectPLACEWATER = 0x01;
const int objectPLACEGROUND = 0x02;
const int objectPLACEUSER = 0x04;
const int objectCIRCLE = 0x08;
const int objectBOUND = 0x10;
const int objectNOBMP = 0x20;
const int objectNOLIGHT = 0x40;
const int objectDEFLIGHT = 0x80;
const int objectGRNDLIGHT = 0x100;
const int objectNOSOFT = 0x200;
const int objectNOSOFT2 = 0x400;
const int objectANIMATED = 0x80000000;

class CEWorldModel
{
private:
  std::unique_ptr<CEAnimation> m_animation; // apparently, world models only support 1 animation. TODO: allow unbounded animations
  std::unique_ptr<CEGeometry> m_geometry;
  
  std::string m_model_name;

  std::array<TBound, 8> m_bounding_box;
  std::vector<SquareBoundingBox> m_bounding_rec;

  std::array<Vector3d, 6> m_far_vertices;
  std::unique_ptr<CESimpleGeometry> m_far_geometry;

  std::vector<glm::mat4> m_far_instances;
  std::vector<glm::mat4> m_near_instances;
  
  std::vector<Transform> m_transforms;
  
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
  
  CEGeometry* getGeometry() const;
  CESimpleGeometry* getFarGeometry();
  TObjInfo* getObjectInfo(); // TODO: Make this unnecessary
  
  void renderInstancesWithLOD(const std::vector<Transform>& instances, const Camera& camera);
  void renderBoundingBox(const glm::mat4& viewProjectionMatrix);
  void renderRadiusCylinder(const glm::mat4& viewProjectionMatrix);
  void renderPhysicsMeshDebug(const glm::mat4& viewProjectionMatrix);
  int determineLOD(const Transform& transform, const Camera& camera);
  
  const std::vector<Transform>& getTransforms() const;
  
  bool hasBoundingBox();
  const std::array<TBound, 8>& getBoundingBoxes() const;
};
