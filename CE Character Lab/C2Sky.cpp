//
//  C2Sky.cpp
//  CE Character Lab
//
//  Created by Tyler Minard on 8/18/15.
//  Copyright (c) 2015 Tyler Minard. All rights reserved.
//

#include "C2Sky.h"

#include "CETexture.h"
#include "shader.h"
#include "transform.h"
#include "camera.h"

#include "tga.h"

C2Sky::C2Sky(std::ifstream& instream)
{
    std::vector<uint16_t> raw_sky_texture_data;
    raw_sky_texture_data.resize(256*256);
    instream.read(reinterpret_cast<char *>(raw_sky_texture_data.data()), 256*256*sizeof(uint16_t));
    
    this->m_texture = std::unique_ptr<CETexture>(new CETexture(raw_sky_texture_data, 256*256, 256, 256));
    this->m_shader = std::unique_ptr<NewShader>(new NewShader("sky.vs", "sky.fs"));
    
    this->loadIntoHardwareMemory();
}

C2Sky::C2Sky(std::unique_ptr<CETexture> sky_texture)
: m_texture(std::move(sky_texture))
{
    this->m_shader = std::unique_ptr<NewShader>(new NewShader("sky.vs", "sky.fs"));
    this->loadIntoHardwareMemory();
}

C2Sky::~C2Sky()
{
}

void C2Sky::saveTextureAsBMP(const std::string &file_name)
{
    this->m_texture->saveToBMPFile(file_name);
}

void C2Sky::loadTextureIntoMemory()
{
    // Have to do something special for our texture as we want it as a cube
    // TODO: refactor CETexture to handle different types
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
    std::vector<std::string> faces = {
        "day_cloudtop/cloudtop_rt.tga",
        "day_cloudtop/cloudtop_lf.tga",
        "day_cloudtop/cloudtop_up.tga",
        "day_cloudtop/cloudtop_dn.tga",
        "day_cloudtop/cloudtop_bk.tga",
        "day_cloudtop/cloudtop_ft.tga"
    };
    
    for (int i = 0; i < faces.size(); i++) {
       // if (i == 22) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB5_A1, 256, 256, 0, GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, this->m_texture->getRawData()->data());
//        } else {
//            std::unique_ptr<Tga> mtga = std::unique_ptr<Tga>(new Tga(faces[i].data()));
//            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, mtga->HasAlphaChannel() ? GL_RGBA : GL_RGB, mtga->GetWidth(), mtga->GetWidth(), 0, mtga->HasAlphaChannel() ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, mtga->GetPixels().data());
//        }
    }
    
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    
    this->m_cube_texture = textureID;
    
    glBindTexture(GL_TEXTURE_2D, 0);
}

// Build objects
void C2Sky::loadIntoHardwareMemory()
{
    float skyboxVertices[] = {
        // positions
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
    
    glBindVertexArray(0);
}

void C2Sky::Render(GLFWwindow* window, Camera& camera)
{
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glm::mat4 view = glm::mat4(glm::mat3(camera.GetVM()));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width/(float)height, 0.1f, 100000.f);

    this->m_shader->use();
    this->m_shader->setMat4("view", view);
    this->m_shader->setMat4("projection", projection);
    
    glBindVertexArray(this->m_vertex_array_object);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, this->m_cube_texture);
    
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}
