#include "index_buffer.h"
#include "calgine_pch.h"
#include <cstdint>
#include <glad/gl.h>

namespace Calgine {

IndexBuffer::IndexBuffer(const uint32_t* indices, uint32_t _count)
  : count(_count)
{
  glCreateBuffers(1, &renderer_id);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer_id);  
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
}

void IndexBuffer::bind() const
{
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer_id);  
}

}