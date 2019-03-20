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

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>

#include <iostream>
#include <fstream>

#include "new_shader.h"

class CETexture;
class NewShader;
class Camera;
class Transform;

class C2Sky
{
private:
    const float SKY_DISTANCE = 40000.f;

    std::unique_ptr<CETexture> m_texture;
    std::unique_ptr<NewShader> m_shader;
    
    GLuint m_vertex_array_object;
    GLuint m_vertex_array_buffer;
    GLuint m_cube_texture;
    
    void loadIntoHardwareMemory();
    void loadTextureIntoMemory();
public:
    C2Sky(std::ifstream& instream);
    C2Sky(std::unique_ptr<CETexture> sky_texture);
    ~C2Sky();
    
    void saveTextureAsBMP(const std::string& file_name );
    void Render(GLFWwindow* window, Camera& camera);
};
