#pragma once

#include "calgine/core/renderer/vertex.h"
#include "glad/gl.h"

namespace Calgine {

class VertexBuffer
{
public:
  VertexBuffer(std::span<const Vertex> vertex);
  inline ~VertexBuffer()
  {
    // Skip cleanup - OpenGL context may be destroyed during shutdown
  }

  void bind() const;
  void unbind() const;

private:
  uint32_t renderer_id;
  
};
}