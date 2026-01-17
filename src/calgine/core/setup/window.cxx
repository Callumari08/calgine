#include "window.h"
#include "../useful_funcs.h"
#include "SDL3/SDL_stdinc.h"
#include "SDL3/SDL_video.h"
#include "calgine/core/log.h"

namespace Calgine {
  Window::Window(std::string window_title, VsyncState state, int width, int height, int extra_flags)
  {
    Log::get_engine_logger()->info("{}: Creating window", window_title);

    sdl_window = SDL_CreateWindow(window_title.c_str(), width, height,
      extra_flags | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);

    if (sdl_window == nullptr) 
    {
      throw_sdl_error(std::format("{} failed to create window: ", window_title));
    }

    gl_context = SDL_GL_CreateContext(sdl_window);

    if (!gl_context) 
    {
      throw_sdl_error(std::format("{} failed to create GL Context", window_title));
    }

    set_vsync_state(state);

    id = SDL_GetWindowID(sdl_window);
  }

  Window::~Window()
  {
    SDL_GL_DestroyContext(gl_context);
    SDL_DestroyWindow(sdl_window);
  }

  void Window::request_close() 
  {
    close_requested = true;
  }

  bool Window::should_close() const
  {
    return close_requested;
  }

  SDL_Window* Window::raw() const
  {
    return sdl_window;
  }

  SDL_GLContext Window::get_context() const
  {
    return gl_context;
  }

  Uint32 Window::get_id() const
  {
    return SDL_GetWindowID(sdl_window);
  }

  int Window::width() const
  {
    int width;
    SDL_GetWindowSize(sdl_window, &width, nullptr);
    return width;
  }

  int Window::height() const
  {
    int height;
    SDL_GetWindowSize(sdl_window, nullptr, &height);
    return height;
  }

  void Window::set_window_title(std::string& title)
  {
    SDL_SetWindowTitle(sdl_window, title.c_str());
  }
  
  std::string Window::get_window_title() const
  {
    return SDL_GetWindowTitle(sdl_window);
  }

  void Window::set_vsync_state(VsyncState state)
  {
    vsync_state = state;
    SDL_GL_SetSwapInterval(vsync_state);
  }
}