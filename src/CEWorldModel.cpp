#include "CEWorldModel.h"

#include "CETexture.h"
#include "CEGeometry.h"
#include "CESimpleGeometry.h"
#include "CEAnimation.h"

#include "vertex.h"
#include <map>
#include <cmath>

#include "IndexedMeshLoader.h"
#include "shader_program.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// #include <GLFW/glfw3.h>

#include "transform.h"
#include "camera.h"

SquareBoundingBox ConvertTBoundToSquareBoundingBox(const TBound& bound) {
  SquareBoundingBox bbox;
  
  float halfWidth = (bound.a / 2.0f);
  float halfHeight = (bound.b / 2.0f);
  float halfSize = std::fmax(halfWidth, halfHeight);
  
  // Define the corners in local space (centered at the object's local origin), accounting for scaling
  glm::vec3 localCorners[4] = {
    {bound.cx - halfSize, bound.cy - halfHeight, 0},
    {bound.cx + halfSize, bound.cy - halfHeight, 0},
    {bound.cx + halfSize, bound.cy + halfHeight, 0},
    {bound.cx - halfSize, bound.cy + halfHeight, 0}
  };
  
  return bbox;
}

CEWorldModel::CEWorldModel(const CEMapType type, std::ifstream& instream)
{
  this->m_old_object_info = std::unique_ptr<TObjInfo>(new TObjInfo());
  
  instream.read(reinterpret_cast<char *>(this->m_old_object_info.get()), 64);
  this->m_old_object_info->Radius *= 2;
  this->m_old_object_info->YLo *= 2;
  this->m_old_object_info->YHi *= 2;
  this->m_old_object_info->linelenght = (this->m_old_object_info->linelenght / 128) * 128;
  
  int _vcount, _fcount, _object_count, _tsize, texture_height; // a C2 world model can contain multiple sub objects. The game doesn't care about this, and throws the details away.
  std::vector<uint8_t> _obj_buffer;
  std::vector<TFace> face_data;
  std::vector<TPoint3d> file_vertex_data;
  
  std::vector<uint16_t> texture_data; // object texture
  std::vector<uint16_t> spirit_texture_data;
  
  instream.read(reinterpret_cast<char *>(&_vcount), 4);
  instream.read(reinterpret_cast<char *>(&_fcount), 4);
  instream.read(reinterpret_cast<char *>(&_object_count), 4);
  instream.read(reinterpret_cast<char *>(&_tsize), 4);
  
  texture_height = 256; // All c2 stock objects bitmaps are 256 px height
  
  spirit_texture_data.resize(128*128);
  texture_data.resize(_tsize);
  face_data.resize(_fcount);
  file_vertex_data.resize(_vcount);
  _obj_buffer.resize((size_t)_object_count*48); // just throw these details away for now (see above note)
  
  instream.read(reinterpret_cast<char *>(face_data.data()), (long long)_fcount<<6);
  instream.read(reinterpret_cast<char *>(file_vertex_data.data()), (long long)_vcount<<4);
  instream.read(reinterpret_cast<char *>(_obj_buffer.data()), (long long)_object_count*48);
  instream.read(reinterpret_cast<char *>(texture_data.data()), _tsize);
  
  for (int v=0; v < _vcount; v++) {
    file_vertex_data[v].z *= -2.0f; // Original models need to be inverted across z axis
    file_vertex_data[v].x *= 2.0f;
    file_vertex_data[v].y *= 2.0f;
  }
  
  if (type == CEMapType::C2) instream.read(reinterpret_cast<char *>(spirit_texture_data.data()), 128*128*2);
  
  // load bmp model
  float mxx = file_vertex_data.at(0).x+0.5f;
  float mnx = file_vertex_data.at(0).x-0.5f;
  
  float mxy = file_vertex_data.at(0).y+0.5f;
  float mny = file_vertex_data.at(0).y-0.5f;
  
  for (int v=0; v<_vcount; v++) {
    float x = file_vertex_data.at(v).x;
    float y = file_vertex_data.at(v).y;
    if (x > mxx) mxx=x;
    if (x < mnx) mnx=x;
    if (y > mxy) mxy=y;
    if (y < mny) mny=y;
  }
  
  m_far_vertices[0].x = mnx;
  m_far_vertices[0].y = mny;
  m_far_vertices[0].z = 2; // UV - LL
  
  m_far_vertices[1].x = mnx;
  m_far_vertices[1].y = mxy;
  m_far_vertices[1].z = 0; // UL
  
  m_far_vertices[2].x = mxx;
  m_far_vertices[2].y = mny;
  m_far_vertices[2].z = 3; // LR
  
  
  m_far_vertices[3].x = mxx;
  m_far_vertices[3].y = mny;
  m_far_vertices[3].z = 3; // LR
  
  m_far_vertices[4].x = mnx;
  m_far_vertices[4].y = mxy;
  m_far_vertices[4].z = 0; // UL
  
  m_far_vertices[5].x = mxx;
  m_far_vertices[5].y = mxy;
  m_far_vertices[5].z = 1; // UR
  
  std::unique_ptr<IndexedMeshLoader> m_loader(new IndexedMeshLoader(file_vertex_data, face_data));
  
  // load the geo
  std::unique_ptr<CETexture> cTexture;
  if (type == CEMapType::C2) cTexture = std::unique_ptr<CETexture>(new CETexture(spirit_texture_data, 128*128*2, 128, 128));
  
  std::unique_ptr<CETexture> mTexture = std::unique_ptr<CETexture>(new CETexture(texture_data, 256*256*2, 256, 256));
  
  std::unique_ptr<CEGeometry> mGeo = std::unique_ptr<CEGeometry>(new CEGeometry(m_loader->getVertices(), m_loader->getIndices(), std::move(mTexture), "basic_shader"));
  
  std::vector<Vertex> cVertices;
  cVertices.clear();
  for (int v = 0; v < m_far_vertices.size(); v++) {
    glm::vec2 uv;
    switch ((int)m_far_vertices[v].z) {
      case 0: // UL
        uv = glm::vec2(0, 0);
        break;
      case 1: // UR
        uv = glm::vec2(1, 0);
        break;
      case 2: // LL
        uv = glm::vec2(0, 1);
        break;
      case 3: // LR
        uv = glm::vec2(1, 1);
        break;
    }
    Vertex vt(
              glm::vec3(m_far_vertices[v].x, m_far_vertices[v].y, 0.f),
              uv,
              glm::vec3(1.f,1.f,1.f), // TODO: build normals?
              false,
              1.f,
              0,
              faceHasOpacity
              );
    cVertices.push_back(vt);
  }
  
  this->m_geometry = std::move(mGeo);
  
  if (type == CEMapType::C2) {
    std::unique_ptr<CESimpleGeometry> cGeo = std::unique_ptr<CESimpleGeometry>(new CESimpleGeometry(cVertices, std::move(cTexture)));
    this->m_far_geometry = std::move(cGeo);
  }
  
  if (m_old_object_info->flags & objectANIMATED && type == CEMapType::C2) {
    std::vector<short int> raw_animation_data;
    int ani_vcount;
    int kps, total_frames, total_ani_ms;
    instream.read(reinterpret_cast<char *>(&ani_vcount), 4);
    instream.read(reinterpret_cast<char *>(&ani_vcount), 4); // Repeated, for some reason...
    instream.read(reinterpret_cast<char *>(&kps), 4);
    instream.read(reinterpret_cast<char *>(&total_frames), 4); // FILE FORMAT BUG: 1 minus actual amount
    instream.read(reinterpret_cast<char *>(&total_ani_ms), 4);
    total_frames++; // fix the bug in the file manually
    
    raw_animation_data.resize((size_t)ani_vcount*total_frames*6);
    total_ani_ms = (total_frames * 1000) / kps;
    instream.read(reinterpret_cast<char *>(raw_animation_data.data()), (long long)ani_vcount*total_frames*6);
    std::unique_ptr<CEAnimation> mAni = std::unique_ptr<CEAnimation>( new CEAnimation("OBJECT_ANIMATION", kps, total_frames, total_ani_ms));
    mAni->setAnimationData(raw_animation_data, ani_vcount, face_data, file_vertex_data);
    this->m_animation = std::move(mAni);
  }
  
//  if (m_old_object_info->flags & objectBOUND) {
//    std::vector<Vertex> vData = m_loader->getVertices();
//    this->_generateBoundingBox(vData);
//  }
}

