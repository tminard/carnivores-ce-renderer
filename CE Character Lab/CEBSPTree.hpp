/*
 * A MRBSP implementation derived from http://graphicsinterface.org/wp-content/uploads/gi1997-10.pdf
 *
 * Build a multi-resolution BSP tree from a given resource source.
 * This will construct multiple sub-trees of decreasing level-of-detail (LOD).
 *
 * Calling `Load` will recursively generate a BSP tree with multiple levels of detail. Touching a node will initiate production of the sub-trees, which will be
 * pre-loaded into hardware memory.
 *
 * Calling `Draw` with a given position will quickly walk the tree, collect the underlying geometries for the given
 * section of terrain (selecting the appropriate LOD based on the distance from view) up to the view distance.
 * The produced collection will then be merged, and inserted into the pipeline for rendering.
 *
 * Note that this tree will dynamically generate required nodes for each requested LOD; a separate service
 * should handle correct timing of initialization so that CPU -> GPU operations will occur appropriately to correctly time CPU and memory usage.
 *
 * TODO: Add LOD (add "lesser" and "greater", and make current "mid"
 * TODO: Make C2*File pointers a `GameResource` path, and `checkout()` the resource while in use; dynamically unload nodes while "far away" from player to continually free resources
 */

#ifndef CEBSPTree_hpp
#define CEBSPTree_hpp

#include <stdio.h>
#include <OpenGL/gl3.h>
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <map>

class C2MapFile;
class C2MapRscFile;
class Vertex;

class BSPTerrainLeaf
{
private:
    std::vector <Vertex> m_vertices;
    std::vector <unsigned int> m_indices;
    int m_num_indices;
    
    GLuint m_vertex_array_object;
    GLuint m_vertex_array_buffer;
    GLuint m_indices_array_buffer;

    void InitHWMemory();
    
    glm::vec2 calcAtlasUV(int texID, glm::vec2 uv);
    glm::vec3 calcWorldVertex(int tile_x, int tile_y);
    std::array<glm::vec2, 4> calcUVMapForQuad(int x, int y, bool quad_reversed, int rotation_code);
public:
    BSPTerrainLeaf(C2MapFile* c_map_weak, C2MapRscFile* c_rsc_weak, glm::i32vec2 pos, int range);
    ~BSPTerrainLeaf();
};

class CEBSPTreeNode
{
private:
    std::unique_ptr<BSPTerrainLeaf> m_terrain;

    std::unique_ptr<CEBSPTreeNode> m_front;
    std::unique_ptr<CEBSPTreeNode> m_back;
public:
    CEBSPTreeNode(C2MapFile* c_map_weak, C2MapRscFile* c_rsc_weak);
    ~CEBSPTreeNode();
};

class CEBSP
{
private:
    std::unique_ptr<CEBSPTreeNode> m_root;
public:
    CEBSP(C2MapFile* c_map_weak, C2MapRscFile* c_rsc_weak);
    ~CEBSP();
};
#endif /* CEBSPTree_hpp */
