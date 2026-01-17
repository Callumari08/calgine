#pragma once

#include "SDL3/SDL_stdinc.h"
#include "calgine_pch.h"

#include "window.h"
#include "calgine_api.h"

namespace Calgine {
class Window;

class CALGINE_API WindowHandler 
{
private:
  std::vector<std::unique_ptr<Window>> windows;

  WindowHandler() = default;

public:
  WindowHandler(const WindowHandler&) = delete;
  WindowHandler& operator=(const WindowHandler&) = delete;

  static WindowHandler* get_instance();

  std::vector<std::unique_ptr<Window>>& get_windows();

  Window* get_window(Uint32 id);

  void cleanup_closed_windows();

  template<typename... Args>
  std::unique_ptr<Window>& emplace_new_window(Args&&... args)
  {
    windows.emplace_back(
      std::make_unique<Window>(std::forward<Args>(args)...)
    );

    return windows[windows.size() - 1];
  }
};

}