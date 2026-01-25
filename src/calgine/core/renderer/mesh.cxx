#include "mesh.h"

#include "calgine_pch.h"
#include "glad/gl.h"

namespace Calgine {

Mesh::Mesh(std::span<const Vertex> vertices, std::span<const uint32_t> indices)
  : index_count(indices.size())
{
  vao = std::make_unique<VertexArray>();
  vao->bind();

  vbo = std::make_unique<VertexBuffer>(vertices);
  vbo->bind();

  ibo = std::make_unique<IndexBuffer>(
    const_cast<uint32_t*>(indices.data()),
    indices.size()
  );
  ibo->bind();

  vao->set_layout();
}

void Mesh::bind() const
{
  vao->bind();
}

void Mesh::draw() const
{
  bind();
  glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, nullptr);
}
  
}