#pragma once

#include "calgine/core/physics/physics_types.h"
#include "calgine_api.h"
#include "calgine_pch.h"
#include <glm/glm.hpp>

namespace Calgine {

class EventContext;
class RigidBody;

/**
 * @brief Timing and counts for a single physics step (see PhysicsWorld::get_last_step_stats()).
 *
 * All times are wall-clock milliseconds measured on the main thread.
 */
struct CALGINE_API PhysicsStepStats
{
  /** @brief Syncing enabled state and kinematic targets from GameObjects into Jolt. */
  float pre_step_ms = 0.0f;
  /** @brief Jolt's simulation (PhysicsSystem::Update, runs on the job threads). */
  float simulate_ms = 0.0f;
  /** @brief Writing dynamic body poses back into GameObject transforms. */
  float post_step_ms = 0.0f;
  /** @brief Turning Jolt contacts into CollisionEvents. */
  float events_ms = 0.0f;
  /** @brief Whole step, including the above. */
  float total_ms = 0.0f;

  uint32_t bodies = 0;
  uint32_t active_bodies = 0;
  uint32_t collision_events = 0;
};

/**
 * @brief The engine's physics simulation (backed by Jolt Physics).
 *
 * @details
 * The App owns the lifecycle: the world is created in App::systems_init(), stepped once per fixed
 * timestep right after every Behaviour's fixed_update_tick(), and shut down when the App is destroyed.
 *
 * Add a RigidBody behaviour to a GameObject to give it a body in this world. Collisions are reported
 * through the EventContext as CollisionEvent (see collision_event.h).
 */
class CALGINE_API PhysicsWorld
{
public:
  static PhysicsWorld& get_instance();

  PhysicsWorld(const PhysicsWorld&) = delete;
  PhysicsWorld& operator=(const PhysicsWorld&) = delete;

  bool is_initialized() const { return impl != nullptr; }

  void set_gravity(const glm::vec3& gravity);
  glm::vec3 get_gravity() const;

  /**
   * @brief Casts a ray against every body in the world and returns the closest hit.
   *
   * @param origin World-space start of the ray.
   * @param direction Direction of the ray (does not need to be normalized).
   * @param max_distance How far the ray travels.
   */
  std::optional<RaycastHit> raycast(const glm::vec3& origin, const glm::vec3& direction, float max_distance) const;

  /** @brief When enabled, collider wireframes are drawn every frame. */
  void set_debug_draw_enabled(bool enabled) { debug_draw_enabled = enabled; }
  bool is_debug_draw_enabled() const { return debug_draw_enabled; }

  /** @brief Number of RigidBodies currently registered with the world. */
  size_t get_body_count() const;

  /** @brief Stats for the most recent physics step. */
  const PhysicsStepStats& get_last_step_stats() const { return last_step_stats; }

  /** @brief Number of steps simulated since init(). */
  uint64_t get_total_steps() const { return total_steps; }

  /**
   * @brief Called on the main thread after every physics step with that step's stats.
   *
   * @details A frame can run zero or several steps, so use this rather than polling
   * get_last_step_stats() when every step needs to be recorded (e.g. benchmarks). Pass nullptr to clear.
   */
  void set_step_stats_callback(std::function<void(const PhysicsStepStats&)> callback) { step_stats_callback = std::move(callback); }

  struct Impl;

private:
  PhysicsWorld();
  ~PhysicsWorld();

  void init();
  void step(float delta_time, EventContext& event_context);
  void debug_draw();
  void shutdown();

  Impl* get_impl() const { return impl.get(); }

  std::unique_ptr<Impl> impl;
  bool debug_draw_enabled = false;

  PhysicsStepStats last_step_stats;
  uint64_t total_steps = 0;
  std::function<void(const PhysicsStepStats&)> step_stats_callback;

  friend class App;
  friend class RigidBody;
};

}
