#include "CEWorldModel.h"

#include "CETexture.h"
#include "CEGeometry.h"
#include "CESimpleGeometry.h"
#include "CEAnimation.h"

#include "vertex.h"
#include <map>

#include "IndexedMeshLoader.h"

#include <GLFW/glfw3.h>

#include "transform.h"
#include "camera.h"

CEWorldModel::CEWorldModel(const CEMapType type, std::ifstream& instream)
{
  this->m_old_object_info = new TObjInfo();
  
  instream.read(reinterpret_cast<char *>(this->m_old_object_info), 64);
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
  _obj_buffer.resize(_object_count*48); // just throw these details away for now (see above note)
  
  instream.read(reinterpret_cast<char *>(face_data.data()), _fcount<<6);
  instream.read(reinterpret_cast<char *>(file_vertex_data.data()), _vcount<<4);
  instream.read(reinterpret_cast<char *>(_obj_buffer.data()), _object_count*48);
  instream.read(reinterpret_cast<char *>(texture_data.data()), _tsize);
  
  for (int v=0; v < _vcount; v++) {
    file_vertex_data[v].z *= -1.0f; // Original models need to be inverted across z axis
  }

  if (type == C2) instream.read(reinterpret_cast<char *>(spirit_texture_data.data()), 128*128*2);
  
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
  
  IndexedMeshLoader* m_loader = new IndexedMeshLoader(file_vertex_data, face_data);
  
  // load the geo
  std::unique_ptr<CETexture> cTexture;
  if (type == C2) cTexture = std::unique_ptr<CETexture>(new CETexture(spirit_texture_data, 128*128*2, 128, 128));

  std::unique_ptr<CETexture> mTexture = std::unique_ptr<CETexture>(new CETexture(texture_data, 256*256*2, 256, 256));
  
  std::unique_ptr<CEGeometry> mGeo = std::unique_ptr<CEGeometry>(new CEGeometry(m_loader->getVertices(), m_loader->getIndices(), std::move(mTexture)));
  
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

  if (type == C2) {
    std::unique_ptr<CESimpleGeometry> cGeo = std::unique_ptr<CESimpleGeometry>(new CESimpleGeometry(cVertices, std::move(cTexture)));
    this->m_far_geometry = std::move(cGeo);
  }
  
  if (m_old_object_info->flags & objectANIMATED && type == C2) {
    std::vector<short int> raw_animation_data;
    int ani_vcount;
    int kps, total_frames, total_ani_ms;
    instream.read(reinterpret_cast<char *>(&ani_vcount), 4); // some vertice info we dont need
    instream.read(reinterpret_cast<char *>(&ani_vcount), 4); // Repeated, for some reason...
    instream.read(reinterpret_cast<char *>(&kps), 4);
    instream.read(reinterpret_cast<char *>(&total_frames), 4); // FILE FORMAT BUG: 1 minus actual amount
    instream.read(reinterpret_cast<char *>(&total_ani_ms), 4);
    total_frames++; // fix the bug in the file manually
    
    raw_animation_data.resize(ani_vcount*total_frames*6);
    total_ani_ms = (total_frames * 1000) / kps;
    instream.read(reinterpret_cast<char *>(raw_animation_data.data()), ani_vcount*total_frames*6);
    std::unique_ptr<CEAnimation> mAni = std::unique_ptr<CEAnimation>( new CEAnimation("OBJECT_ANIMATION", kps, total_frames, total_ani_ms));
    mAni->setAnimationData(raw_animation_data);
    this->m_animation = std::move(mAni);
  }
  
  if (m_old_object_info->flags & objectBOUND) {
    std::vector<Vertex> vData = m_loader->getVertices();
    this->_generateBoundingBox(vData);
  }
  
  delete m_loader;
}

CEGeometry* CEWorldModel::getGeometry()
{
  return this->m_geometry.get();
}

CESimpleGeometry* CEWorldModel::getFarGeometry()
{
  return this->m_far_geometry.get();
}

void CEWorldModel::_generateBoundingBox(std::vector<Vertex>& vertex_data)
{
  float x1 = 0.0, x2=0.0, y1=0.0, y2=0.0, z1=0.0, z2=0.0;
  bool first;
  
  for (int o=0; o<8; o++) {
    first = true;
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
    
    m_bounding_box[o].y1 = y1;
    m_bounding_box[o].y2 = y2;
    m_bounding_box[o].cx = (x1+x2) / 2;
    m_bounding_box[o].cy = (z1+z2) / 2;
    m_bounding_box[o].a  = (x2-x1) / 2;
    m_bounding_box[o].b  = (z2-z1) / 2;
    
  }
}

CEWorldModel::~CEWorldModel()
{
  delete this->m_old_object_info;
}

TObjInfo* CEWorldModel::getObjectInfo()
{
  return this->m_old_object_info;
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
