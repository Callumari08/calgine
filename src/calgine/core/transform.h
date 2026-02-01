#pragma once

#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>
#include "calgine_api.h"

namespace Calgine {

class CALGINE_API Transform
{
public:
  Transform(glm::vec3 position, glm::vec3 rotation = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f));
  
  static inline Transform zero()
  {
    Transform transform(glm::vec3(0.0f));
    return transform;
  }

  glm::vec3 position;
  glm::vec3 rotation;
  glm::vec3 scale = glm::vec3(1.0f);

  /**
   * @brief Combines the position, rotation, and scale into a single 4x4 transformation matrix.
   * 
   * @return glm::mat4 
   */
  glm::mat4 to_matrix() const;
};
}