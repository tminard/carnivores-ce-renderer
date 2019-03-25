//
//  vertex.cpp
//  CE Character Lab
//
//  Created by Tyler Minard on 8/18/15.
//  Copyright (c) 2015 Tyler Minard. All rights reserved.
//

#include "vertex.h"

Vertex::Vertex(const glm::vec3& position, const glm::vec2& texture_coord, const glm::vec3& normal, bool hidden, float alpha, uint32_t owner, uint32_t flags)
: m_position(position), m_uv_coord(texture_coord), m_normal(normal), m_hide(hidden), m_owner(owner), m_flags(flags), m_alpha(alpha)
{
  if (m_flags & 4) {
    this->m_alpha = 0.f;
  }
}

std::string Vertex::getHash()
{
  return "VERTEX-" + std::to_string(m_position.x) + "-" + std::to_string(m_position.y) + "-"+ std::to_string(m_position.z) + "-"
    + std::to_string(m_uv_coord.x) + "-"+ std::to_string(m_uv_coord.y) + "-"
    + std::to_string(m_normal.x) + "-"+ std::to_string(m_normal.y) + "-"+ std::to_string(m_normal.z) + "-";
}

bool operator== (Vertex &v1, Vertex &v2)
{
  // pos, tex, and normals should be same. Ignoring hide and owner attributes for now.
  glm::vec3 p1, p2, n1, n2;
  glm::vec2 uv1, uv2;
  p1 = v1.getPos();
  p2 = v2.getPos();
  n1 = v1.getNormal();
  n2 = v2.getNormal();
  uv1 = v1.getUVPos();
  uv2 = v2.getUVPos();

  if (
        p1.x == p2.x &&
        p1.y == p2.y &&
        p1.z == p2.z &&
      
        n1.x == n2.x &&
        n1.y == n2.y &&
        n1.z == n2.z &&
      
        uv1.x == uv2.x &&
        uv1.y == uv2.y
    ) {
    return true;
  } else {
    return false;
  }
}

bool operator!= (Vertex &v1, Vertex &v2)
{
  return !(v1 == v2);
}

glm::vec3 Vertex::getPos() {
  return this->m_position;
}

glm::vec2 Vertex::getUVPos() {
  return this->m_uv_coord;
}

glm::vec3 Vertex::getNormal() {
  return this->m_normal;
}

bool Vertex::isHidden()
{
  return this->m_hide;
}

uint32_t Vertex::getOwner()
{
  return this->m_owner;
}