CEGeometry* CEWorldModel::getGeometry() const
{
  return this->m_geometry.get();
}

bool CEWorldModel::hasBoundingBox() {
  return m_old_object_info->flags & objectBOUND;
}

const std::array<TBound, 8>& CEWorldModel::getBoundingBoxes() const {
  return m_bounding_box;
}

CESimpleGeometry* CEWorldModel::getFarGeometry()
{
  return this->m_far_geometry.get();
}

/**
 This is an implementation of the original logic, which generated _UP TO_ 8 bounding boxes.
 These appear to be bounding boxes across _sections_ of geo. E.g., vertices marked with "owner" (parent).
 */
void CEWorldModel::_generateBoundingBox(std::vector<Vertex>& vertex_data)
{
  float x1 = 0.0, x2=0.0, y1=0.0, y2=0.0, z1=0.0, z2=0.0;
  bool first;
  
  m_bounding_rec.clear();
  m_bounding_rec.resize(8);
  for (int o=0; o<8; o++) {
    first = true;
    // Mark as skipped (e.g., (sub)object has no visible vertices)
    m_bounding_box[o].a = -1;
    
    for (int v=0; v<(int)vertex_data.size(); v++) {
      Vertex p = vertex_data.at(v);
      if (p.isHidden()) continue;
      if (p.getOwner()!=o) continue;
      
      if (first) {
        x1 = p.getPos().x-1.0f;	x2 = p.getPos().x+1.0f;
        y1 = p.getPos().y-1.0f;	y2 = p.getPos().y+1.0f;
        z1 = p.getPos().z-1.0f;	z2 = p.getPos().z+1.0f;
        first = false;
      }
      
      if (p.getPos().x < x1) x1=p.getPos().x;
      if (p.getPos().x > x2) x2=p.getPos().x;
      
      if (p.getPos().y < y1) y1=p.getPos().y;
      if (p.getPos().y > y2) y2=p.getPos().y;
      
      if (p.getPos().z < z1) z1=p.getPos().z;
      if (p.getPos().z > z2) z2=p.getPos().z;
    }
    
    if (first) continue;
    
    x1-=72.f;
    x2+=72.f;
    z1-=72.f;
    z2+=72.f;
    
    // Why do we do this 8 times?
    // Smallest y position in model (min height)
    m_bounding_box[o].y1 = y1;
    // Largest y position in model (max height)
    m_bounding_box[o].y2 = y2;
    // Avg x (e.g, "center")
    m_bounding_box[o].cx = (x1+x2) / 2;
    // Avg y (center, across Z axis)
    m_bounding_box[o].cy = (z1+z2) / 2;
    m_bounding_box[o].a  = (x2-x1) / 2;
    m_bounding_box[o].b  = (z2-z1) / 2;
    
    m_bounding_rec.push_back(ConvertTBoundToSquareBoundingBox(m_bounding_box[o]));
  }
}

