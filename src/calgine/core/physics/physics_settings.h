#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <glm/glm.hpp>
#include "calgine_api.h"

namespace Calgine {

/** @brief Index of a collision layer, 0 to max_collision_layers - 1. */
using CollisionLayer = uint8_t;
/** @brief One bit per collision layer. */
using CollisionLayerMask = uint16_t;

constexpr uint32_t max_collision_layers = 16;
constexpr CollisionLayer default_collision_layer = 0;
constexpr CollisionLayerMask all_collision_layers = 0xFFFF;

/** @brief Mask with only @p layer set. */
constexpr CollisionLayerMask collision_layer_bit(CollisionLayer layer) { return static_cast<CollisionLayerMask>(1u << layer); }

/**
 * @brief Named collision layers and which pairs of layers collide.
 *
 * @details
 * Every RigidBody belongs to one layer (RigidBodySettings::layer). Two bodies only collide if their
 * layers are set to collide with each other. By default there is one named layer, "default", and every
 * layer collides with every other layer.
 *
 * @code{.cpp}
 * auto& layers = app.settings.physics.layers;
 * layers.set_name(1, "ghosts");
 * layers.set_collides(1, 1, false);   // ghosts pass through each other
 * @endcode
 */
struct CALGINE_API CollisionLayerSettings
{
  std::array<std::string, max_collision_layers> names { "default" };
  std::array<CollisionLayerMask, max_collision_layers> collides_with = make_all_collide();

  void set_name(CollisionLayer layer, const std::string& name)
  {
    if (layer < max_collision_layers)
      names[layer] = name;
  }

  /** @brief Sets whether layers @p a and @p b collide (symmetric). */
  void set_collides(CollisionLayer a, CollisionLayer b, bool collide)
  {
    if (a >= max_collision_layers || b >= max_collision_layers)
      return;

    if (collide)
    {
      collides_with[a] |= collision_layer_bit(b);
      collides_with[b] |= collision_layer_bit(a);
    }
    else
    {
      collides_with[a] &= static_cast<CollisionLayerMask>(~collision_layer_bit(b));
      collides_with[b] &= static_cast<CollisionLayerMask>(~collision_layer_bit(a));
    }
  }

  bool collides(CollisionLayer a, CollisionLayer b) const
  {
    if (a >= max_collision_layers || b >= max_collision_layers)
      return false;
    return (collides_with[a] & collision_layer_bit(b)) != 0;
  }

  /** @brief Layer index for a name, if any layer has it. */
  std::optional<CollisionLayer> find(const std::string& name) const
  {
    for (uint32_t i = 0; i < max_collision_layers; i++)
      if (!name.empty() && names[i] == name)
        return static_cast<CollisionLayer>(i);
    return std::nullopt;
  }

private:
  static std::array<CollisionLayerMask, max_collision_layers> make_all_collide()
  {
    std::array<CollisionLayerMask, max_collision_layers> masks {};
    masks.fill(all_collision_layers);
    return masks;
  }
};

/**
 * @brief Physics configuration, set through app.settings.physics before App::start_systems().
 *
 * @code{.cpp}
 * app.settings.physics.gravity = glm::vec3(0.0f, -20.0f, 0.0f);
 * app.settings.physics.max_bodies = 100000;
 * app.start_systems();
 * @endcode
 */
struct CALGINE_API PhysicsSettings
{
  glm::vec3 gravity = glm::vec3(0.0f, -9.81f, 0.0f);

  // ---- Capacity ----
  // When these run out Jolt drops contacts (and logs a warning); raise them for very large scenes.

  /** @brief Maximum number of bodies that can exist at once. */
  uint32_t max_bodies = 65536;
  /** @brief Maximum number of body pairs whose bounding boxes overlap at once. */
  uint32_t max_body_pairs = 65536;
  /** @brief Maximum number of contact constraints (roughly touching pairs x contact points) per step. */
  uint32_t max_contact_constraints = 65536;
  /** @brief Scratch memory preallocated for each step, in megabytes. Falls back to malloc when exceeded. */
  uint32_t temp_allocator_mb = 10;

  // ---- Simulation ----

  /** @brief Physics worker threads. -1 means one per CPU core, minus one. */
  int worker_threads = -1;
  /** @brief Collision sub-steps per fixed step. Raise for fast or very stiff setups (costs proportionally more). */
  int collision_steps = 1;

  // ---- Rendering / diagnostics ----

  /**
   * @brief Draw moving bodies at a pose blended between the last two physics steps.
   * Makes motion smooth at any frame rate, at the cost of drawing up to one step (1/60 s) behind.
   * Individual bodies can opt out with RigidBodySettings::interpolate.
   */
  bool interpolation = true;

  /** @brief Log a one-time warning when a kinematic body is moved outside fixed_update_tick() (its collider will trail it). */
  bool warn_kinematic_moved_outside_fixed_update = true;

  CollisionLayerSettings layers;
};

}
