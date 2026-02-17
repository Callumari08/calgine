#include "vertex_buffer.h"
#include "calgine/core/renderer/vertex.h"
#include "calgine_pch.h"
#include "glad/gl.h"
#include <span>

namespace Calgine {

VertexBuffer::VertexBuffer(std::span<const Vertex> vertex)
{
  glCreateBuffers(1, &renderer_id);
  glBindBuffer(GL_ARRAY_BUFFER, renderer_id);
  glBufferData(GL_ARRAY_BUFFER, vertex.size_bytes(), vertex.data(), GL_STATIC_DRAW);
}

void VertexBuffer::bind() const
{
  glBindBuffer(GL_ARRAY_BUFFER, renderer_id);
}

void VertexBuffer::unbind() const
{
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

}