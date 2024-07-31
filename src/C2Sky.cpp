  //
  //  C2Sky.cpp
  //  CE Character Lab
  //
  //  Created by Tyler Minard on 8/18/15.
  //  Copyright (c) 2015 Tyler Minard. All rights reserved.
  //

#include "C2Sky.h"

#include "CETexture.h"
#include "transform.h"
#include "camera.h"

#include "tga.h"

C2Sky::C2Sky(std::ifstream& instream, std::filesystem::path shaderPath)
{
  std::vector<uint16_t> raw_sky_texture_data;
  raw_sky_texture_data.resize(256*256);
  instream.read(reinterpret_cast<char *>(raw_sky_texture_data.data()), 256*256*sizeof(uint16_t));

  this->m_texture = std::unique_ptr<CETexture>(new CETexture(raw_sky_texture_data, 256*256, 256, 256));
  this->m_shader = std::unique_ptr<ShaderProgram>(new ShaderProgram((shaderPath / "sky.vs").string(), (shaderPath / "sky.fs").string()));
  this->m_cloud_shader = std::unique_ptr<ShaderProgram>(new ShaderProgram((shaderPath / "sky_clouds.vs").string(), (shaderPath / "sky_clouds.fs").string()));

  this->loadIntoHardwareMemory();
}

C2Sky::C2Sky(std::unique_ptr<CETexture> sky_texture, std::filesystem::path shaderPath)
: m_texture(std::move(sky_texture))
{
  this->m_shader = std::unique_ptr<ShaderProgram>(new ShaderProgram((shaderPath / "sky.vs").string(), (shaderPath / "sky.fs").string()));
  this->m_cloud_shader = std::unique_ptr<ShaderProgram>(new ShaderProgram((shaderPath / "sky_clouds.vs").string(), (shaderPath / "sky_clouds.fs").string()));
  this->loadIntoHardwareMemory();
}

C2Sky::~C2Sky()
{
  glDeleteBuffers(1, &this->m_vertex_array_buffer);
  glDeleteBuffers(1, &this->m_cloud_uv_buffer);
  glDeleteTextures(1, &this->m_cube_texture);
  glDeleteVertexArrays(1, &this->m_vertex_array_object);
}

const GLint C2Sky::getTextureID() const {
  return m_texture->getTextureID();
}

void C2Sky::saveTextureAsBMP(const std::string &file_name)
{
  this->m_texture->saveToBMPFile(file_name);
}

void C2Sky::loadTextureIntoMemory()
{
    // Generate and bind the cubemap texture
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    /*
     GL_TEXTURE_CUBE_MAP_POSITIVE_X    Right
     GL_TEXTURE_CUBE_MAP_NEGATIVE_X    Left
     GL_TEXTURE_CUBE_MAP_POSITIVE_Y    Top
     GL_TEXTURE_CUBE_MAP_NEGATIVE_Y    Bottom
     GL_TEXTURE_CUBE_MAP_POSITIVE_Z    Back
     GL_TEXTURE_CUBE_MAP_NEGATIVE_Z    Front
     */

    for (int i = 0; i < 6; i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RED, 256, 256, 0, GL_RED, GL_UNSIGNED_BYTE, this->m_texture->getRawData()->data());

        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cerr << "OpenGL error after glTexImage2D for face " << i << ": " << error << std::endl;
            // Handle the error appropriately here
            // Cleanup before returning to avoid leaking resources
            glDeleteTextures(1, &textureID);
            return;
        }
    }

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // Store the texture ID for later use
    this->m_cube_texture = textureID;

    // Unbind the texture
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void C2Sky::updateClouds()
{
  double time = glfwGetTime();
  double time_delta = this->last_cloud_time - time;

  float max_uv = (SKY_DISTANCE / 256.f);

  float unit_sec = (max_uv / (240.f * 10.f));
  float start = last_cloud_start + (unit_sec * (float)time_delta);
  
  float cloudUV[] = {
    start, start,
    max_uv, start,
    max_uv, max_uv,
    max_uv, max_uv,
    start, max_uv,
    start, start
  };

  // UPDATE BUFFER
  glBindBuffer(GL_ARRAY_BUFFER, this->m_cloud_uv_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cloudUV), &cloudUV, GL_STATIC_DRAW);

  last_cloud_time = time;
  last_cloud_start = start;
}

  // Build objects
