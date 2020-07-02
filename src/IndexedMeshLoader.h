/*
 Convert a C2 mesh to an indexed mesh using face and vertice data
*/

#ifndef __CE_Character_Lab__IndexedMeshLoader__
#define __CE_Character_Lab__IndexedMeshLoader__

#include <stdio.h>
#include <memory>
#include <vector>
#include <cstdint>

#include "g_shared.h"
#include "vertex.h"

class IndexedMeshLoader
{
private:
  std::vector<TPoint3d> m_vertices_data;
  std::vector<TFace> m_faces_data;
  
  std::vector<unsigned int> m_indices;
  std::vector<Vertex> m_vertices;
public:
  IndexedMeshLoader(std::vector<TPoint3d> vertices, std::vector<TFace> faces);
  ~IndexedMeshLoader();

  std::vector<Vertex> getVertices();
  std::vector<unsigned int> getIndices();
};

#endif /* defined(__CE_Character_Lab__IndexedMeshLoader__) */
