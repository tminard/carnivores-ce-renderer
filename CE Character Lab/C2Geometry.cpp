/*
 //initialization
 glGenVertexArrays
 glBindVertexArray
 
 glGenBuffers
 glBindBuffer
 glBufferData
 
 glVertexAttribPointer
 glEnableVertexAttribArray
 
 glBindVertexArray(0)
 
 glDeleteBuffers //you can already delete it after the VAO is unbound, since the
 //VAO still references it, keeping it alive (see comments below).
 
 ...
 
 //rendering
 glBindVertexArray
 glDrawWhatever
 */

#include "C2Geometry.h"
#include <iterator>
#include "C2Texture.h"
#include "Vertex.h"

void BrightenTexture(WORD* A, int L);
void DATASHIFT(WORD* d, int cnt);
void GenerateModelMipMaps(TModel *mptr);
void GenerateAlphaFlags(TModel *mptr);
void CorrectModel(TModel *mptr);

C2Geometry::C2Geometry(std::vector<TPoint3d> vertex_data, std::vector<TFace> face_data, std::vector<WORD> texture_data, int texture_height)
  : m_vertices(vertex_data), m_faces(face_data), m_raw_texture_data(texture_data)
{
  this->_load_gl_object();
  this->_generate_textures(texture_data, texture_height);
  this->_generate_old_model_data();
}

C2Geometry::~C2Geometry()
{
  glDeleteVertexArrays(1, &this->m_vertexArrayObject);
	delete this->m_old_model_data;
}

std::vector<TPoint3d> C2Geometry::getVertices()
{
  return this->m_vertices;
}

void C2Geometry::setVertices(std::vector<TPoint3d> v)
{
  // TODO: assert size should match?
  this->m_vertices = v;
  
  // reload the buffer immediately
  glBindBuffer(GL_ARRAY_BUFFER, this->m_vertexArrayBuffer[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(TPoint3d)*this->m_vertices.size(), this->m_vertices.data(), GL_STATIC_DRAW);
}

void C2Geometry::removeLightInfo()
{
  memset(m_vertice_light, 0, sizeof(float)*4*1024);
  memcpy(this->m_old_model_data->VLight, this->m_vertice_light, sizeof(float)*4*1024);
}

void C2Geometry::_generate_textures(std::vector<WORD> texture_data, int texture_height)
{
  m_texture_data_a = texture_data;
  m_texture_height = texture_height;
}

TModel* C2Geometry::getTModel()
{
	return this->m_old_model_data;
}

void C2Geometry::syncOldVerticeData()
{
	std::copy(this->m_vertices.begin(), this->m_vertices.end(), this->m_old_model_data->gVertex);
}

void C2Geometry::saveTextureAsBMP(const std::string &file_name)
{
  std::unique_ptr<C2Texture> cTexture = std::unique_ptr<C2Texture>(new C2Texture(m_raw_texture_data, 256*256, 256, 256));
  cTexture->saveToBMPFile(file_name);
}

/*
 * WARNING: Memory leak potential, if TModel already defined.
 */
void C2Geometry::_generate_old_model_data()
{
	this->m_old_model_data = new TModel();

	this->m_old_model_data->VCount = (int)this->m_vertices.size(); // NOTE: potential bug, as max vertice/face count is now MAX_INT
	this->m_old_model_data->FCount = (int)this->m_faces.size();
	this->m_old_model_data->TextureSize = (int)this->m_texture_data_a.size(); // same as potential bug above. Max texture size is 46340x46340
	this->m_old_model_data->TextureHeight = this->m_texture_height;
	std::copy(this->m_faces.begin(), this->m_faces.end(), this->m_old_model_data->gFace);
	std::copy(this->m_vertices.begin(), this->m_vertices.end(), this->m_old_model_data->gVertex);
	memcpy(this->m_old_model_data->VLight, this->m_vertice_light, sizeof(float)*4*1024);
	this->m_old_model_data->lpTexture = new WORD[this->m_old_model_data->TextureSize];
	std::copy(this->m_texture_data_a.begin(), this->m_texture_data_a.end(), this->m_old_model_data->lpTexture);

	BrightenTexture(this->m_old_model_data->lpTexture, this->m_old_model_data->TextureSize/2);

	DATASHIFT(this->m_old_model_data->lpTexture, this->m_old_model_data->TextureSize);
	GenerateModelMipMaps(this->m_old_model_data);
	GenerateAlphaFlags(this->m_old_model_data);

	CorrectModel(this->m_old_model_data);
}

void C2Geometry::_load_gl_object()
{
  glGenVertexArrays(1, &this->m_vertexArrayObject);
  glBindVertexArray(this->m_vertexArrayObject);
  
  glGenBuffers(NUM_BUFFERS, this->m_vertexArrayBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, this->m_vertexArrayBuffer[POSITION_VB]);
  glBufferData(GL_ARRAY_BUFFER, (int)this->m_vertices.size()*sizeof(TPoint3d), this->m_vertices.data(), GL_STATIC_DRAW);
  
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(short)*2, 0);

  glBindVertexArray(0);
}

void C2Geometry::Render()
{
  glBindVertexArray(this->m_vertexArrayObject);
  
  glDrawArrays(GL_TRIANGLES, 0, (int)this->m_vertices.size());
  
  glBindVertexArray(0);
}

