#pragma once

#include "SDL3/SDL_stdinc.h"
#include "calgine_pch.h"

#include "window.h"
#include "calgine_api.h"

namespace Calgine {
class Window;

/**
 * @brief Singleton manager for all application windows.
 * 
 * @details
 * WindowHandler provides centralized management of all Window instances in the application.
 * It follows the singleton pattern to ensure a single point of control for window lifecycle
 * management, including creation, retrieval, and cleanup of closed windows.
 * 
 * Windows are stored as unique_ptr instances, with the WindowHandler maintaining sole
 * ownership of all managed windows.
 */
class CALGINE_API WindowHandler 
{
private:
  std::vector<std::unique_ptr<Window>> windows;

  WindowHandler() = default;

public:
  WindowHandler(const WindowHandler&) = delete;
  WindowHandler& operator=(const WindowHandler&) = delete;

  /**
   * @brief Get the singleton instance of WindowHandler.
   * 
   * @return WindowHandler* Pointer to the singleton WindowHandler instance.
   */
  static WindowHandler* get_instance();

  /**
   * @brief Get a reference to the vector of all managed windows.
   * 
   * @return std::vector<std::unique_ptr<Window>>& Reference to the internal windows vector.
   */
  std::vector<std::unique_ptr<Window>>& get_windows();

  /**
   * @brief Retrieve a window by its SDL window ID.
   * 
   * @param id The SDL window ID to search for.
   * @return Window* Pointer to the Window if found, nullptr otherwise.
   */
  Window* get_window(Uint32 id);

  /**
   * @brief Remove all windows that have been marked for closure.
   * 
   * @details
   * Iterates through all managed windows and removes those that have had
   * request_close() called on them. This frees the memory associated with
   * closed windows.
   */
  void cleanup_closed_windows();

  /**
   * @brief Create and add a new window to the handler.
   * 
   * @tparam Args Variadic template parameters forwarded to Window constructor.
   * @param args Arguments to forward to the Window constructor.
   * @return std::unique_ptr<Window>& Reference to the newly created Window.
   * 
   * @details
   * Creates a new Window instance using perfect forwarding of the provided
   * arguments. The window is emplaced into the internal windows vector and
   * ownership is managed by WindowHandler.
   */
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