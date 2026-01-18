#pragma once

#include <calgine_pch.h>
#include "SDL3/SDL_stdinc.h"
#include "calgine_api.h"
#include <SDL3/SDL_video.h>

// WindowHandler should own all Windows (but I won't force it to here)

namespace Calgine {

// could include different types of Vsync buffers here
enum VsyncState 
{
  disabled,
  enabled,
  adaptive = -1,
};

/**
 * @brief Window wrapper that abstracts SDL_Window from the application
 */
class CALGINE_API Window
{
#define DEFAULT_WINDOW_WIDTH 1920 / 2
#define DEFAULT_WINDOW_HEIGHT 1080 / 2
private:
  Uint32 id;
  bool close_requested = false;

  SDL_Window* sdl_window = nullptr;
  SDL_GLContext gl_context = nullptr;

  VsyncState vsync_state;
  
public:
  Window(std::string window_title, VsyncState state = VsyncState::disabled, 
    int width = DEFAULT_WINDOW_WIDTH, int height = DEFAULT_WINDOW_HEIGHT, int extra_flags = 0);
  ~Window();

  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;

  Window(Window&&) noexcept = delete;
  Window& operator=(Window&&) noexcept = delete;

  void request_close();
  bool should_close() const;

  SDL_Window* raw() const;
  SDL_GLContext get_context() const;

  Uint32 get_id() const;

  int width() const;
  int height() const;

  void set_window_title(const std::string& title);
  std::string get_window_title() const;

  void set_vsync_state(VsyncState state);
  VsyncState get_vsync_state() const;
};

}