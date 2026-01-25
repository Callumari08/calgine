#include "vertex_buffer.h"
#include "calgine/core/renderer/vertex.h"
#include "calgine_pch.h"
#include "glad/gl.h"
#include <span>

namespace Calgine {

VertexBuffer::VertexBuffer(std::span<const Vertex> vertex)
{
  static_assert(std::is_standard_layout_v<Vertex>);
  static_assert(sizeof(Vertex) == sizeof(float) * (4 + 3 + 3 + 2));

  glCreateBuffers(1, &renderer_id);
  glBindBuffer(GL_ARRAY_BUFFER, renderer_id);
  glBufferData(GL_ARRAY_BUFFER, vertex.size_bytes(), vertex.data(), GL_STATIC_DRAW);
}

VertexBuffer::~VertexBuffer()
{
  glDeleteBuffers(1, &renderer_id);
}

void VertexBuffer::bind()
{
  glBindBuffer(GL_ARRAY_BUFFER, renderer_id);
}

void VertexBuffer::unbind()
{
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

}