#pragma once

#include <cstdint>
namespace Calgine {

class IndexBuffer
{
public:
  IndexBuffer(uint32_t* indices, uint32_t _count);
  ~IndexBuffer();

  void bind();
  void unbind();

  uint32_t get_count()
  {
    return count;
  }

private:
  uint32_t renderer_id;
  uint32_t count;
};
}