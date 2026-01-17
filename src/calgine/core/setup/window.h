#pragma once

#include <calgine_pch.h>
#include "SDL3/SDL_stdinc.h"
#include "calgine/core/setup/window_handler.h"
#include "calgine_api.h"
#include <SDL3/SDL_video.h>
#include <cstddef>
#include <sys/types.h>

// WindowHandler should own all Windows (but I won't force it to here)

namespace Calgine {

// could include different types of Vsync buffers here
enum VsyncState 
{
  disabled,
  enabled,
};

/**
 * @brief Window wrappper that abstracts SDL_Window from the application
 */
class CALGINE_API Window
{
#define DEFAULT_WINDOW_WIDTH 1600
#define DEFAULT_WINDOW_HEIGHT 120
#define MAX_TITLE_LENGTH 128
private:
  Uint32 id;
  bool close_requested = false;

  SDL_Window* sdl_window;
  SDL_GLContext gl_context;

  VsyncState vsync_state;
  
public:
  Window(std::string window_title, VsyncState state = VsyncState::disabled, 
    int width = DEFAULT_WINDOW_WIDTH, int height = DEFAULT_WINDOW_HEIGHT, int extra_flags = 0);
  ~Window();

  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;

  Window(Window&&) noexcept;
  Window& operator=(Window&&) noexcept;

  void request_close();
  bool should_close() const;

  SDL_Window* raw() const;
  SDL_GLContext get_context() const;

  Uint32 get_id() const;

  int width() const;
  int height() const;

  void set_window_title(std::string& title);
  std::string get_window_title() const;

  void set_vsync_state(VsyncState state);
  VsyncState get_vsync_state();

  size_t get_index() const;
};

}