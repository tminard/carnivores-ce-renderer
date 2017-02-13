//
//  CEGeneralVertexBuffer.h
//  CE Character Lab
//
//  Created by Tyler Minard on 2/12/17.
//  Copyright © 2017 Tyler Minard. All rights reserved.
//

#pragma once

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>

#include <vector>

class Vertex;

class CEVertexBuffer {
protected:
  enum {
    VERTEX_VB=0,
    INDEX_VB=1,
    NUM_BUFFERS=2
  };

  GLuint m_vertexArrayObject;
  GLuint m_vertexArrayBuffers[NUM_BUFFERS];

  int m_indice_data_size;
  bool m_uploaded_to_gpu;

public:
  CEVertexBuffer(const Vertex* vertex_data, int vertex_data_size, const unsigned int* indice_data, int indice_data_size);
  CEVertexBuffer();
  ~CEVertexBuffer();

  void Draw();
  virtual void uploadToGPU(const Vertex* vertex_data, int vertex_data_size, const unsigned int* indice_data, int indice_data_size);
};
