#include "render_texture.h"

#include "calgine/core/log.h"
#include "glad/gl.h"
#include <cstdint>


namespace Calgine {

RenderTexture::RenderTexture(const std::shared_ptr<Texture>& texture)
  : texture_ptr(texture)
{
  if (!texture_ptr || !texture_ptr->get_data())
  {
    Log::get_engine_logger()->error("Failed to create Render Texture: invalid texture data");
    renderer_id = 0;
    return;
  }

  glGenTextures(1, &renderer_id);
  glBindTexture(GL_TEXTURE_2D, renderer_id);

  GLenum internalFormat = GL_RGBA;
  GLenum dataFormat = GL_RGBA;
  uint32_t channels = texture_ptr->get_channels();

  if (channels == 3)
  {
    internalFormat = GL_RGB;
    dataFormat = GL_RGB;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  }

  uint32_t width = texture_ptr->get_width();
  uint32_t height = texture_ptr->get_height();
  const uint8_t* data_ptr = texture_ptr->get_data();

  glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 
    0, dataFormat, GL_UNSIGNED_BYTE, data_ptr);

  if (channels == 3)
  {
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  }

  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
  {
    Log::get_engine_logger()->error("glTexImage2D failed with error: {}", error);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(1, &renderer_id);
    renderer_id = 0;
    return;
  }

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glBindTexture(GL_TEXTURE_2D, 0);
}

RenderTexture::~RenderTexture()
{
  if (renderer_id != 0)
    glDeleteTextures(1, &renderer_id);
}

void RenderTexture::bind(uint32_t slot) const
{
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, renderer_id);
}

void RenderTexture::unbind() const
{
  glBindTexture(GL_TEXTURE_2D, 0);
}

}