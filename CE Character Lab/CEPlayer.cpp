#include "CEPlayer.hpp"

glm::vec3 CEPlayer::getPosition()
{
  return this->m_position;
}

void CEPlayer::setPosition(glm::vec3 position)
{
  this->m_position = position;
}
