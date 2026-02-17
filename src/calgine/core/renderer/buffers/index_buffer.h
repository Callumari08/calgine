#pragma once

#include <cstdint>
#include "glad/gl.h"

namespace Calgine {

class IndexBuffer
{
public:
  IndexBuffer(uint32_t* indices, uint32_t _count);
  inline ~IndexBuffer()
  {
    glDeleteBuffers(1, &renderer_id);
  }

  void bind() const;
  void unbind() const;

  uint32_t get_count()
  {
    return count;
  }

private:
  uint32_t renderer_id;
  uint32_t count;
};
}