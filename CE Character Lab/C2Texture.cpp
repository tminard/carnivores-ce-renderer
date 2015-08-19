#include "C2Texture.h"

#include <list>
#include <iterator>
#include <utility>

#include "bitmap.h"


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
  if (_did_load_map_texture_data) {
    delete m_old_texture_data;
  }
}

C2Texture::C2Texture(const std::vector<uint16_t>& raw_texture_data, int texture_size, int texture_height, int texture_width)
  : m_raw_data(raw_texture_data), m_height(texture_height), m_width(texture_width)
{
}

void C2Texture::brighten()
{
  BrightenTexture(this->m_raw_data.data(), (int)this->m_raw_data.capacity()/2);
}

// WARNING: Only call this once! Otherwise, CRAZY things may happen
void C2Texture::_generate_old_texture_data()
{
  if (_did_load_map_texture_data) {
    return;
  }

  this->m_old_texture_data = new TEXTURE;

  if (this->m_height < 64 || this->m_width < 64) {
    throw "ERROR: using C2Texture with image smaller than 64x64. Edit base class to bypass TEXTURE generation.";
  }

  // copy over
  std::copy_if(this->m_raw_data.begin(), this->m_raw_data.end(), this->m_old_texture_data->DataA, LimitTo(128*128)); // note old TEXTURE data is limited to 128*128. results may be unpredictable if height <> 128 or width <> 128

  // copied this from old code
  int tsize_limited = std::min(this->m_height*this->m_width, 128*128);
  for (int td = 0; td < tsize_limited; td++) {
    if (!this->m_old_texture_data->DataA[td]) {
      this->m_old_texture_data->DataA[td] = 1;
    }
  }
  
  BrightenTexture(this->m_old_texture_data->DataA, tsize_limited);
  CalcMidColor(this->m_old_texture_data->DataA, tsize_limited, this->m_old_texture_data->mR, this->m_old_texture_data->mG, this->m_old_texture_data->mB); // When you remove this, be sure to introduce mid color to this class. Needed for map view

  GenerateMipMap(this->m_old_texture_data->DataA, this->m_old_texture_data->DataB, 64); // Note: we assume that textures smaller than 128*128 using this class will not be a problem at 64*64... Be careful
  GenerateMipMap(this->m_old_texture_data->DataB, this->m_old_texture_data->DataC, 32);
  GenerateMipMap(this->m_old_texture_data->DataC, this->m_old_texture_data->DataD, 16);
  memcpy(this->m_old_texture_data->SDataC[0], this->m_old_texture_data->DataC, 32*32*2);
  memcpy(this->m_old_texture_data->SDataC[1], this->m_old_texture_data->DataC, 32*32*2);
  
  DATASHIFT((unsigned short *)this->m_old_texture_data, sizeof(TEXTURE)); // WTF
  for (int w=0; w<32*32; w++)
    this->m_old_texture_data->SDataC[1][w] = FadeTab[48][this->m_old_texture_data->SDataC[1][w]>>1];
  
  ApplyAlphaFlags(this->m_old_texture_data->DataA, tsize_limited);
  ApplyAlphaFlags(this->m_old_texture_data->DataB, 64*64);
  ApplyAlphaFlags(this->m_old_texture_data->DataC, 32*32);
  
  _did_load_map_texture_data = true;
}

// Not really thread safe. TODO: make this unnecessary. Generates on first use. ONLY use for map textures! Assumes 128*128.
TEXTURE* C2Texture::getMapTextureData()
{
  if (_did_load_map_texture_data) {
    return this->m_old_texture_data;
  } else {
    this->_generate_old_texture_data();
    return this->m_old_texture_data;
  }
}

// Saves the C2Texture as a bitmap (32 bit).
// TODO: This is not very efficient...
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
}