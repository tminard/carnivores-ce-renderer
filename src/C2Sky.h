  //
  //  C2Sky.h
  //  CE Character Lab
  //
  //  Created by Tyler Minard on 8/18/15.
  //  Copyright (c) 2015 Tyler Minard. All rights reserved.
  //

#pragma once

#include <stdio.h>
#include <memory>
#include <vector>
#include <filesystem>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>

#include <iostream>
#include <fstream>

#include "shader_program.h"

class CETexture;
class ShaderProgram;
class Tga;

struct Camera;
struct Transform;

class C2Sky
{
private:
  const float SKY_DISTANCE = 8000.f;
  const float SKY_HEIGHT = 748.f;

  std::unique_ptr<ShaderProgram> m_shader;

  GLuint m_vertex_array_object;
  GLuint m_vertex_array_buffer;
  GLuint m_cube_texture;

  std::unique_ptr<CETexture> m_texture;
  std::unique_ptr<ShaderProgram> m_cloud_shader;
  GLuint m_cloud_uv_buffer;
  glm::vec4 m_sky_rgba;
  double last_cloud_time;
  float last_cloud_start;

  void loadIntoHardwareMemory();
  void loadTextureIntoMemory();
public:
  C2Sky(std::ifstream& instream, std::filesystem::path shaderPath);
  C2Sky(std::unique_ptr<CETexture> sky_texture, std::filesystem::path shaderPath);
  ~C2Sky();

  void saveTextureAsBMP(const std::string& file_name );
  void Render(GLFWwindow* window, Camera& camera);
  void setRGBA(glm::vec4 color);
  void updateClouds();
  
  const GLint getTextureID() const;
};
