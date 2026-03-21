#pragma once

#include "glm/ext/vector_float4.hpp"
#include <glm/vec4.hpp>

namespace Calgine {

class Colour
{
public:
  Colour (const glm::vec4& vector) : vector4(vector) {}
  Colour(float red = 0.0f, float green = 0.0f, float blue = 0.0f, float alpha = 1.0f)
    : vector4(red, green, blue, alpha) {}

  explicit operator glm::vec4() const { return vector4; }
  glm::vec4* operator->() { return &vector4; }

  static inline Colour red() { return Colour(1.0); }
  static inline Colour green() { return Colour(0.0f, 1.0f); }
  static inline Colour blue() { return Colour(0.0f, 0.0f, 1.0f); }
  static inline Colour white() { return glm::vec4(1.0f); }
  static inline Colour black() { return Colour(0.0f, 0.0f, 0.0f); }
  static inline Colour transparent() { return glm::vec4(0.0f); } 
private:
  glm::vec4 vector4;
};

}