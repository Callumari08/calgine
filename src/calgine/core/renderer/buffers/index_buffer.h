#pragma once

#include <cstdint>

namespace Calgine {

class IndexBuffer
{
public:
  IndexBuffer(const uint32_t* indices, uint32_t count);
  inline ~IndexBuffer()
  {
    // Skipping cleanup because OpenGL context may be destroyed during shutdown
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