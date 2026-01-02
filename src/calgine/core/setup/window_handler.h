#pragma once

#include <SDL3/SDL_video.h>
#include <mutex>
#include <string>

// could include different types of Vsync buffers here
enum VsyncState {
  disabled,
  enabled,
};

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

class WindowHandler 
{
private:
  static WindowHandler* instance;
  static std::mutex instance_mutex;

  // Private constructor to prevent instantiation
  WindowHandler() = default;

  static SDL_Window* window;
  static SDL_GLContext gl_context;
  static VsyncState vsync_state;

  static void quit_gracefully(SDL_Window* window, SDL_GLContext ctx);

public:
  static std::string window_name;
  WindowHandler(const WindowHandler&) = delete;
  WindowHandler& operator=(const WindowHandler&) = delete;

  // Returns the singleton instance of WindowHandler
  static WindowHandler* get_instance();

  // Returns an `SDL_Window` if one exists, otherwise it creates one.
  static SDL_Window* get_window();
  // Returns an `SDL_GLContext` if one exists, otherwise it creates one.
  // `window` mustn't be NULL, otherwise this function will throw an exception (TODO)
  static SDL_GLContext get_gl_context();
  static VsyncState get_vsync_state();
  static void set_vsync_state(VsyncState state);
};