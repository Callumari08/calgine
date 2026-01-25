#include "calgine/core/renderer/buffers/vertex_array.h"
#include "calgine/core/renderer/vertex.h"
#include <glad/gl.h>

namespace Calgine {

VertexArray::VertexArray()
{
  glGenVertexArrays(1, &renderer_id);
}

void VertexArray::bind()
{
  glBindVertexArray(renderer_id);
}

void VertexArray::set_layout()
{
  bind();

  glEnableVertexAttribArray(0);

  glVertexAttribPointer(
    0, 3, GL_FLOAT, GL_FALSE,
    sizeof(Vertex),
    (void*)offsetof(Vertex, position)
  );

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(
    1, 3, GL_FLOAT, GL_FALSE,
    sizeof(Vertex),
    (void*)offsetof(Vertex, normal)
  );

  glEnableVertexAttribArray(2);
  glVertexAttribPointer(
    2, 2, GL_FLOAT, GL_FALSE,
    sizeof(Vertex),
    (void*)offsetof(Vertex, uv)
  );
}

}