CEWorldModel::~CEWorldModel() {}

// Function to generate wireframe box vertices as triangles for AABB visualization
std::vector<Vertex> generateWireframeBox(const glm::vec3& min, const glm::vec3& max) {
  std::vector<Vertex> vertices;
  
  // Define 8 corners of the box
  std::vector<glm::vec3> corners = {
    {min.x, min.y, min.z}, // 0: bottom-left-front
    {max.x, min.y, min.z}, // 1: bottom-right-front 
    {max.x, max.y, min.z}, // 2: top-right-front
    {min.x, max.y, min.z}, // 3: top-left-front
    {min.x, min.y, max.z}, // 4: bottom-left-back
    {max.x, min.y, max.z}, // 5: bottom-right-back
    {max.x, max.y, max.z}, // 6: top-right-back
    {min.x, max.y, max.z}  // 7: top-left-back
  };
  
  // Create wireframe using thin triangular faces for the 12 edges
  // Each edge becomes a thin rectangular face made of 2 triangles
  float lineWidth = 0.01f; // Very thin lines
  
  std::vector<std::pair<int, int>> edges = {
    // Bottom face
    {0, 1}, {1, 2}, {2, 3}, {3, 0},
    // Top face  
    {4, 5}, {5, 6}, {6, 7}, {7, 4},
    // Vertical edges
    {0, 4}, {1, 5}, {2, 6}, {3, 7}
  };
  
  for (const auto& edge : edges) {
    glm::vec3 start = corners[edge.first];
    glm::vec3 end = corners[edge.second];
    glm::vec3 direction = glm::normalize(end - start);
    
    // Create a simple perpendicular vector for line thickness
    glm::vec3 perp = glm::vec3(0, 1, 0);
    if (glm::abs(glm::dot(direction, perp)) > 0.9f) {
      perp = glm::vec3(1, 0, 0);
    }
    perp = glm::normalize(glm::cross(direction, perp)) * lineWidth;
    
    // Create 4 vertices for the rectangular face (2 triangles)
    glm::vec3 v1 = start - perp;
    glm::vec3 v2 = start + perp;  
    glm::vec3 v3 = end + perp;
    glm::vec3 v4 = end - perp;
    
    // First triangle: v1, v2, v3
    vertices.emplace_back(v1, glm::vec2(0, 0), glm::vec3(0, 1, 0), false, 1.0f, 0, 0);
    vertices.emplace_back(v2, glm::vec2(0, 0), glm::vec3(0, 1, 0), false, 1.0f, 0, 0);
    vertices.emplace_back(v3, glm::vec2(0, 0), glm::vec3(0, 1, 0), false, 1.0f, 0, 0);
    
    // Second triangle: v1, v3, v4
    vertices.emplace_back(v1, glm::vec2(0, 0), glm::vec3(0, 1, 0), false, 1.0f, 0, 0);
    vertices.emplace_back(v3, glm::vec2(0, 0), glm::vec3(0, 1, 0), false, 1.0f, 0, 0);
    vertices.emplace_back(v4, glm::vec2(0, 0), glm::vec3(0, 1, 0), false, 1.0f, 0, 0);
  }
  
  return vertices;
}

