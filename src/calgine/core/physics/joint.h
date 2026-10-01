#pragma once

#include "calgine/core/behaviour.h"
#include "calgine/core/physics/physics_world.h"
#include "calgine_api.h"
#include "calgine_pch.h"
#include <cfloat>
#include <glm/glm.hpp>

namespace Calgine {

class RigidBody;

enum class JointType
{
  /** @brief Locks the two bodies together in their current relative position and rotation. */
  fixed,
  /** @brief Ball-and-socket: the anchor points stay together, rotation is free. */
  point,
  /** @brief Door hinge: rotation only around the axis. Supports limits (degrees) and a motor (degrees/second). */
  hinge,
  /** @brief Rail: movement only along the axis, no rotation. Supports limits (metres) and a motor (metres/second). */
  slider,
  /** @brief Rope or rod: keeps the anchor points between min_distance and max_distance apart. */
  distance,
};

/**
 * @brief Settings used when a Joint connects its bodies.
 *
 * Anchors and axes are local to their GameObject by default, and are converted to world space once,
 * when the joint is created.
 */
struct CALGINE_API JointSettings
{
  JointType type = JointType::point;

  /** @brief The other body. nullptr attaches the joint to the world (a fixed point in space). */
  RigidBody* connected_body = nullptr;

  /** @brief Attachment point on this joint's own body. */
  glm::vec3 anchor = glm::vec3(0.0f);
  bool anchor_is_local = true;

  /** @brief Distance joints only: attachment point on the connected body (local to it, or world space if attached to the world). */
  glm::vec3 connected_anchor = glm::vec3(0.0f);

  /** @brief Hinge rotation axis, or slider direction. */
  glm::vec3 axis = glm::vec3(0.0f, 1.0f, 0.0f);
  bool axis_is_local = true;

  /** @brief Hinge (degrees) or slider (metres) limits, relative to the pose when the joint is created. */
  bool limits_enabled = false;
  float limit_min = 0.0f;
  float limit_max = 0.0f;

  /** @brief Distance joints: allowed distance range. Negative means "the distance when the joint is created". */
  float min_distance = -1.0f;
  float max_distance = -1.0f;

  /** @brief Hinge and slider: drive the joint at a target speed (degrees/s or metres/s). */
  bool motor_enabled = false;
  float motor_target_velocity = 0.0f;
  /** @brief Strongest torque (N m, hinge) or force (N, slider) the motor may use. */
  float motor_max_force = FLT_MAX;
};

/**
 * @brief Connects this GameObject's RigidBody to another RigidBody, or to the world.
 *
 * @details
 * Needs a RigidBody on the same GameObject. The joint is created once both bodies exist (it waits if
 * either hasn't started yet). If the connected body is destroyed, the joint breaks and stays broken.
 *
 * @code{.cpp}
 * JointSettings hinge;
 * hinge.type = JointType::hinge;
 * hinge.connected_body = frame.get_behaviour<RigidBody>();
 * hinge.anchor = glm::vec3(-1.0f, 0.0f, 0.0f);   // door's left edge
 * hinge.axis = glm::vec3(0.0f, 1.0f, 0.0f);
 * door.add_behaviour<Joint>(hinge);
 * @endcode
 */
class CALGINE_API Joint : public Behaviour
{
public:
  Joint();
  explicit Joint(const JointSettings& settings);
  ~Joint() override;

  const JointSettings& get_settings() const { return settings; }

  /** @brief True once the joint exists in the physics world. */
  bool is_active() const;
  /** @brief True if the connected body was destroyed. */
  bool is_broken() const { return broken; }

  /** @brief Temporarily turns the joint off without destroying it. */
  void set_enabled(bool enabled);
  bool is_enabled() const { return enabled; }

  /** @brief Hinge and slider only. */
  void set_motor_enabled(bool enabled);
  /** @brief Hinge (degrees/s) and slider (metres/s) only. */
  void set_motor_target_velocity(float velocity);

  /** @brief Hinge: current angle in degrees. Slider: current position in metres. Otherwise 0. */
  float get_current_value() const;

  RigidBody* get_connected_body() const { return settings.connected_body; }

private:
  JointSettings settings;
  static constexpr uint32_t invalid_joint_id = 0xffffffff;
  uint32_t joint_id = invalid_joint_id;
  bool enabled = true;
  bool broken = false;

  void start_tick() override;
  void on_destroy() override;

  void apply_motor();
  void remove_from_world();

  static PhysicsWorld::Impl* world_impl();

  friend class PhysicsWorld;
  friend struct PhysicsWorld::Impl;
};

}
