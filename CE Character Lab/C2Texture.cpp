#include "C2Texture.h"

#include <list>
#include <iterator>
#include <utility>

#include "bitmap.h"

#include "shader.h"


// These helpers and forward declares are only needed to support old code. Remove when able.
struct LimitTo
{
  LimitTo( const int n_ ) : n(n_)
  {}
  
  template< typename T >
  bool operator()( const T& )
  {
    return n-->0;
  }
  
  int n;
};
void BrightenTexture(WORD* A, int L);
void DATASHIFT(WORD* d, int cnt);
void GenerateMipMap(WORD* A, WORD* D, int L);
void ApplyAlphaFlags(WORD* tptr, int cnt);
void CalcMidColor(WORD* tptr, int l, int &mr, int &mg, int &mb);
extern WORD  FadeTab[65][0x8000]; // OOoooo scary. Be careful


C2Texture::~C2Texture()
{
  glDeleteTextures(1, &this->m_texture_id);
}

C2Texture::C2Texture(const std::vector<uint16_t>& raw_texture_data, int texture_size, int texture_height, int texture_width)
  : m_raw_data(raw_texture_data), m_height(texture_height), m_width(texture_width)
{
  this->loadTextureIntoHardwareMemory();
}

void C2Texture::Use()
{
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_texture_id);
}

void C2Texture::loadTextureIntoHardwareMemory()
{
  glGenTextures(1, &this->m_texture_id);
  glBindTexture(GL_TEXTURE_2D, m_texture_id);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, m_raw_data.data());

  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  
  glBindTexture(GL_TEXTURE_2D, 0);
}

// Saves the C2Texture as a bitmap (32 bit).
void C2Texture::saveToBMPFile(std::string file_name)
{
  CBitmap* cBit = new CBitmap();
  uint32_t RedMask = 0;
  uint32_t GreenMask = 0;
  uint32_t BlueMask = 0;
  uint32_t AlphaMask = 0;
  
  //argb1555
  RedMask = 0x7C00;
  GreenMask = 0x3E0;
  BlueMask = 0x1F;
  AlphaMask = 0x8000;
  
  // C2 textures are stored in memory as they feed into the graphics program. Flip the rows to store in the file.
  std::list<uint16_t> raw_texture_data_flipped;
  std::vector<uint16_t>::iterator raw_it = this->m_raw_data.begin();
  
  // swap subsections with the end
  for (int row=0; row < this->m_height; ++row) {
    raw_texture_data_flipped.insert(raw_texture_data_flipped.begin(), raw_it, raw_it+this->m_width);
    raw_it += this->m_width;
  }
  
  std::vector<uint16_t> v{ std::make_move_iterator(std::begin(raw_texture_data_flipped)),
    std::make_move_iterator(std::end(raw_texture_data_flipped)) };

  if (cBit->SetBits(v.data(), this->m_width, this->m_height, RedMask, GreenMask, BlueMask, AlphaMask)) {
    if (!cBit->Save(file_name.c_str())) {
      std::cout << "Failed to save bitmap!";
    }
  } else {
    std::cout << "Failed to save bitmap! set bits failed";
  }
  
  delete cBit;
}

std::vector<uint16_t>* C2Texture::getRawData()
{
  return &m_raw_data;
}
