#pragma once

#include "calgine/core/behaviour.h"
#include "calgine/core/physics/physics_types.h"
#include "calgine/core/physics/physics_world.h"
#include "calgine/core/physics/collision_event.h"
#include "calgine_api.h"
#include "calgine_pch.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Calgine {

/**
 * @brief Gives a GameObject a body in the PhysicsWorld.
 *
 * @details
 * The body is created in start_tick() from the GameObject's *world* transform. The collider size is
 * multiplied by the world scale at that moment.
 *
 * - dynamic: physics writes the result of every step back into the GameObject's transform.
 *   Changing the Transform yourself has no effect on the body; use teleport() or velocities/forces.
 * - kinematic: the body follows the GameObject's world transform every step. Move the GameObject
 *   (or its parent) and the body sweeps there, pushing dynamic bodies out of the way.
 * - static: never moves. Use teleport() if you really need to move it.
 *
 * Disabling the GameObject (set_active(false)) removes the body from the simulation until re-enabled.
 *
 * @code{.cpp}
 * RigidBodySettings settings;
 * settings.shape = ColliderShape::make_box(glm::vec3(1.0f));
 * game_object.add_behaviour<RigidBody>(settings);
 * @endcode
 */
class CALGINE_API RigidBody : public Behaviour
{
public:
  /** @brief A dynamic 1x1x1 box. */
  RigidBody();
  explicit RigidBody(const RigidBodySettings& settings);
  ~RigidBody() override;

  const RigidBodySettings& get_settings() const { return settings; }
  MotionType get_motion_type() const { return settings.motion; }

  /** @brief False before start_tick(), after destruction, or if body creation failed. */
  bool is_valid() const;

  /** @brief Continuous force in Newtons, applied during the next step. Call every fixed_update for a sustained push. */
  void add_force(const glm::vec3& force);
  /** @brief Instant change in momentum (kg m/s). */
  void add_impulse(const glm::vec3& impulse);
  void add_torque(const glm::vec3& torque);

  void set_linear_velocity(const glm::vec3& velocity);
  glm::vec3 get_linear_velocity() const;
  void set_angular_velocity(const glm::vec3& velocity);
  glm::vec3 get_angular_velocity() const;

  /** @brief Instantly moves the body (and the GameObject) to a world position and rotation. */
  void teleport(const glm::vec3& world_position, const glm::quat& world_rotation);
  /** @brief Same as above, with the rotation given as Euler angles in degrees. */
  void teleport(const glm::vec3& world_position, const glm::vec3& world_rotation_degrees);

  /** @brief All CollisionEvents this frame that involve this GameObject. */
  std::vector<const CollisionEvent*> get_collisions(const EventContext& event_context) const;

private:
  RigidBodySettings settings;

  static constexpr uint32_t invalid_body_id = 0xffffffff;
  uint32_t body_id = invalid_body_id;
  bool in_simulation = false;

  void start_tick() override;
  void on_destroy() override;

  static PhysicsWorld::Impl* world_impl();

  void create_body();
  void destroy_body();

  // Called by PhysicsWorld around each step.
  void pre_step(float delta_time);
  void post_step();

  friend class PhysicsWorld;
};

}
