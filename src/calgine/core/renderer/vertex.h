#pragma once

#include <glm/ext/vector_float4.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float2.hpp>

namespace Calgine {

struct Vertex
{
  glm::vec4 colour;
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 uv;
};

}