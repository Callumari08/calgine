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

  Mesh(Mesh&&) = default;
  Mesh& operator=(Mesh&&) = default;

  void bind() const;
  void draw() const;

  inline uint32_t get_index_count() const { return index_count; }

  /**
   * @brief CPU copy of the vertex positions, kept after upload for collision shapes
   * (see ColliderShape::make_convex_hull / make_mesh).
   */
  const std::vector<glm::vec3>& get_positions() const { return positions; }
  /** @brief CPU copy of the triangle indices into get_positions(). */
  const std::vector<uint32_t>& get_indices() const { return indices; }

private:
  std::vector<glm::vec3> positions;
  std::vector<uint32_t> indices;

  std::unique_ptr<VertexArray> vao;
  std::unique_ptr<VertexBuffer> vbo;
  std::unique_ptr<IndexBuffer> ibo;

  uint32_t index_count;
};


}