void CEWorldModel::renderBoundingBox(const glm::mat4& viewProjectionMatrix) {
  // Debug: Check model data and flags
  static bool firstDebug = true;
  if (firstDebug) {
    std::cout << "🔍 Model debug info:" << std::endl;
    std::cout << "   Has objectBOUND flag: " << (hasBoundingBox() ? "YES" : "NO") << std::endl;
    std::cout << "   Object flags: 0x" << std::hex << m_old_object_info->flags << std::dec << std::endl;
    std::cout << "   Transform instances: " << m_transforms.size() << std::endl;
    std::cout << "   Object radius: " << m_old_object_info->Radius << std::endl;
    std::cout << "   Object YLo/YHi: " << m_old_object_info->YLo << "/" << m_old_object_info->YHi << std::endl;
    firstDebug = false;
  }
  
  // For debugging purposes, let's generate bounding boxes for all objects, not just those with objectBOUND
  // This matches the original Carnivores approach where physics AABB could be generated for any object
  if (m_transforms.empty()) {
    return; // No instances to render
  }
  
  // Static variables to avoid recreation every frame
  static std::unique_ptr<CESimpleGeometry> boundingBoxGeometry;
  static bool geometryInitialized = false;
  
  // Initialize geometry once
  if (!geometryInitialized) {
    // Create a simple wireframe box geometry (unit cube)
    glm::vec3 min(-0.5f, -0.5f, -0.5f);
    glm::vec3 max(0.5f, 0.5f, 0.5f);
    std::vector<Vertex> boxVertices = generateWireframeBox(min, max);
    
    // Create a simple white texture for the wireframe
    const int texSize = 4;
    std::vector<uint16_t> whiteTextureData(texSize * texSize, 0xFFFF);
    std::unique_ptr<CETexture> wireframeTexture = std::make_unique<CETexture>(whiteTextureData, texSize * texSize * 2, texSize, texSize);
    
    boundingBoxGeometry = std::make_unique<CESimpleGeometry>(boxVertices, std::move(wireframeTexture));
    geometryInitialized = true;
  }
  
  // Collect all transform matrices for bounding boxes
  std::vector<glm::mat4> boundingBoxTransforms;
  
  // Create transforms for each transform instance of this model
  for (const auto& modelTransform : m_transforms) {
    glm::vec3 modelPosition = *const_cast<Transform&>(modelTransform).GetPos();
    
    // Generate AABB from object's basic dimensions scaled to match visual objects
    float radius = m_old_object_info->Radius * 0.0625f; // Scale down 16x to match rendered objects
    float yLo = m_old_object_info->YLo * 0.0625f;
    float yHi = m_old_object_info->YHi * 0.0625f;
    
    if (radius > 0) { // Valid object with dimensions
      // Create a simple cylindrical AABB using radius and height
      glm::vec3 aabbMin(-radius, yLo, -radius);
      glm::vec3 aabbMax(radius, yHi, radius);
      
      // Calculate size and center
      glm::vec3 size = aabbMax - aabbMin;
      glm::vec3 center = (aabbMax + aabbMin) * 0.5f;
      
      // Create transform matrix: translate to world position + local center, then scale by AABB size  
      glm::mat4 transform = glm::mat4(1.0f);
      transform = glm::translate(transform, modelPosition + center);
      transform = glm::scale(transform, size);
      
      boundingBoxTransforms.push_back(transform);
    }
  }
  
  // Render all bounding boxes if we have any
  if (!boundingBoxTransforms.empty()) {
    // Set up shader for wireframe rendering
    auto shader = boundingBoxGeometry->getShader();
    if (shader) {
      shader->use();
      shader->setBool("useCustomColor", true);
      shader->setVec3("customColor", glm::vec3(0.0f, 1.0f, 1.0f)); // Cyan wireframe boxes
      shader->setBool("enableShadows", false);
      shader->setMat4("projection_view", viewProjectionMatrix); // Set the camera matrices
    }
    
    // Update instanced transforms and render
    boundingBoxGeometry->UpdateInstances(boundingBoxTransforms);
    boundingBoxGeometry->DrawInstances();
    
    // Reset shader state
    if (shader) {
      shader->setBool("useCustomColor", false);
    }
  }
}

