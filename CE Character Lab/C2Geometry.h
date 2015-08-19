/*
* This object is responsible for storing immutable geometry (vertices, indices, etc),
* and textures (eventually "C2Material").
*
* Currently just wraps Carnivores' TModel struct until elements can be broken
* out into modern 3D.
*
* Equivalent to SceneKit's SCNGeometry.
*/
#pragma once

#include <memory>
#include <vector>
#include <cstdint>

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>

#include "g_shared.h"

class Vertex;

class C2Geometry {
private:
  int m_texture_height;

  float m_vertice_light[4][1024]; // not sure how to store this in vector. TODO: actuall calculate this
  
  std::vector<TPoint3d> m_vertices; // TODO: move to new format
  std::vector<TFace> m_faces; //TODO: move indices into faces, or extract the details (tex position mostly)
  
  enum {
    POSITION_VB,
    NUM_BUFFERS
  };
  
  GLuint m_vertexArrayObject;
  GLuint m_vertexArrayBuffer[NUM_BUFFERS];
  std::vector < Vertex > m_p_vertices;

  // Replace all this with reference to C2Texture (or C2Material)
  std::vector<uint16_t> m_raw_texture_data;
  std::vector<WORD> m_texture_data_a; //lpTexture
  std::vector<WORD> m_texture_data_b; //lpTextur2
  std::vector<WORD> m_texture_data_c; //lpTextur3

  TModel* m_old_model_data;

  void _generate_textures(std::vector<WORD> texture_data, int texture_height);
  void _generate_old_model_data();

public:
  C2Geometry(std::vector<TPoint3d> vertex_data, std::vector<TFace> face_data, std::vector<WORD> texture_data, int texture_height);
  ~C2Geometry();

  std::vector<TPoint3d> getVertices();
  void setVertices(std::vector<TPoint3d> v);
  void _load_gl_object(); // generate VAO and VBO

  void syncOldVerticeData(); // make the old Tmodel vertice data synced
  void removeLightInfo();
  TModel* getTModel();
  void saveTextureAsBMP(const std::string& file_name );
  
  void Render();
};

