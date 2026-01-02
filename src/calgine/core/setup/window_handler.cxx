#include "window_handler.h"
#include "../useful_funcs.h"
#include <SDL3/SDL_video.h>
#include <cstddef>
#include <mutex>
#include <ostream>
#include <stdexcept>
#include <iostream>
#include <string>

WindowHandler* WindowHandler::instance = nullptr;
std::mutex WindowHandler::instance_mutex;

SDL_Window* WindowHandler::window = nullptr;
SDL_GLContext WindowHandler::gl_context = NULL;
VsyncState WindowHandler::vsync_state = VsyncState::disabled;

std::string WindowHandler::window_name;

WindowHandler* WindowHandler::get_instance() 
{
  if (instance == nullptr) 
  {
    std::lock_guard<std::mutex> lock(instance_mutex);
    if (instance == nullptr) 
    {
      instance = new WindowHandler();
    }
  }
  return instance;
}

SDL_Window* WindowHandler::get_window() 
{
  if (window != nullptr) 
  {
    if (window_name.compare(std::to_string(*SDL_GetWindowTitle(window))) == 0)
    {
      SDL_SetWindowTitle(window, window_name.c_str());
      std::cout << "Changed window name to " << window_name << std::endl;
    }
    return window;
  }

  std::cout << "Creating window\n";

  window = SDL_CreateWindow(
    window_name.c_str(),
    WINDOW_WIDTH,
    WINDOW_HEIGHT,
    SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
  );

  if (window == nullptr) 
  {
    throw_sdl_error("Failed to create window: ");
    quit_gracefully(window, gl_context);
  }

  return window;
}

SDL_GLContext WindowHandler::get_gl_context() 
{
  if (gl_context != NULL)
    return gl_context;

  if (window == nullptr)
  {
    throw std::runtime_error("Window does not exist. You need to call get_window for this function to work.");
    quit_gracefully(window, gl_context);
  }

  std::cout << "Creating GL Context\n";

  gl_context = SDL_GL_CreateContext(window);

  if (!gl_context) {
    throw_sdl_error("Failed to create gl_context: ");
    quit_gracefully(window, gl_context);
  }

  return gl_context;
}

VsyncState WindowHandler::get_vsync_state() 
{
  return vsync_state;
} 

void WindowHandler::set_vsync_state(VsyncState state)
{
  vsync_state = state;
  SDL_GL_SetSwapInterval(vsync_state);
}

void WindowHandler::quit_gracefully(SDL_Window* window, SDL_GLContext ctx)
{
  if (window != nullptr)
  {
    SDL_DestroyWindow(window);
  }
  if (ctx != NULL)
  {
    SDL_GL_DestroyContext(ctx);
  }

  SDL_Quit();
}