void CEWorldModel::renderRadiusCylinder(const glm::mat4& viewProjectionMatrix)
{
  if (!m_old_object_info || m_old_object_info->Radius <= 0) {
    return; // No valid radius
  }
  
  // Static variables to avoid recreation every frame
  static std::unique_ptr<CESimpleGeometry> radiusGeometry;
  static bool radiusGeometryInitialized = false;
  
  // Initialize radius cylinder geometry once
  if (!radiusGeometryInitialized) {
    // Create cylinder vertices (circle at base and top)
    std::vector<Vertex> cylinderVertices;
    
    // Generate cylinder wireframe with 16 segments
    int segments = 16;
    float radius = 1.0f; // Unit cylinder, scaled per instance
    
    // Bottom circle
    for (int i = 0; i < segments; i++) {
      float angle1 = (float)i / segments * 2.0f * M_PI;
      float angle2 = (float)(i + 1) / segments * 2.0f * M_PI;
      
      // Bottom circle edge
      cylinderVertices.push_back(Vertex(glm::vec3(cos(angle1) * radius, 0.0f, sin(angle1) * radius), glm::vec2(0, 0), glm::vec3(0, 1, 0), false, 1.0f, 0, 0));
      cylinderVertices.push_back(Vertex(glm::vec3(cos(angle2) * radius, 0.0f, sin(angle2) * radius), glm::vec2(0, 0), glm::vec3(0, 1, 0), false, 1.0f, 0, 0));
      
      // Top circle edge  
      cylinderVertices.push_back(Vertex(glm::vec3(cos(angle1) * radius, 1.0f, sin(angle1) * radius), glm::vec2(0, 0), glm::vec3(0, 1, 0), false, 1.0f, 0, 0));
      cylinderVertices.push_back(Vertex(glm::vec3(cos(angle2) * radius, 1.0f, sin(angle2) * radius), glm::vec2(0, 0), glm::vec3(0, 1, 0), false, 1.0f, 0, 0));
      
      // Vertical lines connecting top and bottom
      cylinderVertices.push_back(Vertex(glm::vec3(cos(angle1) * radius, 0.0f, sin(angle1) * radius), glm::vec2(0, 0), glm::vec3(0, 1, 0), false, 1.0f, 0, 0));
      cylinderVertices.push_back(Vertex(glm::vec3(cos(angle1) * radius, 1.0f, sin(angle1) * radius), glm::vec2(0, 0), glm::vec3(0, 1, 0), false, 1.0f, 0, 0));
    }
    
    // Create a simple white texture for the wireframe  
    const int texSize = 4;
    std::vector<uint16_t> whiteTextureData(texSize * texSize, 0xFFFF);
    auto wireframeTexture = std::make_unique<CETexture>(whiteTextureData, texSize * texSize * 2, texSize, texSize);
    radiusGeometry = std::make_unique<CESimpleGeometry>(cylinderVertices, std::move(wireframeTexture));
    radiusGeometryInitialized = true;
  }
  
  // Scale radius down to match visual object scaling (16x reduction)  
  float radius = m_old_object_info->Radius * 0.5f * 0.0625f; // Scale down 16x to match rendered objects
  float height = (m_old_object_info->YHi - m_old_object_info->YLo) * 0.0625f; // Scale down 16x to match rendered objects
  
  // Debug: Log radius values and compare to bounding box
  static int debugObjectCount = 0;
  debugObjectCount++;
  if (debugObjectCount <= 5) {
    // Calculate what the bounding box size would be
    glm::vec3 aabbMin(-radius, m_old_object_info->YLo, -radius);
    glm::vec3 aabbMax(radius, m_old_object_info->YHi, radius);
    glm::vec3 aabbSize = aabbMax - aabbMin;
    
    std::cout << "🟡 Radius debug " << debugObjectCount << ": radius=" << radius 
              << " height=" << height 
              << " aabb_width=" << aabbSize.x << " aabb_depth=" << aabbSize.z
              << " YLo=" << m_old_object_info->YLo << " YHi=" << m_old_object_info->YHi << std::endl;
  }
  
  // Collect transforms for all instances 
  std::vector<glm::mat4> radiusTransforms;
  int instanceCount = 0;
  for (const auto& transform : m_transforms) {
    glm::vec3 position = *const_cast<Transform&>(transform).GetPos();
    
    // Debug: Log first few positions
    if (instanceCount < 3 && debugObjectCount <= 5) {
      std::cout << "   Instance " << instanceCount << " pos=[" << position.x << "," << position.y << "," << position.z << "]" << std::endl;
    }
    
    // Create transform matrix: translate to world position + YLo offset, then scale by radius and height
    glm::mat4 matrix = glm::translate(glm::mat4(1.0f), position + glm::vec3(0, m_old_object_info->YLo, 0));
    matrix = glm::scale(matrix, glm::vec3(radius, height, radius));
    
    radiusTransforms.push_back(matrix);
    instanceCount++;
  }
  
  // Render all radius cylinders if we have any
  if (!radiusTransforms.empty()) {
    // Set up shader for wireframe rendering
    auto shader = radiusGeometry->getShader();
    if (shader) {
      shader->use();
      shader->setBool("useCustomColor", true);
      shader->setVec3("customColor", glm::vec3(1.0f, 1.0f, 0.0f)); // Yellow wireframe cylinders
      shader->setBool("enableShadows", false);
      shader->setMat4("projection_view", viewProjectionMatrix);
    }
    
    // Update instanced transforms and render
    radiusGeometry->UpdateInstances(radiusTransforms);
    radiusGeometry->DrawInstances();
    
    // Reset shader state
    if (shader) {
      shader->setBool("useCustomColor", false);
    }
  }
}

