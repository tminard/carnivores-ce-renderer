#pragma once

#include <memory>

class CEWorld;

class CEScene
{
private:
  std::unique_ptr<CEWorld> m_world;

public:
  void load();
  CEWorld* world() const;
};
