#pragma once

#include "calgine/core/setup/window.h"
#include "calgine_pch.h"
#include "calgine_api.h"
#include <string>

namespace Calgine {

struct AppSettings
{
  std::string app_name = "A Calgine App";
  VsyncState default_vsync_state = VsyncState::enabled;
};

/**
 * @brief Base class for all Calgine applications.
 *
 * @details
 * The App class manages the core application lifecycle, including system initialization,
 * the main game loop, event handling, and rendering. User applications must derive from
 * this class and implement the pure virtual get_app_name() method.
 *
 * The App class handles:
 * - SDL3 and OpenGL initialization
 * - Window management through WindowHandler
 * - Main loop execution with tick updates (preloop, update, late_update)
 * - SDL event processing (window close, quit events)
 * - Rendering for all active windows
 * - GameObject hierarchy lifecycle management
 *
 * ## Basic Usage
 *
 * 1. Create a derived class implementing get_app_name()
 * 2. Call systems_init() to initialize SDL, OpenGL, and create windows
 * 3. Set up your GameObject hierarchy and attach Behaviour components
 * 4. Call main_loop() to start the application
 *
 * @code{.cpp}
 * class MyApp : public Calgine::App
 * {
 * public:
 *   std::string get_app_name() override
 *   {
 *     return "My Application";
 *   }
 * };
 *
 * class MyBehaviour : public Calgine::Behaviour
 * {
 * protected:
 *   void update_tick() override
 *   {
 *     // Called every frame during the update phase
 *     std::cout << "Update tick!\n";
 *   }
 * };
 *
 * int main()
 * {
 *   MyApp app;
 *
 *   // Initialize SDL, OpenGL, and create windows
 *   app.systems_init();
 *
 *   // Access the GameObject hierarchy root
 *   Calgine::Hierarchy& hierarchy = Calgine::Hierarchy::get_instance();
 *   Calgine::GameObject& root = hierarchy.get_hierarchy_root();
 *
 *   // Create a child GameObject and attach a behaviour
 *   Calgine::GameObject& gameObj = root.instantiate_child();
 *   gameObj.set_name("MyGameObject");
 *   gameObj.add_behaviour<MyBehaviour>();
 *
 *   // Start the main loop (runs until all windows are closed)
 *   app.main_loop();
 *
 *   return 0;
 * }
 * @endcode
 *
 * @see GameObject for information about the scene hierarchy
 * @see Behaviour for creating game logic components
 * @see Hierarchy for accessing the root GameObject
 */
class CALGINE_API App // Abstract
{
public:
  AppSettings settings;

  //virtual AppSettings get_app_settings() = 0;
  //virtual void set_app_settings();
  /**
   * @brief Initializes all required systems (SDL3, OpenGL, logging).
   * 
   * This method must be called before main_loop(). It performs the following:
   * - Initializes the logging system with the application name
   * - Sets up SDL3 video subsystem
   * - Configures OpenGL context attributes (OpenGL 4.5 Core Profile)
   * - Creates initial windows through WindowHandler
   * - Initializes GLAD2 for OpenGL function loading
   * 
   * @throws std::runtime_error if SDL initialization fails
   * @throws std::runtime_error if GLAD initialization fails
   */
  void systems_init();

  /**
   * @brief Runs the main application loop until all windows are closed.
   * 
   * Before entering the loop, this method calls preloop_tick() once on all 
   * GameObjects in the hierarchy. Then, each frame executes the following steps:
   * 1. Processes SDL events (window close, quit, etc.)
   * 2. Calls update_tick() on all Behaviours attached to GameObjects
   * 3. Calls late_tick() on all Behaviours attached to GameObjects
   * 4. Renders all active windows
   * 5. Processes pending GameObject deletions
   * 6. Cleans up closed windows
   * 
   * The loop continues until either SDL_EVENT_QUIT is received or all
   * windows have been closed.
   * 
   * @see GameObject::tick_self_and_children() for how tick methods are propagated through the hierarchy
   */
  void main_loop();

private:
  void handle_sdl_events(bool& running);
  void render_windows();
};

} // namespace Calgine