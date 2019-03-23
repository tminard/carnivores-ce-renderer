//
//  CESimpleGeometry.hpp
//  CE Character Lab
//
//  Created by Minard, Tyler on 3/21/19.
//  Copyright © 2019 Tyler Minard. All rights reserved.
//

#pragma once

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <memory>
#include <vector>
#include <cstdint>
#include <fstream>

class Vertex;
class CETexture;
class ShaderProgram;
class Camera;
class Transform;

class CESimpleGeometry {
private:
  GLuint m_vertex_array_object;
  GLuint m_vertex_array_buffer;
  
  std::vector<Vertex> m_vertices;
  std::unique_ptr<CETexture> m_texture;
  
  std::unique_ptr<ShaderProgram> m_shader;
public:
  CESimpleGeometry(std::vector < Vertex > vertices, std::unique_ptr<CETexture> texture);
  ~CESimpleGeometry();
  
  void loadObjectIntoMemoryBuffer();
  
  CETexture* getTexture();
  ShaderProgram* getShader();
  
  void Update(Transform& transform, Camera& camera);
  void Draw();
  void DrawInstances();
};
