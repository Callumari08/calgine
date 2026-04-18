#pragma once

#include "calgine_api.h"
#include "calgine_pch.h"

namespace Calgine {

/**
 * @brief Manages OpenGL Framebuffer Objects for offscreen rendering.
 * 
 * @details
 * FrameBuffer encapsulates the creation and management of OpenGL FBOs (Framebuffer Objects).
 * It handles both color texture attachments and depth renderbuffer attachments, and provides
 * simple bind/unbind functionality for rendering to texture.
 * 
 * Example usage:
 * @code
 * FrameBuffer fb(1920, 1080);
 * // ... later in render loop ...
 * fb.bind();
 * // render scene
 * fb.unbind();
 * uint32_t texture = fb.get_color_texture();
 * @endcode
 */
class CALGINE_API FrameBuffer
{
public:
  /**
   * @brief Creates a FrameBuffer with specified dimensions.
   * 
   * @param width Width of the framebuffer texture
   * @param height Height of the framebuffer texture
   */
  FrameBuffer(uint32_t width, uint32_t height);
  ~FrameBuffer();

  /**
   * @brief Binds the framebuffer for rendering.
   * 
   * Also sets the viewport to match the framebuffer dimensions.
   */
  void bind() const;

  /**
   * @brief Unbinds the framebuffer (returns to rendering to window).
   */
  void unbind() const;

  /**
   * @brief Gets the OpenGL name of the framebuffer.
   */
  uint32_t get_fbo_id() const { return fbo; }

  /**
   * @brief Gets the OpenGL name of the color texture attachment.
   */
  uint32_t get_color_texture() const { return color_texture; }

  /**
   * @brief Gets the width of the framebuffer.
   */
  uint32_t get_width() const { return width; }

  /**
   * @brief Gets the height of the framebuffer.
   */
  uint32_t get_height() const { return height; }

private:
  uint32_t fbo = 0;
  uint32_t color_texture = 0;
  uint32_t depth_rbo = 0;
  uint32_t width = 0;
  uint32_t height = 0;

  void create();
  void cleanup();
};

}
