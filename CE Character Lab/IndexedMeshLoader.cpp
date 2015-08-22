//
//  IndexedMeshLoader.cpp
//  CE Character Lab
//
//  Created by Tyler Minard on 8/19/15.
//  Copyright (c) 2015 Tyler Minard. All rights reserved.
//

#include "vertex.h"
#include "IndexedMeshLoader.h"

#include <map>

IndexedMeshLoader::~IndexedMeshLoader()
{
  
}

IndexedMeshLoader::IndexedMeshLoader(std::vector<TPoint3d> vertices, std::vector<TFace> faces)
: m_vertices_data(vertices), m_faces_data(faces)
{
  this->m_vertices.clear();
  this->m_indices.clear();
  unsigned int cur_index = 0;

  for (int f = 0; f < (int)m_faces_data.size(); f++) {
    Vertex v1(
                                                                    glm::vec3(m_vertices_data.at(m_faces_data.at(f).v1).x, m_vertices_data.at(m_faces_data.at(f).v1).y, m_vertices_data.at(m_faces_data.at(f).v1).z),
                                                                    glm::vec2((float)((float)m_faces_data.at(f).tax/256.f), (float)((float)m_faces_data.at(f).tay/256.f)),
                                                                    glm::vec3(0,0,0), m_vertices_data.at(m_faces_data.at(f).v1).hide == 1, m_vertices_data.at(m_faces_data.at(f).v1).owner
                                                                    );
    Vertex v2(
                                                                    glm::vec3(m_vertices_data.at(m_faces_data.at(f).v2).x, m_vertices_data.at(m_faces_data.at(f).v2).y, m_vertices_data.at(m_faces_data.at(f).v2).z),
                                                                    glm::vec2((float)((float)m_faces_data.at(f).tbx/256.f), (float)((float)m_faces_data.at(f).tby/256.f)),
                                                                    glm::vec3(0,0,0), m_vertices_data.at(m_faces_data.at(f).v2).hide == 1, m_vertices_data.at(m_faces_data.at(f).v2).owner
                                                                    );
    Vertex v3(
                                                                    glm::vec3(m_vertices_data.at(m_faces_data.at(f).v3).x, m_vertices_data.at(m_faces_data.at(f).v3).y, m_vertices_data.at(m_faces_data.at(f).v3).z),
                                                                    glm::vec2((float)((float)m_faces_data.at(f).tcx/256.f), (float)((float)m_faces_data.at(f).tcy/256.f)),
                                                                    glm::vec3(0,0,0), m_vertices_data.at(m_faces_data.at(f).v3).hide == 1, m_vertices_data.at(m_faces_data.at(f).v3).owner
                                                                    );
    
    m_vertices.push_back(v1);
    m_indices.push_back(cur_index++);
    
    m_vertices.push_back(v2);
    m_indices.push_back(cur_index++);
    
    m_vertices.push_back(v3);
    m_indices.push_back(cur_index++);

  }
}

/*
 
 // load it!
 std::map<std::unique_ptr<Vertex>, unsigned int> mapped_vertice_data;
 std::map<std::unique_ptr<Vertex>, unsigned int>::iterator vertex_data_it;
 this->m_indices.resize(m_faces_data.size()*3);
 this->m_indices.clear();
 
 unsigned int cur_index = 0;
 for (int f = 0; f < (int)m_faces_data.size(); f++) {
 std::unique_ptr<Vertex> v1 = std::unique_ptr<Vertex>(new Vertex(
 glm::vec3(m_vertices_data.at(m_faces_data.at(f).v1).x, m_vertices_data.at(m_faces_data.at(f).v1).y, m_vertices_data.at(m_faces_data.at(f).v1).z),
 glm::vec2(m_faces_data.at(f).tax, m_faces_data.at(f).tay),
 glm::vec3(0,0,0), m_vertices_data.at(m_faces_data.at(f).v1).hide == 1, m_vertices_data.at(m_faces_data.at(f).v1).owner
 ));
 std::unique_ptr<Vertex> v2 = std::unique_ptr<Vertex>(new Vertex(
 glm::vec3(m_vertices_data.at(m_faces_data.at(f).v2).x, m_vertices_data.at(m_faces_data.at(f).v2).y, m_vertices_data.at(m_faces_data.at(f).v2).z),
 glm::vec2(m_faces_data.at(f).tbx, m_faces_data.at(f).tby),
 glm::vec3(0,0,0), m_vertices_data.at(m_faces_data.at(f).v2).hide == 1, m_vertices_data.at(m_faces_data.at(f).v2).owner
 ));
 std::unique_ptr<Vertex> v3 = std::unique_ptr<Vertex>(new Vertex(
 glm::vec3(m_vertices_data.at(m_faces_data.at(f).v3).x, m_vertices_data.at(m_faces_data.at(f).v3).y, m_vertices_data.at(m_faces_data.at(f).v3).z),
 glm::vec2(m_faces_data.at(f).tcx, m_faces_data.at(f).tcy),
 glm::vec3(0,0,0), m_vertices_data.at(m_faces_data.at(f).v3).hide == 1, m_vertices_data.at(m_faces_data.at(f).v3).owner
 ));
 
 mapped_vertice_data.insert(std::make_pair(std::move(v1), cur_index++));
 m_indices.push_back(cur_index);
 
 mapped_vertice_data.insert(std::make_pair(std::move(v2), cur_index++));
 m_indices.push_back(cur_index);
 
 mapped_vertice_data.insert(std::make_pair(std::move(v3), cur_index++));
 m_indices.push_back(cur_index);
 
 
 vertex_data_it = mapped_vertice_data.find(v1);
 if (vertex_data_it == mapped_vertice_data.end()) {
 // add it and return index (size)
 mapped_vertice_data.insert(std::make_pair(std::move(v1), cur_index++));
 m_indices.push_back(cur_index);
 } else {
 // return location
 m_indices.push_back(vertex_data_it->second);
 }
 
 vertex_data_it = mapped_vertice_data.find(v2);
 if (vertex_data_it == mapped_vertice_data.end()) {
 // add it and return index (size)
 mapped_vertice_data.insert(std::make_pair(std::move(v2), cur_index++));
 m_indices.push_back(cur_index);
 } else {
 // return location
 m_indices.push_back(vertex_data_it->second);
 }
 
 vertex_data_it = mapped_vertice_data.find(v3);
 if (vertex_data_it == mapped_vertice_data.end()) {
 // add it and return index (size)
 mapped_vertice_data.insert(std::make_pair(std::move(v3), cur_index++));
 m_indices.push_back(cur_index);
 } else {
 // return location
 m_indices.push_back(vertex_data_it->second);
 }
 
}

// convert it to a more managable source
for (vertex_data_it = mapped_vertice_data.begin(); vertex_data_it != mapped_vertice_data.end(); vertex_data_it++) {
  this->m_vertices.push_back(*vertex_data_it->first);
}

*/

std::vector<Vertex> IndexedMeshLoader::getVertices()
{
  return this->m_vertices;
}

std::vector<unsigned int> IndexedMeshLoader::getIndices()
{
  return this->m_indices;
}

