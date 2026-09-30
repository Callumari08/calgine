#pragma once

#include <glm/glm.hpp>
#include "calgine_api.h"

namespace Calgine {

class GameObject;

/**
 * @brief How a RigidBody moves.
 */
enum class MotionType
{
  /** @brief Never moves (floors, walls). */
  static_body,
  /** @brief Moved by code through its GameObject's Transform; pushes dynamic bodies but isn't pushed back. */
  kinematic,
  /** @brief Fully simulated: gravity, forces and collisions move it. Physics owns its Transform. */
  dynamic,
};

/**
 * @brief Collision shape of a RigidBody, in the GameObject's local units (multiplied by world scale).
 */
struct CALGINE_API ColliderShape
{
  enum Type
  {
    box,
    sphere,
    capsule,
  };

  Type type = box;

  /** @brief Box only: half the size along each axis. */
  glm::vec3 half_extents = glm::vec3(0.5f);

  /** @brief Sphere and capsule radius. */
  float radius = 0.5f;

  /** @brief Capsule only: half the length of the cylinder part, along the local Y axis. */
  float half_height = 0.5f;

  static ColliderShape make_box(const glm::vec3& half_extents)
  {
    ColliderShape shape;
    shape.type = box;
    shape.half_extents = half_extents;
    return shape;
  }

  static ColliderShape make_sphere(float radius)
  {
    ColliderShape shape;
    shape.type = sphere;
    shape.radius = radius;
    return shape;
  }

  static ColliderShape make_capsule(float half_height, float radius)
  {
    ColliderShape shape;
    shape.type = capsule;
    shape.half_height = half_height;
    shape.radius = radius;
    return shape;
  }
};

/**
 * @brief Settings used when a RigidBody creates its physics body.
 */
struct CALGINE_API RigidBodySettings
{
  MotionType motion = MotionType::dynamic;
  ColliderShape shape;

  /** @brief Mass in kg. 0 means "calculate from the shape's volume" (density 1000 kg/m^3). */
  float mass = 0.0f;
  float friction = 0.2f;
  /** @brief Bounciness, 0 = no bounce, 1 = perfectly elastic. */
  float restitution = 0.0f;
  float linear_damping = 0.05f;
  float angular_damping = 0.05f;
  /** @brief Multiplier on world gravity for this body. */
  float gravity_factor = 1.0f;

  /** @brief Sensors report collisions but don't physically block anything (triggers). */
  bool is_sensor = false;
};

/**
 * @brief Result of PhysicsWorld::raycast().
 */
struct CALGINE_API RaycastHit
{
  GameObject* game_object = nullptr;
  glm::vec3 point = glm::vec3(0.0f);
  glm::vec3 normal = glm::vec3(0.0f);
  float distance = 0.0f;
};

}
