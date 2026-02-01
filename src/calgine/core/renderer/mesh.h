#pragma once

#include "calgine_api.h"
#include "calgine_pch.h"
#include "calgine/core/renderer/buffers/index_buffer.h"
#include "calgine/core/renderer/buffers/vertex_array.h"
#include "calgine/core/renderer/buffers/vertex_buffer.h"
#include "calgine/core/renderer/vertex.h"

namespace Calgine {

class CALGINE_API Mesh
{
public:

  Mesh(const std::span<const Vertex> vertices, std::span<const uint32_t> indices);
  ~Mesh() = default;

  void bind() const;
  void draw() const;

  inline uint32_t get_index_count() const { return index_count; }

private:
  std::unique_ptr<VertexArray> vao;
  std::unique_ptr<VertexBuffer> vbo;
  std::unique_ptr<IndexBuffer> ibo;

  uint32_t index_count;
};


}