#pragma once

#include <cstdint>
namespace Calgine {
 
class VertexArray
{
public:
  VertexArray();

  void bind();

  void set_layout();
private:
  uint32_t renderer_id;
};

}