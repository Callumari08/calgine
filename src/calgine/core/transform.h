#pragma once

#include "glm/ext/vector_float3.hpp"
#include <glm/glm.hpp>

namespace Calgine {

class Transform
{
public:
  glm::vec3 position;
  glm::vec3 rotation;
  glm::vec3 scale = glm::vec3(1.0f);

  glm::mat4 matrix() const;
};
}