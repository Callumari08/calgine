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
    glDeleteVertexArrays(1, &renderer_id);
  }

  void bind() const;

  void set_layout() const;
private:
  uint32_t renderer_id;
};

}