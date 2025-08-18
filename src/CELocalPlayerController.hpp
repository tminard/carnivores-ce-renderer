#pragma once

#include <stdio.h>
#include <memory>

#include <glm/vec3.hpp>
#include "CEObservable.hpp"
#include "CEBasePlayerController.hpp"

class C2MapFile;
class C2MapRscFile;
class ICapsuleCollision;

class CELocalPlayerController : public CEBasePlayerController, public CEObservable
{
private:
  float m_world_width;
  float m_world_height;
  float m_tile_size;
  
  float m_walk_speed;
  float m_player_height;
  
  float m_current_speed;
  float m_target_speed;
  float m_acceleration;
  float m_deceleration;
  
  float m_bobble_speed;
  float m_bobble_amount;
  float m_bobble_time;

  float computeSlope(float x, float z);
  glm::vec3 computeSlidingDirection(float x, float z);
  float getPredictiveHeight(const glm::vec3& currentPos, const glm::vec3& movementDir, float speed, float deltaTime);
  float getAdaptiveSmoothingFactor(float heightDifference, float speed, float slopeAngle);
  
  const float maxSlopeAngle = 45.0f;
  float m_previousHeight = 0.0f;
  
  // Predictive height sampling parameters
  float m_predictiveDistance = 1.5f; // Distance ahead to sample (in tiles)
  
  // Adaptive smoothing parameters
  float m_baseSmoothingFactor = 0.15f;
  float m_minSmoothingFactor = 0.05f;
  float m_maxSmoothingFactor = 0.25f;
  
  bool m_is_jumping = false;
  float m_vertical_speed = 0.0f;
  const float m_jump_speed = 12.f;
  const float m_gravity = 48.f;
  double m_last_jump_time = 0.0;
  const double m_jump_cooldown = 0.15; // Cooldown period in seconds
  double m_died_at = 0.0;
  bool m_dead = false;
  glm::vec3 m_body_at;
  
  // Optional capsule collision component for world object collision
  std::unique_ptr<ICapsuleCollision> m_capsuleCollision;

public:
  CELocalPlayerController(float world_width, float world_height, float tile_size, std::shared_ptr<C2MapFile> map, std::shared_ptr<C2MapRscFile> rsc);
  
  // Constructor with capsule collision component
  CELocalPlayerController(float world_width, float world_height, float tile_size, std::shared_ptr<C2MapFile> map, std::shared_ptr<C2MapRscFile> rsc, std::unique_ptr<ICapsuleCollision> capsuleCollision);
  
  // Explicit destructor to handle unique_ptr with incomplete type
  ~CELocalPlayerController();

  // CEBasePlayerController interface implementation
  glm::vec3 getPosition() const override;
  glm::vec2 getWorldPosition() const override;
  void setPosition(glm::vec3 position) override;
  void setElevation(float elevation) override;
  void lookAt(glm::vec3 direction) override;
  Camera* getCamera() override;
  void update(double currentTime, double deltaTime) override;
  std::string getControllerType() const override { return "local_player"; }
  
  void kill(double killedAt);
  void panAroundBody(double currentTime);

  // Local player specific methods

  void moveForward(double timeDelta);
  void moveBackward(double timeDelta);
  void strafeRight(double timeDelta);
  void strafeLeft(double timeDelta);
  
  void move(double currentTime, double deltaTime, bool forwardPressed, bool backwardPressed, bool rightPressed, bool leftPressed);
  
  void jump(double currentTime);

  void DBG_printLocationInformation() const;
  
  bool isAlive(double currentTime);
  
  // Capsule collision management
  
  /**
   * Set or replace the capsule collision component.
   * @param capsuleCollision New collision component (can be nullptr to disable)
   */
  void setCapsuleCollision(std::unique_ptr<ICapsuleCollision> capsuleCollision);
  
  /**
   * Get the current capsule collision component.
   * @return Pointer to collision component (can be nullptr if not set)
   */
  ICapsuleCollision* getCapsuleCollision() const;
  
  /**
   * Check if capsule collision is enabled and active.
   * @return true if collision detection is active
   */
  bool hasCapsuleCollision() const;
};
