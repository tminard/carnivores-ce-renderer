#include <iterator>

#include "CEGeometry.h"
#include "CETexture.h"
#include "CEVertexBuffer.h"
#include "Vertex.h"

/*
 * The old C2 model format supported ignoring lighting.
 * We carry this over as a hint, which may be ignored
 * in the modern renderer.
 */
void CEGeometry::DEP_hint_ignoreLighting()
{
  // Yeah, ignore this for now
}

CEGeometry::CEGeometry(std::vector < Vertex > vertices, std::vector < uint32_t > indices, std::shared_ptr<CETexture> texture)
: m_vertices(vertices), m_indices(indices), m_texture(texture), m_vertex_buffer(std::make_unique<CEVertexBuffer>())
{
  this->m_vertex_buffer->uploadToGPU(m_vertices.data(), (int)m_vertices.size(), m_indices.data(), (int)m_indices.size());
}

CEGeometry::CEGeometry(std::vector < Vertex > vertices, std::vector < unsigned int > indices, std::shared_ptr<CETexture> texture, std::unique_ptr<CEVertexBuffer> vertex_buffer)
: m_vertices(vertices), m_indices(indices), m_texture(texture), m_vertex_buffer(std::move(vertex_buffer))
{
  this->m_vertex_buffer->uploadToGPU(m_vertices.data(), (int)m_vertices.size(), m_indices.data(), (int)m_indices.size());
}

CEGeometry::~CEGeometry()
{
}

CETexture* CEGeometry::getTexture()
{
  return this->m_texture.get();
}

void CEGeometry::saveTextureAsBMP(const std::string &file_name)
{
  this->m_texture->saveToBMPFile(file_name);
}

void CEGeometry::Draw()
{
  m_texture->Use();

  m_vertex_buffer->Draw();
}

void CEGeometry::exportAsOBJ(const std::string& file_name)
{
  
}

