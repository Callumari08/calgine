#pragma once

#include "glm/vec4.hpp"
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

namespace Calgine {

struct Vertex
{
  glm::vec4 colour;
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 uv;
};

}