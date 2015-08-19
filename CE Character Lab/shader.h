//
//  shader.h
//  CE Character Lab
//
//  Created by Tyler Minard on 8/18/15.
//  Copyright (c) 2015 Tyler Minard. All rights reserved.
//

#ifndef __CE_Character_Lab__shader__
#define __CE_Character_Lab__shader__

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <string>

#include "transform.h"

class Shader
{
public:
  Shader(const std::string& fileName);
  
  void Bind();
  void Update(const Transform& transform, const Camera& camera);
  
  virtual ~Shader();
protected:
private:
  static const unsigned int NUM_SHADERS = 2;
  static const unsigned int NUM_UNIFORMS = 3;
  void operator=(const Shader& shader) {}
  Shader(const Shader& shader) {}
  
  std::string LoadShader(const std::string& fileName);
  void CheckShaderError(GLuint shader, GLuint flag, bool isProgram, const std::string& errorMessage);
  GLuint CreateShader(const std::string& text, unsigned int type);
  
  GLuint m_program;
  GLuint m_shaders[NUM_SHADERS];
  GLuint m_uniforms[NUM_UNIFORMS];
};

#endif /* defined(__CE_Character_Lab__shader__) */
