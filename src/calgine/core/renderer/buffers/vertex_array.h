#pragma once

#include <cstdint>
#include "glad/gl.h"

namespace Calgine {
 
class VertexArray
{
public:
  VertexArray();
  inline ~VertexArray()
  {
    // Skip cleanup - OpenGL context may be destroyed during shutdown
  }

  void bind() const;

  void set_layout() const;
private:
  uint32_t renderer_id;
};

}