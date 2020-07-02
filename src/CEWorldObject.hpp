/*
 Represents an instance of a model in a world
*/
#pragma once

#include <memory>
#include <glm/glm.hpp>

class CEWorldModel;

class CEWorldObject
{
private:
  std::shared_ptr<CEWorldModel> m_model;

  glm::mat4 m_view_matrix;
  glm::mat4 m_projection_matrix;

public:
};
