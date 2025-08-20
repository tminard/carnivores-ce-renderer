//
//  C23DFFile.cpp
//  CE Character Lab
//
//  Created by Claude Code for 3DF UI element loading
//

#include "C23DFFile.h"
#include <iostream>
#include <stdexcept>
#include <fstream>

C23DFFile::C23DFFile(const std::string& filename)
{
    // Verify structure sizes match expected 3DF format
    static_assert(sizeof(C23DFVertex) == 16, "C23DFVertex should be 16 bytes");
    static_assert(sizeof(C23DFFace) == 64, "C23DFFace should be 64 bytes");
    static_assert(sizeof(C23DFBone) == 48, "C23DFBone should be 48 bytes");
    static_assert(sizeof(C23DFHeader) == 16, "C23DFHeader should be 16 bytes");
    
    std::cout << "Structure sizes: Vertex=" << sizeof(C23DFVertex) 
              << " Face=" << sizeof(C23DFFace) 
              << " Bone=" << sizeof(C23DFBone) 
              << " Header=" << sizeof(C23DFHeader) << std::endl;
    
    load(filename);
}

C23DFFile::~C23DFFile()
{
    // Smart pointers handle cleanup automatically
}

void C23DFFile::load(const std::string& filename)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open 3DF file: " + filename);
    }
    
    std::cout << "Loading 3DF file: " << filename << std::endl;
    
    // Read header (16 bytes)
    file.read(reinterpret_cast<char*>(&m_header), sizeof(C23DFHeader));
    
    std::cout << "3DF Header - Vertices: " << m_header.vertexCount 
              << ", Faces: " << m_header.faceCount 
              << ", Bones: " << m_header.boneCount 
              << ", Texture Size: " << m_header.textureSize << std::endl;
    
    // Read faces (64 bytes each) - manually to match JavaScript exactly
    m_faces.resize(m_header.faceCount);
    for (uint32_t i = 0; i < m_header.faceCount; i++) {
        // Read vertex indices
        file.read(reinterpret_cast<char*>(&m_faces[i].v1), sizeof(uint16_t));
        file.read(reinterpret_cast<char*>(&m_faces[i].v2), sizeof(uint16_t));
        file.read(reinterpret_cast<char*>(&m_faces[i].v3), sizeof(uint16_t));
        
        // Read UV coordinates sequentially as per JavaScript: u1,v1,u2,v2,u3,v3
        file.read(reinterpret_cast<char*>(&m_faces[i].u1), sizeof(uint8_t));
        file.read(reinterpret_cast<char*>(&m_faces[i].uv1), sizeof(uint8_t));
        file.read(reinterpret_cast<char*>(&m_faces[i].u2), sizeof(uint8_t));
        file.read(reinterpret_cast<char*>(&m_faces[i].uv2), sizeof(uint8_t));
        file.read(reinterpret_cast<char*>(&m_faces[i].u3), sizeof(uint8_t));
        file.read(reinterpret_cast<char*>(&m_faces[i].uv3), sizeof(uint8_t));
        
        // Read flags
        file.read(reinterpret_cast<char*>(&m_faces[i].flags), sizeof(uint16_t));
        
        // Skip remaining bytes (64 - 6 - 6 - 2 = 50 bytes)
        file.seekg(50, std::ios::cur);
        
        // Debug first few faces
        if (i < 3) {
            std::cout << "Face " << i << " manual read: v1=" << m_faces[i].v1 
                      << " v2=" << m_faces[i].v2 << " v3=" << m_faces[i].v3
                      << " u1=" << (int)m_faces[i].u1 << " uv1=" << (int)m_faces[i].uv1
                      << " u2=" << (int)m_faces[i].u2 << " uv2=" << (int)m_faces[i].uv2  
                      << " u3=" << (int)m_faces[i].u3 << " uv3=" << (int)m_faces[i].uv3
                      << " flags=" << m_faces[i].flags << std::endl;
        }
    }
    
    // Read vertices (16 bytes each)
    m_vertices.resize(m_header.vertexCount);
    for (uint32_t i = 0; i < m_header.vertexCount; i++) {
        file.read(reinterpret_cast<char*>(&m_vertices[i]), sizeof(C23DFVertex));
        
        // Debug first few vertices
        if (i < 5) {
            std::cout << "Vertex " << i << ": (" << m_vertices[i].x 
                      << ", " << m_vertices[i].y << ", " << m_vertices[i].z 
                      << ") bone=" << m_vertices[i].ownerBone 
                      << " hide=" << m_vertices[i].hideFlag << std::endl;
        }
    }
    
    // Read bones (48 bytes each)
    m_bones.resize(m_header.boneCount);
    for (uint32_t i = 0; i < m_header.boneCount; i++) {
        file.read(reinterpret_cast<char*>(&m_bones[i]), sizeof(C23DFBone));
    }
    
    // Read texture data (BGRA5551 format, 256 pixels wide)
    if (m_header.textureSize > 0) {
        m_textureData.resize(m_header.textureSize);
        file.read(reinterpret_cast<char*>(m_textureData.data()), m_header.textureSize);
        
        // Calculate texture height (width is always 256)
        uint32_t textureHeight = m_header.textureSize / (256 * sizeof(uint16_t));
        
        std::cout << "Loaded 3DF texture: " << 256 << "x" << textureHeight << std::endl;
    }
    
    file.close();
}

