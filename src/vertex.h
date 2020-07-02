#ifndef __CE_Character_Lab__vertex__
#define __CE_Character_Lab__vertex__

#include <stdio.h>
#include <glm/glm.hpp>
#include <string>

class Vertex
{
private:
  /** NOTE: Order matters here **/
  glm::vec3 m_position;
  glm::vec2 m_uv_coord;
  glm::vec3 m_normal;

  float m_alpha;
  uint32_t m_flags;
  uint32_t m_owner = 0; // C2 specific flag ranging 0-8. Used for bounding box
  bool m_hide = false;

public:
  Vertex(const glm::vec3& position, const glm::vec2& texture_coord, const glm::vec3& normal, bool hidden, float alpha, uint32_t owner, uint32_t flags);
  
  friend bool operator== (Vertex &v1, Vertex &v2);
  friend bool operator!= (Vertex &v1, Vertex &v2);
  
  glm::vec3 getPos();
  glm::vec2 getUVPos();
  glm::vec3 getNormal();
  
  bool isHidden();
  uint32_t getOwner();
  std::string getHash();
};

#endif /* defined(__CE_Character_Lab__vertex__) */
