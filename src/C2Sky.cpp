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

#include <cmath>
#include <nlohmann/json.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using json = nlohmann::json;

SkyType C2Sky::readSkyTypeFromConfig()
{
  try {
    std::ifstream configFile("config.json");
    if (!configFile.is_open()) {
      std::cerr << "Warning: Could not open config.json, defaulting to flat sky" << std::endl;
      return SkyType::FLAT;
    }
    
    json config;
    configFile >> config;
    
    // Check if map.sky exists
    if (config.contains("map") && config["map"].contains("sky")) {
      std::string skyType = config["map"]["sky"];
      if (skyType == "dome") {
        return SkyType::DOME;
      } else if (skyType == "flat") {
        return SkyType::FLAT;
      } else {
        std::cerr << "Warning: Unknown sky type '" << skyType << "', defaulting to flat" << std::endl;
        return SkyType::FLAT;
      }
    }
    
    // Default to flat if not specified
    return SkyType::FLAT;
    
  } catch (const std::exception& e) {
    std::cerr << "Warning: Error reading config.json: " << e.what() << ", defaulting to flat sky" << std::endl;
    return SkyType::FLAT;
  }
}

C2Sky::C2Sky(std::ifstream& instream, std::filesystem::path shaderPath)
{
  // Read sky type from config
  m_sky_type = readSkyTypeFromConfig();
  
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
  // Read sky type from config
  m_sky_type = readSkyTypeFromConfig();
  
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
  
  // Clean up dome resources only if they were created
  if (m_sky_type == SkyType::DOME) {
    glDeleteBuffers(1, &this->m_dome_vertex_buffer);
    glDeleteBuffers(1, &this->m_dome_uv_buffer);
    glDeleteVertexArrays(1, &this->m_dome_vertex_array_object);
  }
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

    // Set texture parameters to prevent edge artifacts
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    
    // Generate mipmaps to reduce aliasing
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // Store the texture ID for later use
    this->m_cube_texture = textureID;

    // Unbind the texture
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void C2Sky::updateClouds()
{
  double time = glfwGetTime();
  double time_delta = this->last_cloud_time - time;

  // Fixed cloud UV scaling - use normalized coordinates
  float max_uv = 12.0f; // A bit more tiling (2x original)
  float unit_sec = (max_uv / (240.f * 10.f));
  float start = last_cloud_start + (unit_sec * (float)time_delta);
  
  // Wrap UV coordinates to prevent stretching
  start = fmod(start, max_uv);
  
  float cloudUV[] = {
    start, start,
    start + max_uv, start,
    start + max_uv, start + max_uv,
    start + max_uv, start + max_uv,
    start, start + max_uv,
    start, start
  };

  // UPDATE BUFFER
  glBindBuffer(GL_ARRAY_BUFFER, this->m_cloud_uv_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cloudUV), &cloudUV, GL_STATIC_DRAW);

  last_cloud_time = time;
  last_cloud_start = start;
}