void C2Sky::loadIntoHardwareMemory()
{
  float start = (float)glfwGetTime();
  float r_f = (SKY_DISTANCE / 256.f / 4.f) + start;
  float cloudUV[] = {
    start, start,
    r_f, start,
    r_f, r_f,
    r_f, r_f,
    start, r_f,
    start, start
  };
  this->last_cloud_time = glfwGetTime();
  this->last_cloud_start = start;

  float skyboxVertices[] = {
      // Clouds
    -SKY_DISTANCE,  SKY_HEIGHT, -SKY_DISTANCE, // LL
    SKY_DISTANCE,  SKY_HEIGHT, -SKY_DISTANCE, // UL
    SKY_DISTANCE,  SKY_HEIGHT,  SKY_DISTANCE, // UR

    SKY_DISTANCE,  SKY_HEIGHT,  SKY_DISTANCE, // UR
    -SKY_DISTANCE,  SKY_HEIGHT,  SKY_DISTANCE, // LR
    -SKY_DISTANCE,  SKY_HEIGHT, -SKY_DISTANCE, // LL

      // box
    -SKY_DISTANCE,  SKY_DISTANCE, -SKY_DISTANCE,
    -SKY_DISTANCE, -SKY_DISTANCE, -SKY_DISTANCE,
    SKY_DISTANCE, -SKY_DISTANCE, -SKY_DISTANCE,
    SKY_DISTANCE, -SKY_DISTANCE, -SKY_DISTANCE,
    SKY_DISTANCE,  SKY_DISTANCE, -SKY_DISTANCE,
    -SKY_DISTANCE,  SKY_DISTANCE, -SKY_DISTANCE,

    -SKY_DISTANCE, -SKY_DISTANCE,  SKY_DISTANCE,
    -SKY_DISTANCE, -SKY_DISTANCE, -SKY_DISTANCE,
    -SKY_DISTANCE,  SKY_DISTANCE, -SKY_DISTANCE,
    -SKY_DISTANCE,  SKY_DISTANCE, -SKY_DISTANCE,
    -SKY_DISTANCE,  SKY_DISTANCE,  SKY_DISTANCE,
    -SKY_DISTANCE, -SKY_DISTANCE,  SKY_DISTANCE,

    SKY_DISTANCE, -SKY_DISTANCE, -SKY_DISTANCE,
    SKY_DISTANCE, -SKY_DISTANCE,  SKY_DISTANCE,
    SKY_DISTANCE,  SKY_DISTANCE,  SKY_DISTANCE,
    SKY_DISTANCE,  SKY_DISTANCE,  SKY_DISTANCE,
    SKY_DISTANCE,  SKY_DISTANCE, -SKY_DISTANCE,
    SKY_DISTANCE, -SKY_DISTANCE, -SKY_DISTANCE,

    -SKY_DISTANCE, -SKY_DISTANCE,  SKY_DISTANCE,
    -SKY_DISTANCE,  SKY_DISTANCE,  SKY_DISTANCE,
    SKY_DISTANCE,  SKY_DISTANCE,  SKY_DISTANCE,
    SKY_DISTANCE,  SKY_DISTANCE,  SKY_DISTANCE,
    SKY_DISTANCE, -SKY_DISTANCE,  SKY_DISTANCE,
    -SKY_DISTANCE, -SKY_DISTANCE,  SKY_DISTANCE,

      // TOP
    -SKY_DISTANCE,  SKY_DISTANCE, -SKY_DISTANCE,
    SKY_DISTANCE,  SKY_DISTANCE, -SKY_DISTANCE,
    SKY_DISTANCE,  SKY_DISTANCE,  SKY_DISTANCE,
    SKY_DISTANCE,  SKY_DISTANCE,  SKY_DISTANCE,
    -SKY_DISTANCE,  SKY_DISTANCE,  SKY_DISTANCE,
    -SKY_DISTANCE,  SKY_DISTANCE, -SKY_DISTANCE,

    -SKY_DISTANCE, -SKY_DISTANCE, -SKY_DISTANCE,
    -SKY_DISTANCE, -SKY_DISTANCE,  SKY_DISTANCE,
    SKY_DISTANCE, -SKY_DISTANCE, -SKY_DISTANCE,
    SKY_DISTANCE, -SKY_DISTANCE, -SKY_DISTANCE,
    -SKY_DISTANCE, -SKY_DISTANCE,  SKY_DISTANCE,
    SKY_DISTANCE, -SKY_DISTANCE,  SKY_DISTANCE
  };

  this->loadTextureIntoMemory();
  this->m_shader->setInt("skybox", 0);

  glGenBuffers(1, &this->m_vertex_array_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, this->m_vertex_array_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

  glGenVertexArrays(1, &this->m_vertex_array_object);
  glBindVertexArray(this->m_vertex_array_object);
  glBindBuffer(GL_ARRAY_BUFFER, this->m_vertex_array_buffer);

    // configure shader locations
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);

  glGenBuffers(1, &this->m_cloud_uv_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, this->m_cloud_uv_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cloudUV), &cloudUV, GL_STATIC_DRAW);

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);

  glBindVertexArray(0);
}

void C2Sky::setRGBA(glm::vec4 color)
{
  m_sky_rgba = color;

  this->m_shader->use();
  this->m_shader->setVec4("sky_color", m_sky_rgba.r / 255.f, m_sky_rgba.g / 255.f, m_sky_rgba.b / 255.f, m_sky_rgba.a);
  this->m_cloud_shader->use();
  this->m_cloud_shader->setVec4("sky_color", m_sky_rgba.r / 255.f, m_sky_rgba.g / 255.f, m_sky_rgba.b / 255.f, m_sky_rgba.a);
}

void C2Sky::Render(GLFWwindow* window, Camera& camera)
{
  //this->updateClouds();
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);

  glm::mat4 view = camera.GetVM();
  view[3][0] = 0.0f;
  view[3][1] = 0.0f;
  view[3][2] = 0.0f;

  glm::mat4 projection = glm::perspective(glm::radians(80.0f), (float)width / (float)height, 0.1f, 100000.f);
  float elapsedTime = static_cast<float>(glfwGetTime());

  // Render the skybox
  glDepthFunc(GL_LEQUAL); // Change depth function so depth test passes when values are equal to depth buffer's content
  this->m_shader->use();
  this->m_shader->setMat4("view", view);
  this->m_shader->setMat4("projection", projection);
  this->m_shader->setFloat("time", elapsedTime); // Pass the elapsed time to the shader


  glBindVertexArray(this->m_vertex_array_object);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, this->m_cube_texture);

  glDrawArrays(GL_TRIANGLES, 6, 36);

  this->m_cloud_shader->use();
  this->m_cloud_shader->setMat4("view", view);
  this->m_cloud_shader->setMat4("projection", projection);
  this->m_cloud_shader->setFloat("time", elapsedTime); // Pass the elapsed time to the shader

  this->m_texture->use();
  glDrawArrays(GL_TRIANGLES, 0, 6);

  glDepthFunc(GL_LESS); // Set depth function back to default

  glBindVertexArray(0);
}
