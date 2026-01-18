#include "window_handler.h"

#include "calgine_pch.h"

#include <SDL3/SDL_video.h>

namespace Calgine {

WindowHandler* WindowHandler::get_instance() 
{
  static WindowHandler instance;
  return &instance;
}

std::vector<std::unique_ptr<Window>>& WindowHandler::get_windows()
{
  return windows;
}

Window* WindowHandler::get_window(Uint32 id)
{
  for (auto& window : windows)
  {
    if (window->get_id() == id)
      return window.get();
  }
  return nullptr;
}

void WindowHandler::cleanup_closed_windows()
{
  windows.erase(
    std::remove_if(windows.begin(), windows.end(),
      [](const std::unique_ptr<Window>& window)
      {
        return window->should_close();
      }),
    windows.end()
  );
}

}