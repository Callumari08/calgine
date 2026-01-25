#pragma once

#include "calgine/core/game_object.h"
#include "calgine/core/renderer/mesh.h"
#include "calgine/core/renderer/shader.h"
#include "calgine/core/renderer/camera.h"
#include "calgine/core/window/window.h"
#include "calgine_pch.h"
#include "calgine_api.h"
#include "imgui.h"
#include <memory>
#include <string>

namespace Calgine {

/**
 * @brief Configuration settings for a Calgine application.
 *
 * @details
 * This structure holds application-wide configuration settings that can be
 * modified before calling systems_init(). All settings have sensible defaults.
 */
struct AppSettings
{
  /** @brief The name of the application, used for logging and window titles. */
  std::string app_name = "A Calgine App";
  
  /** @brief The default VSync state for windows. Can be enabled, disabled, or adaptive. */
  VsyncState default_vsync_state = VsyncState::enabled;

  ImGuiContext* imgui_context = nullptr;

  bool enable_imgui_docking = true;

  bool enable_imgui_viewport = true;
};

/**
 * @brief Base class for all Calgine applications.
 *
 * @details
 * The App class manages the core application lifecycle, including system initialization,
 * the main game loop, event handling, and rendering. User applications can either instantiate
 * App directly or derive from it to add custom functionality.
 *
 * The App class handles:
 * - SDL3 and OpenGL initialization
 * - Window management through WindowHandler
 * - Main loop execution with tick updates (preloop, update, late_update)
 * - SDL event processing (window close, quit events)
 * - Rendering for all active windows
 * - GameObject hierarchy lifecycle management
 *
 * ## Configuration
 *
 * Applications can be configured through the public `settings` member variable before
 * calling systems_init(). The AppSettings structure provides options for:
 * - Application name (used for logging and window titles)
 * - Default VSync state for windows
 *
 * ## Basic Usage
 *
 * 1. Create an App instance (or derive from App for custom functionality)
 * 2. Configure app settings (optional)
 * 3. Call systems_init() to initialize SDL, OpenGL, and create windows
 * 4. Set up your GameObject hierarchy and attach Behaviour components
 * 5. Call main_loop() to start the application
 *
 * @code{.cpp}
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
 *   Calgine::App app;
 *
 *   // Configure application settings (optional)
 *   app.settings.app_name = "My Application";
 *   app.settings.default_vsync_state = Calgine::VsyncState::adaptive;
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
 * @see AppSettings for application configuration options
 * @see GameObject for information about the scene hierarchy
 * @see Behaviour for creating game logic components
 * @see Hierarchy for accessing the root GameObject
 */
class CALGINE_API App // Abstract
{
#define GLSL_VERSION "#version 450"

public:
  App();
  ~App();

  static App& get_instance();

  inline void start_systems()
  {
    systems_init();
    ImGui::SetCurrentContext(settings.imgui_context);
  }

  /**
   * @brief Application configuration settings.
   * 
   * @details
   * This public member provides access to application-wide configuration settings.
   * Settings should be modified before calling systems_init() to take effect during
   * initialization. Settings can be modified after initialization, but some changes
   * (like app_name) may not be reflected in already-created resources.
   *
   * @code{.cpp}
   * Calgine::App app;
   * app.settings.app_name = "My Game";
   * app.settings.default_vsync_state = Calgine::VsyncState::disabled;
   * app.systems_init();
   * @endcode
   *
   * @see AppSettings for available configuration options
   */
  AppSettings settings;

  //virtual AppSettings get_app_settings() = 0;
  //virtual void set_app_settings();
  /**
   * @brief Initializes all required systems (SDL3, OpenGL, logging).
   *
   * @warning Do NOT call directly! use the start_systems() inline function!
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
  static App* instance;

  std::unique_ptr<Shader> shader;
  std::unique_ptr<Mesh> mesh;
  std::unique_ptr<Camera> camera;

  void init_buffers();
  void init_imgui();
  void handle_sdl_events(bool& running);
  void render_windows(GameObject& root);
  void do_stuff_on_single_window();
};

} // namespace Calgine