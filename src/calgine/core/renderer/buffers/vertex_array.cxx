#include "calgine/core/renderer/buffers/vertex_array.h"
#include "calgine/core/renderer/vertex.h"
#include <glad/gl.h>

namespace Calgine {

VertexArray::VertexArray()
{
  glGenVertexArrays(1, &renderer_id);
}

void VertexArray::bind() const
{
  glBindVertexArray(renderer_id);
}

void VertexArray::set_layout() const
{
  bind();

  // Colour (location 0)
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(
    0, 4, GL_FLOAT, GL_FALSE,
    sizeof(Vertex),
    (void*)offsetof(Vertex, colour)
  );

  // Position (location 1)
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(
    1, 3, GL_FLOAT, GL_FALSE,
    sizeof(Vertex),
    (void*)offsetof(Vertex, position)
  );

  // Normal (location 2)
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(
    2, 3, GL_FLOAT, GL_FALSE,
    sizeof(Vertex),
    (void*)offsetof(Vertex, normal)
  );

  // TexCoord/UV (location 3)
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(
    3, 2, GL_FLOAT, GL_FALSE,
    sizeof(Vertex),
    (void*)offsetof(Vertex, uv)
  );
}

}