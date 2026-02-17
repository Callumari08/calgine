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
    glDeleteBuffers(1, &renderer_id);
  }

  void bind() const;
  void unbind() const;

private:
  uint32_t renderer_id;
  
};
}