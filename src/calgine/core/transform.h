#pragma once

#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "calgine_api.h"

namespace Calgine {

/**
 * @brief Position, rotation (Euler angles in degrees, applied X then Y then Z) and scale.
 *
 * @details
 * A GameObject's Transform is its *local* transform, relative to its parent.
 * Use GameObject::get_world_transform() / get_world_matrix() for world space.
 */
class CALGINE_API Transform
{
public:
  Transform(glm::vec3 position, glm::vec3 rotation = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f));

  static inline const Transform zero()
  {
    Transform transform(glm::vec3(0.0f));
    return transform;
  }

  /**
   * @brief Builds a Transform from a matrix (translation, rotation, scale). Shear is discarded.
   */
  static Transform from_matrix(const glm::mat4& matrix);

  /** @brief Converts Euler angles in degrees (same order as to_matrix()) to a quaternion. */
  static glm::quat euler_degrees_to_quat(const glm::vec3& euler_degrees);

  /** @brief Converts a quaternion to Euler angles in degrees (same order as to_matrix()). */
  static glm::vec3 quat_to_euler_degrees(const glm::quat& rotation);

  glm::vec3 position;
  glm::vec3 rotation;
  glm::vec3 scale = glm::vec3(1.0f);

  /** @brief Rotation as a quaternion. */
  glm::quat get_rotation_quat() const;

  /** @brief Sets rotation from a quaternion (stored as Euler degrees). */
  void set_rotation_quat(const glm::quat& rotation);

  /**
   * @brief Combines the position, rotation, and scale into a single 4x4 transformation matrix.
   *
   * @return glm::mat4
   */
  glm::mat4 to_matrix() const;
};
}
