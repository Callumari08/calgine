#include "frame_buffer.h"

#include "glad/gl.h"
#include "calgine/core/log.h"

namespace Calgine {

FrameBuffer::FrameBuffer(uint32_t width, uint32_t height)
  : width(width), height(height)
{
  create();
}

FrameBuffer::~FrameBuffer()
{
  cleanup();
}

void FrameBuffer::create()
{
  if (width == 0 || height == 0)
  {
    Log::get_engine_logger()->error("FrameBuffer dimensions must be non-zero");
    return;
  }

  // Create color texture
  glGenTextures(1, &color_texture);
  glBindTexture(GL_TEXTURE_2D, color_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
               0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glBindTexture(GL_TEXTURE_2D, 0);

  // Create depth renderbuffer
  glGenRenderbuffers(1, &depth_rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, depth_rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  // Create framebuffer and attach
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_2D, color_texture, 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, depth_rbo);

  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE)
  {
    Log::get_engine_logger()->error("FrameBuffer incomplete: {}", status);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    cleanup();
    return;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  Log::get_engine_logger()->info("FrameBuffer created: {}x{}", width, height);
}

void FrameBuffer::cleanup()
{
  if (fbo != 0)
    glDeleteFramebuffers(1, &fbo);
  if (color_texture != 0)
    glDeleteTextures(1, &color_texture);
  if (depth_rbo != 0)
    glDeleteRenderbuffers(1, &depth_rbo);

  fbo = 0;
  color_texture = 0;
  depth_rbo = 0;
}

void FrameBuffer::bind() const
{
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glViewport(0, 0, width, height);
}

void FrameBuffer::unbind() const
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

} // namespace Calgine
