/*
 * Responsible for managing the root node of a scene, and coordinating all rendering.
 * Roughly analogous to a root SCNNode.
 */
#pragma once

#include <glm/glm.hpp>

class CEWorldModel;
class CEWorldObject;
class CEWorldState;

class CEWorld
{
private:

public:
  /*
   Register a static model into the library of models
   Essentially becomes managed. Allows us to efficiently manage resources.
   */
  void add(CEWorldModel& model);

  /*
   Place an object in the worldspace
   */
  void place(CEWorldObject& object, glm::vec3 position);

  /*
   Update given pitch, yaw, direction, range, renderer configuration, etc
   */
  void update(CEWorldState& state);

  /*
   Render everything
   */
  void render(double time_delta);
};
