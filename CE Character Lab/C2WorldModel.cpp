#include "C2WorldModel.h"

#include "C2Texture.h"
#include "C2Geometry.h"
#include "C2Animation.h"

#include "vertex.h"
#include <map>

#include "IndexedMeshLoader.h"

#include <glfw/glfw3.h>

#include "transform.h"
#include "shader.h"

C2WorldModel::C2WorldModel(std::ifstream& instream)
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
  texture_height = 256;//_tsize>>9; // All c2 stock objects bitmaps are 256 px height

  spirit_texture_data.resize(128*128);
  texture_data.resize(_tsize);
  face_data.resize(_fcount);
  file_vertex_data.resize(_vcount);
  _obj_buffer.resize(_object_count*48); // just throw these details away for now
  
  instream.read(reinterpret_cast<char *>(face_data.data()), _fcount<<6);
  instream.read(reinterpret_cast<char *>(file_vertex_data.data()), _vcount<<4);
  instream.read(reinterpret_cast<char *>(_obj_buffer.data()), _object_count*48);
  instream.read(reinterpret_cast<char *>(texture_data.data()), _tsize);

  for (int v=0; v < _vcount; v++) {
    file_vertex_data[v].x *= 0.25f;
    file_vertex_data[v].y *= 0.25f;
    file_vertex_data[v].z *= -0.25f;
  }

  IndexedMeshLoader* m_loader = new IndexedMeshLoader(file_vertex_data, face_data);

  // load the geo
  std::unique_ptr<C2Texture> mTexture = std::unique_ptr<C2Texture>(new C2Texture(texture_data, 256*256*2, 256, 256));
  std::unique_ptr<C2Geometry> mGeo = std::unique_ptr<C2Geometry>(new C2Geometry(m_loader->getVertices(), m_loader->getIndices(), std::move(mTexture)));
  
  instream.read(reinterpret_cast<char *>(spirit_texture_data.data()), 128*128*2);
  
  std::unique_ptr<C2Texture> cTexture = std::unique_ptr<C2Texture>(new C2Texture(spirit_texture_data, 128*128*2, 128, 128));
  this->m_far_texture = std::move(cTexture);
  this->m_geometry = std::move(mGeo);
  
  // load bmp model
  float mxx = file_vertex_data.at(0).x+0.5f;
  float mnx = file_vertex_data.at(0).x-0.5f;
  
  float mxy = file_vertex_data.at(0).x+0.5f;
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
  m_far_vertices[0].y = mxy;
  m_far_vertices[0].z = 0;
  
  m_far_vertices[1].x = mxx;
  m_far_vertices[1].y = mxy;
  m_far_vertices[1].z = 0;
  
  m_far_vertices[2].x = mxx;
  m_far_vertices[2].y = mny;
  m_far_vertices[2].z = 0;
  
  m_far_vertices[3].x = mnx;
  m_far_vertices[3].y = mny;
  m_far_vertices[3].z = 0;
  
  // process flags
  if (m_old_object_info->flags & objectNOLIGHT) {
    m_geometry->hint_ignoreLighting();
  }
  
  if (m_old_object_info->flags & objectANIMATED) {
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
    std::unique_ptr<C2Animation> mAni = std::unique_ptr<C2Animation>( new C2Animation("OBJECT_ANIMATION", kps, total_frames, total_ani_ms));
    mAni->setAnimationData(raw_animation_data);
    this->m_animation = std::move(mAni);
  }
  
  if (m_old_object_info->flags & objectBOUND) {
    std::vector<Vertex> vData = m_loader->getVertices();
    this->_generateBoundingBox(vData);
  }
  
  delete m_loader;
}

C2Geometry* C2WorldModel::getGeometry()
{
  return this->m_geometry.get();
}

void C2WorldModel::_generateBoundingBox(std::vector<Vertex>& vertex_data)
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

C2WorldModel::~C2WorldModel()
{
  delete this->m_old_object_info;
}

TObjInfo* C2WorldModel::getObjectInfo()
{
  return this->m_old_object_info;
}

void C2WorldModel::render()
{
  // update view matrix
  // set shader if different from current shader?
  // set texture if different from current texture?
  m_geometry->Draw();
}
