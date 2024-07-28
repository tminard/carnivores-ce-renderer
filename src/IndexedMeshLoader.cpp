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
    
    // Structures to store accumulated normals and counts for averaging
    std::vector<glm::vec3> vertexNormals(m_vertices_data.size(), glm::vec3(0.0f));
    std::vector<int> normalCounts(m_vertices_data.size(), 0);

    // First pass: compute face normals and accumulate them for each vertex
    for (int f = 0; f < (int)m_faces_data.size(); f++) {
        glm::vec3 v1_pos = glm::vec3(m_vertices_data.at(m_faces_data.at(f).v1).x, m_vertices_data.at(m_faces_data.at(f).v1).y, m_vertices_data.at(m_faces_data.at(f).v1).z);
        glm::vec3 v2_pos = glm::vec3(m_vertices_data.at(m_faces_data.at(f).v2).x, m_vertices_data.at(m_faces_data.at(f).v2).y, m_vertices_data.at(m_faces_data.at(f).v2).z);
        glm::vec3 v3_pos = glm::vec3(m_vertices_data.at(m_faces_data.at(f).v3).x, m_vertices_data.at(m_faces_data.at(f).v3).y, m_vertices_data.at(m_faces_data.at(f).v3).z);

        glm::vec3 face_normal = calculateFaceNormal(v1_pos, v2_pos, v3_pos);
      
        vertexNormals[m_faces_data.at(f).v1] += face_normal;
        normalCounts[m_faces_data.at(f).v1]++;
        
        vertexNormals[m_faces_data.at(f).v2] += face_normal;
        normalCounts[m_faces_data.at(f).v2]++;
        
        vertexNormals[m_faces_data.at(f).v3] += face_normal;
        normalCounts[m_faces_data.at(f).v3]++;
    }

    // Second pass: normalize the accumulated normals
    for (int i = 0; i < vertexNormals.size(); ++i) {
        vertexNormals[i] = glm::normalize(vertexNormals[i]);
    }

    // Third pass: create vertices with the computed vertex normals
    for (int f = 0; f < (int)m_faces_data.size(); f++) {
        glm::vec3 v1_pos = glm::vec3(m_vertices_data.at(m_faces_data.at(f).v1).x, m_vertices_data.at(m_faces_data.at(f).v1).y, m_vertices_data.at(m_faces_data.at(f).v1).z);
        glm::vec3 v2_pos = glm::vec3(m_vertices_data.at(m_faces_data.at(f).v2).x, m_vertices_data.at(m_faces_data.at(f).v2).y, m_vertices_data.at(m_faces_data.at(f).v2).z);
        glm::vec3 v3_pos = glm::vec3(m_vertices_data.at(m_faces_data.at(f).v3).x, m_vertices_data.at(m_faces_data.at(f).v3).y, m_vertices_data.at(m_faces_data.at(f).v3).z);

        Vertex v1(
                  v1_pos,
                  glm::vec2((float)((float)m_faces_data.at(f).tax / 256.f), (float)((float)m_faces_data.at(f).tay / 256.f)),
                  vertexNormals[m_faces_data.at(f).v1], m_vertices_data.at(m_faces_data.at(f).v1).hide == 1, 1.f,
                  m_vertices_data.at(m_faces_data.at(f).v1).owner,
                  m_faces_data.at(f).Flags
                  );
        Vertex v2(
                  v2_pos,
                  glm::vec2((float)((float)m_faces_data.at(f).tbx / 256.f), (float)((float)m_faces_data.at(f).tby / 256.f)),
                  vertexNormals[m_faces_data.at(f).v2], m_vertices_data.at(m_faces_data.at(f).v2).hide == 1, 1.f,
                  m_vertices_data.at(m_faces_data.at(f).v2).owner,
                  m_faces_data.at(f).Flags
                  );
        Vertex v3(
                  v3_pos,
                  glm::vec2((float)((float)m_faces_data.at(f).tcx / 256.f), (float)((float)m_faces_data.at(f).tcy / 256.f)),
                  vertexNormals[m_faces_data.at(f).v3], m_vertices_data.at(m_faces_data.at(f).v3).hide == 1, 1.f,
                  m_vertices_data.at(m_faces_data.at(f).v3).owner,
                  m_faces_data.at(f).Flags
                  );
        
        m_vertices.push_back(v1);
        m_indices.push_back(cur_index++);
        
        m_vertices.push_back(v2);
        m_indices.push_back(cur_index++);
        
        m_vertices.push_back(v3);
        m_indices.push_back(cur_index++);
    }
}

std::vector<Vertex> IndexedMeshLoader::getVertices()
{
    return this->m_vertices;
}

std::vector<unsigned int> IndexedMeshLoader::getIndices()
{
    return this->m_indices;
}
