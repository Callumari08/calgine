#pragma once

#include <cstdint>
namespace Calgine {
 
class VertexArray
{
public:
  VertexArray();

  void bind() const;

  void set_layout() const;
private:
  uint32_t renderer_id;
};

}