TObjInfo* CEWorldModel::getObjectInfo()
{
  return this->m_old_object_info.get();
}

void CEWorldModel::renderFar(Transform& transform, Camera& camera)
{
  m_far_geometry->Update(transform, camera);
  m_far_geometry->Draw();
}

void CEWorldModel::render(Transform& transform, Camera& camera)
{
  m_geometry->Update(transform, camera);
  m_geometry->Draw();
}

void CEWorldModel::addFar(Transform& transform)
{
  this->m_far_instances.push_back(transform.GetStaticModel());
}

void CEWorldModel::updateFarInstances()
{
  m_far_geometry->UpdateInstances(this->m_far_instances);
  this->m_far_instances.clear();
}

void CEWorldModel::renderFarInstances()
{
  m_far_geometry->DrawInstances();
}

void CEWorldModel::addNear(Transform& transform)
{
  this->m_near_instances.push_back(transform.GetStaticModel());
  this->m_transforms.push_back(transform);
}

void CEWorldModel::updateNearInstances()
{
  m_geometry->UpdateInstances(this->m_near_instances);
  this->m_near_instances.clear();
}

void CEWorldModel::renderNearInstances()
{
  m_geometry->DrawInstances();
}

const std::vector<Transform>& CEWorldModel::getTransforms() const {
  return m_transforms;
}
