#pragma once

#include <stdio.h>
#include <array>

#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

using namespace glm;
using namespace std;

struct CETerrainVertex {
  vec3 m_position;
  vec3 m_normal;
  vec4 m_texture_coords;

  CETerrainVertex(const glm::vec3& position, const glm::vec4& texture_coords, const glm::vec3& normal);
};

CETerrainVertex::CETerrainVertex(const glm::vec3& position, const glm::vec4& texture_coords, const glm::vec3& normal) : m_position(position), m_normal(normal), m_texture_coords(texture_coords)
{
}
