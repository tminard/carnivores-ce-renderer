//
//  CESimpleGeometry.cpp
//  CE Character Lab
//
//  Created by Minard, Tyler on 3/21/19.
//  Copyright © 2019 Tyler Minard. All rights reserved.
//

#include "CESimpleGeometry.h"
#include "CETexture.h"
#include "shader_program.h"
#include "vertex.h"
#include "camera.h"
#include "transform.h"

CESimpleGeometry::CESimpleGeometry(std::vector < Vertex > vertices, std::unique_ptr<CETexture> texture)
: m_vertices(vertices), m_texture(std::move(texture))
{
  this->loadObjectIntoMemoryBuffer();
}

CESimpleGeometry::~CESimpleGeometry()
{
  glDeleteBuffers(1, &this->m_vertex_array_buffer);
  glDeleteVertexArrays(1, &this->m_vertex_array_object);
}

ShaderProgram* CESimpleGeometry::getShader()
{
  return this->m_shader.get();
}

void CESimpleGeometry::loadObjectIntoMemoryBuffer()
{
  this->m_shader = std::unique_ptr<ShaderProgram>(new ShaderProgram("resources/shaders/simple_geo.vs", "resources/shaders/simple_geo.fs"));
  
  glGenBuffers(1, &this->m_vertex_array_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, this->m_vertex_array_buffer);
  glBufferData(GL_ARRAY_BUFFER, int(this->m_vertices.size() * sizeof(Vertex)), this->m_vertices.data(), GL_STATIC_DRAW);
  
  glGenVertexArrays(1, &this->m_vertex_array_object);
  glBindVertexArray(this->m_vertex_array_object);
  glBindBuffer(GL_ARRAY_BUFFER, this->m_vertex_array_buffer);

  glEnableVertexAttribArray(0); // position
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
  glEnableVertexAttribArray(1); // uv
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(glm::vec3));
  glEnableVertexAttribArray(2); // normal
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec3)+sizeof(glm::vec2)));
  
  glBindVertexArray(0);
}

// TODO: switch to glDrawElementsInstanced for billboards and https://learnopengl.com/Advanced-OpenGL/Instancing
// https://www.reddit.com/r/opengl/comments/55m1zg/help_with_figuring_out_gldrawelementsinstanced/
void CESimpleGeometry::Draw()
{
  this->m_shader->use();
  this->m_texture->use();
  glBindVertexArray(this->m_vertex_array_object);

  glDrawArrays(GL_TRIANGLES, 0, (int)this->m_vertices.size());
  
  glBindVertexArray(0);
}

void CESimpleGeometry::Update(Transform& transform, Camera& camera)
{
  glm::mat4 MVP = transform.GetStaticModelVP(camera);
  
  this->m_shader->use();
  this->m_shader->setMat4("MVP", MVP);
}