void C2Sky::generateDomeGeometry()
{
  m_dome_vertices.clear();
  m_dome_uvs.clear();
  
  // Generate a hemisphere dome for the clouds
  // We'll create a dome that covers the sky from horizon to zenith
  const float maxElevation = M_PI * 0.5f; // 90 degrees (hemisphere)
  const int rings = DOME_SEGMENTS / 2;    // Vertical divisions
  const int sectors = DOME_SEGMENTS;      // Horizontal divisions
  
  // Generate vertices for each ring and sector
  for (int ring = 0; ring <= rings; ring++) {
    float elevation = (float)ring / rings * maxElevation; // 0 to PI/2
    float y = sin(elevation) * DOME_RADIUS;
    float ringRadius = cos(elevation) * DOME_RADIUS;
    
    for (int sector = 0; sector <= sectors; sector++) {
      float azimuth = (float)sector / sectors * 2.0f * M_PI; // 0 to 2*PI
      float x = cos(azimuth) * ringRadius;
      float z = sin(azimuth) * ringRadius;
      
      // Add vertex position
      m_dome_vertices.push_back(x);
      m_dome_vertices.push_back(y + SKY_HEIGHT); // Offset by sky height
      m_dome_vertices.push_back(z);
      
      // Generate UV coordinates for seamless cloud texture mapping
      float u = (float)sector / sectors;
      float v = (float)ring / rings;
      
      m_dome_uvs.push_back(u);
      m_dome_uvs.push_back(v);
    }
  }
  
  // Now we need to create triangle indices
  // For the dome, we'll create vertices in triangle format directly
  std::vector<float> finalVertices;
  std::vector<float> finalUVs;
  
  for (int ring = 0; ring < rings; ring++) {
    for (int sector = 0; sector < sectors; sector++) {
      // Calculate indices for the current quad
      int current = ring * (sectors + 1) + sector;
      int next = current + sectors + 1;
      
      // First triangle (current, next, current+1)
      int idx1 = current * 3;
      int idx2 = next * 3;
      int idx3 = (current + 1) * 3;
      
      finalVertices.push_back(m_dome_vertices[idx1]);
      finalVertices.push_back(m_dome_vertices[idx1 + 1]);
      finalVertices.push_back(m_dome_vertices[idx1 + 2]);
      
      finalVertices.push_back(m_dome_vertices[idx2]);
      finalVertices.push_back(m_dome_vertices[idx2 + 1]);
      finalVertices.push_back(m_dome_vertices[idx2 + 2]);
      
      finalVertices.push_back(m_dome_vertices[idx3]);
      finalVertices.push_back(m_dome_vertices[idx3 + 1]);
      finalVertices.push_back(m_dome_vertices[idx3 + 2]);
      
      // UV coordinates for first triangle
      int uv_idx1 = current * 2;
      int uv_idx2 = next * 2;
      int uv_idx3 = (current + 1) * 2;
      
      finalUVs.push_back(m_dome_uvs[uv_idx1]);
      finalUVs.push_back(m_dome_uvs[uv_idx1 + 1]);
      
      finalUVs.push_back(m_dome_uvs[uv_idx2]);
      finalUVs.push_back(m_dome_uvs[uv_idx2 + 1]);
      
      finalUVs.push_back(m_dome_uvs[uv_idx3]);
      finalUVs.push_back(m_dome_uvs[uv_idx3 + 1]);
      
      // Second triangle (next, next+1, current+1)
      int idx4 = (next + 1) * 3;
      
      finalVertices.push_back(m_dome_vertices[idx2]);
      finalVertices.push_back(m_dome_vertices[idx2 + 1]);
      finalVertices.push_back(m_dome_vertices[idx2 + 2]);
      
      finalVertices.push_back(m_dome_vertices[idx4]);
      finalVertices.push_back(m_dome_vertices[idx4 + 1]);
      finalVertices.push_back(m_dome_vertices[idx4 + 2]);
      
      finalVertices.push_back(m_dome_vertices[idx3]);
      finalVertices.push_back(m_dome_vertices[idx3 + 1]);
      finalVertices.push_back(m_dome_vertices[idx3 + 2]);
      
      // UV coordinates for second triangle
      int uv_idx4 = (next + 1) * 2;
      
      finalUVs.push_back(m_dome_uvs[uv_idx2]);
      finalUVs.push_back(m_dome_uvs[uv_idx2 + 1]);
      
      finalUVs.push_back(m_dome_uvs[uv_idx4]);
      finalUVs.push_back(m_dome_uvs[uv_idx4 + 1]);
      
      finalUVs.push_back(m_dome_uvs[uv_idx3]);
      finalUVs.push_back(m_dome_uvs[uv_idx3 + 1]);
    }
  }
  
  // Replace the original arrays with the triangulated ones
  m_dome_vertices = finalVertices;
  m_dome_uvs = finalUVs;
  m_dome_vertex_count = finalVertices.size() / 3;
}

  // Build objects
void C2Sky::loadIntoHardwareMemory()
{
  // Generate the dome geometry for clouds only if using dome mode
  if (m_sky_type == SkyType::DOME) {
    generateDomeGeometry();
  }
  
  // Initialize cloud UVs with proper scaling (for the old flat plane - keep for fallback)
  float start = 0.0f;
  float max_uv = 12.0f; // A bit more tiling
  float cloudUV[] = {
    start, start,
    max_uv, start,
    max_uv, max_uv,
    max_uv, max_uv,
    start, max_uv,
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

  // Set up dome geometry for clouds only if using dome mode
  if (m_sky_type == SkyType::DOME) {
    glGenVertexArrays(1, &this->m_dome_vertex_array_object);
    glBindVertexArray(this->m_dome_vertex_array_object);
    
    // Dome vertex buffer
    glGenBuffers(1, &this->m_dome_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, this->m_dome_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, m_dome_vertices.size() * sizeof(float), m_dome_vertices.data(), GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    
    // Dome UV buffer
    glGenBuffers(1, &this->m_dome_uv_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, this->m_dome_uv_buffer);
    glBufferData(GL_ARRAY_BUFFER, m_dome_uvs.size() * sizeof(float), m_dome_uvs.data(), GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);
  }

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

  glm::mat4 projection = glm::perspective(glm::radians(80.0f), (float)width / (float)height, 0.00625f, 6250.f); // Scaled down 16x
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
  
  // Render based on sky type configuration
  if (m_sky_type == SkyType::DOME) {
    // Render the cloud dome
    glBindVertexArray(this->m_dome_vertex_array_object);
    glDrawArrays(GL_TRIANGLES, 0, m_dome_vertex_count);
  } else {
    // Render the flat cloud plane (original behavior)
    glBindVertexArray(this->m_vertex_array_object);
    glDrawArrays(GL_TRIANGLES, 0, 6);
  }

  glDepthFunc(GL_LESS); // Set depth function back to default

  glBindVertexArray(0);
}
