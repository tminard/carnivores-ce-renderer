//
//  C23DFFile.h
//  CE Character Lab
//
//  Created by Claude Code for 3DF UI element loading
//

#ifndef __CE_Character_Lab__C23DFFile__
#define __CE_Character_Lab__C23DFFile__

#include <stdio.h>
#include <string>
#include <vector>
#include <memory>
#include <fstream>

#include "g_shared.h"

// 3DF file format structures based on wiki specification
struct C23DFVertex {
    float x, y, z;          // Coordinates
    uint16_t ownerBone;     // Owner bone index
    uint16_t hideFlag;      // Hide flag
};

struct C23DFFace {
    uint16_t v1, v2, v3;    // Vertex indices (6 bytes)
    uint8_t u1, uv1, u2, uv2, u3, uv3;  // Texture coordinates as bytes: u1,v1,u2,v2,u3,v3 (6 bytes)
    uint16_t flags;         // Face flags (2 bytes)
    // Remaining face data (64 bytes total)
    uint8_t reserved[50];   // Reserved space: 64 - 6 - 6 - 2 = 50 bytes
};

struct C23DFBone {
    char name[32];          // 32-byte name
    float x, y, z;          // Coordinates
    int16_t parentBone;     // Parent bone index
    uint16_t hideFlag;      // Hide flag
};

struct C23DFHeader {
    uint32_t vertexCount;   // Number of vertices
    uint32_t faceCount;     // Number of faces
    uint32_t boneCount;     // Number of bones
    uint32_t textureSize;   // Texture length in bytes
};

class C23DFFile
{
private:
    C23DFHeader m_header;
    std::vector<C23DFVertex> m_vertices;
    std::vector<C23DFFace> m_faces;
    std::vector<C23DFBone> m_bones;
    std::vector<uint8_t> m_textureData;
    
    void load(const std::string& filename);
    
public:
    C23DFFile(const std::string& filename);
    ~C23DFFile();
    
    // Accessors for raw 3DF data
    const C23DFHeader& getHeader() const { return m_header; }
    const std::vector<C23DFVertex>& getVertices() const { return m_vertices; }
    const std::vector<C23DFFace>& getFaces() const { return m_faces; }
    const std::vector<C23DFBone>& getBones() const { return m_bones; }
    const std::vector<uint8_t>& getTextureData() const { return m_textureData; }
};

#endif /* defined(__CE_Character_Lab__C23DFFile__) */