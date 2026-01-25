#pragma once

#include "calgine/core/renderer/vertex.h"
namespace Calgine {

class VertexBuffer
{
public:
  VertexBuffer(std::span<const Vertex> vertex);
  ~VertexBuffer();

  void bind();
  void unbind();


private:
  uint32_t renderer_id;
  
};
}