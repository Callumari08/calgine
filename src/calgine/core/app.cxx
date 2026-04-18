#include "app.h"

#include <SDL3/SDL_video.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL.h>
#include <glad/gl.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
#include <SDL3/SDL_events.h>
#include <glm/glm.hpp>
#include "calgine/core/behaviour.h"
#include "calgine/core/event_context.h"
#include "calgine/core/hierarchies/game_hierarchy.h"
#include "calgine/core/game_object.h"
#include "calgine/core/hierarchies/manager_hierarchy.h"
#include "calgine/core/input/input_system.h"
#include "calgine/core/transform.h"
#include "window/window_handler.h"
#include "log.h"
#include "calgine/core/renderer/renderer.h"
#include "calgine/core/renderer/camera/camera_manager.h"
#include "calgine/core/time.h"
#include "calgine/core/input/raw_input.h"


namespace Calgine {

App* App::instance = nullptr;

App::App()
{
  assert(!instance && "App already exists!");
  instance = this;
}
App::~App()
{
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext(settings.imgui_context);
}

App& App::get_instance()
{
  assert(instance && "App instance does not exist!");
  return *instance;
}

void App::systems_init()
{
  // Loggers
  Log::init(settings.app_name);

  // SDL
  if (!SDL_Init(SDL_INIT_VIDEO)) 
  {
    Log::get_engine_logger()->error("SDL_Init Failed: {}", SDL_GetError());
    exit(-1);
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  // Window Init handles SDL functions

  WindowHandler* window_handler = WindowHandler::get_instance();
  window_handler->emplace_new_window(settings.app_name, settings.default_vsync_state);

  // GLAD

  Log::get_engine_logger()->info("Initializing GLAD2");

  if (!gladLoadGL((GLADloadfunc) SDL_GL_GetProcAddress)) 
  {
    Log::get_engine_logger()->error("Failed to load OpenGL via GLAD2 (is there a window?)");
    exit(-1);
  } 

  std::string gl_version = (const char*)glGetString(GL_VERSION);
  Log::get_engine_logger()->info("OpenGL Version {}", gl_version);
  
  std::string renderer = (const char*)glGetString(GL_RENDERER);
  Log::get_engine_logger()->info("Renderer Device: {}", renderer);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  // Create internal framebuffer for rendering
  framebuffer = std::make_shared<FrameBuffer>(settings.framebuffer_width, settings.framebuffer_height);

  init_imgui();

  Time::get_instance().init();
}

void App::resize_framebuffer(uint32_t width, uint32_t height)
{
  if (!framebuffer || framebuffer->get_width() != width || framebuffer->get_height() != height)
  {
    framebuffer = std::make_shared<FrameBuffer>(width, height);
  }
}

void App::init_imgui()
{
  IMGUI_CHECKVERSION();
  settings.imgui_context = ImGui::CreateContext();
  ImGui::SetCurrentContext(settings.imgui_context);
  ImGuiIO& io = ImGui::GetIO(); (void) io;

  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

  if (settings.enable_imgui_docking)
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  ImGui::StyleColorsDark();

  float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());

  ImGuiStyle& style = ImGui::GetStyle();
  style.ScaleAllSizes(main_scale);
  style.FontScaleDpi = main_scale;

  WindowHandler* window_handler = WindowHandler::get_instance();
  assert(window_handler->get_windows().size() > 0 && "Window has been deleted during initialization.");
  window_handler->get_windows()[0]->initialize_imgui();
}

void App::main_loop()
{
  GameObject& manager_hierarchy = ManagerHierarchy::get();
  GameObject& game_hierarchy = GameHierarchy::get();

  EventContext event_context;

  manager_hierarchy.instantiate_child(Transform::zero(), "InputSystem").add_behaviour<InputSystemBehaviour>();

  // Instead of doing this here, I should implement a scene manager.
  //
  // (Later comment) I actually think it'd be a good idea to keep root for as long as the program runs,
  // and the GameObjects directly below root is the scene, root can have its own behaviours that can
  // manage the scenes.
  //
  // (Even later comment) 2nd comment was stupid, I wouldn't keep all the levels just casually in RAM??? Am I stupid???
  // I'll probably just make root have a behaviour that can load scenes, and unload the scenes which are gameobjects below it.
  manager_hierarchy.tick_self_and_children(preloop, event_context);
  game_hierarchy.tick_self_and_children(preloop, event_context);

  bool running = true;
  while (running) 
  {
    Time::get_instance().update();

    event_context.update_tick_phase(fixed_update);

    int fixed_timesteps = Time::get_instance().consume_fixed_timesteps();
    for (int i = 0; i < fixed_timesteps; i++)
    {
      manager_hierarchy.tick_self_and_children(fixed_update, event_context);
      game_hierarchy.tick_self_and_children(fixed_update, event_context);
    }

    event_context.update_tick_phase(update);

    handle_sdl_events(running, event_context);

    manager_hierarchy.tick_self_and_children(update, event_context);
    game_hierarchy.tick_self_and_children(update, event_context);

    event_context.update_tick_phase(late_update);

    manager_hierarchy.tick_self_and_children(late_update, event_context);
    game_hierarchy.tick_self_and_children(late_update, event_context);

    render_windows(game_hierarchy, manager_hierarchy, event_context);

    event_context.update_tick_phase(final);

    manager_hierarchy.tick_self_and_children(final, event_context);
    game_hierarchy.tick_self_and_children(final, event_context);

    GameObject::process_pending_deletes();
    
    WindowHandler* window_handler = WindowHandler::get_instance();

    window_handler->cleanup_closed_windows();
    
    if (window_handler->get_windows().empty())
      running = false;
  }
}

void App::handle_sdl_events(bool& running, EventContext& event_context)
{
  SDL_Event event;
  while (SDL_PollEvent(&event)) 
  {
    ImGui_ImplSDL3_ProcessEvent(&event);

    switch (event.type) 
    {
      case SDL_EVENT_KEY_DOWN:
      case SDL_EVENT_KEY_UP:
      {
        RawInputEvent input_event;
        input_event.submit_tick = TickType::update;
        input_event.type = RawInputEventType::keyboard;
        input_event.data = RawKeyboardData(event.key.scancode, 
                                           event.key.key, 
                                           event.type == SDL_EVENT_KEY_DOWN, 
                                           event.key.repeat);
        event_context.submit(input_event);
        break;
      }

      case SDL_EVENT_MOUSE_MOTION:
      {
        RawInputEvent input_event;
        input_event.submit_tick = TickType::update;
        input_event.type = RawInputEventType::mouse_move;
        input_event.data = RawMouseMoveData({event.motion.x, event.motion.y},
                                            {event.motion.xrel, event.motion.yrel});
        event_context.submit(input_event);
        break;
      }

      case SDL_EVENT_MOUSE_BUTTON_DOWN:
      case SDL_EVENT_MOUSE_BUTTON_UP:
      {
        RawInputEvent input_event;
        input_event.submit_tick = TickType::update;
        input_event.type = RawInputEventType::mouse_button;
        input_event.data = RawMouseButtonData(event.button.button,
                                              event.type == SDL_EVENT_MOUSE_BUTTON_DOWN,
                                              event.button.clicks,
                                              {event.button.x, event.button.y});
        event_context.submit(input_event);
        break;
      }

      case SDL_EVENT_MOUSE_WHEEL:
      {
        RawInputEvent input_event;
        input_event.submit_tick = TickType::update;
        input_event.type = RawInputEventType::mouse_wheel;
        input_event.data = RawMouseWheelData(event.wheel.y, event.wheel.x);
        event_context.submit(input_event);
        break;
      }

      case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
      {
        if (Window* window = WindowHandler::get_instance()->get_window(event.window.windowID))
          window->request_close();
        break;
      }
      case SDL_EVENT_WINDOW_RESIZED:
      {
        CameraBehaviour* active_camera = CameraManager::get_instance().get_active_camera();
        if (active_camera && event.window.windowID == WindowHandler::get_instance()->get_windows()[0]->get_id())
        {
          Window* window = WindowHandler::get_instance()->get_window(event.window.windowID);
          if (window)
          {
            float new_aspect_ratio = static_cast<float>(window->width()) / static_cast<float>(window->height());
            CameraSettings camera_settings = active_camera->get_settings();
            camera_settings.aspect_ratio = new_aspect_ratio;
            active_camera->apply_settings(camera_settings);
          }
        }
        break;
      }
      case SDL_EVENT_QUIT:
        running = false;
        break;

      default: break;
    }
  }
}

void App::render_windows(GameObject& game_hierarchy, GameObject& manager_hierarchy, EventContext& event_context)
{
  for (auto& window : WindowHandler::get_instance()->get_windows())
  {
    if (window->should_close())
      continue;

    SDL_GL_MakeCurrent(window->raw(), window->get_context());

    Renderer& renderer_instance = Renderer::get_instance();
    
    // Always render scene to internal framebuffer
    framebuffer->bind();

    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderer_instance.begin_frame();

    event_context.update_tick_phase(render);

    manager_hierarchy.tick_self_and_children(render, event_context);
    game_hierarchy.tick_self_and_children(render, event_context);

    renderer_instance.end_frame();

    // Bind window framebuffer for ImGui
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, window->width(), window->height());
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!window->is_imgui_initialized())
      window->initialize_imgui();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    // Optionally render the internal framebuffer to the screen
    if (settings.render_framebuffer_to_screen)
    {
      ImGui::SetNextWindowPos(ImVec2(0, 0));
      ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
      ImGui::Begin("##Framebuffer", nullptr,
                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                   ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs);
      ImVec2 size = ImGui::GetContentRegionAvail();
      ImTextureID texture_id = (ImTextureID)(intptr_t)framebuffer->get_color_texture();
      ImGui::Image(texture_id, size, ImVec2(0, 1), ImVec2(1, 0));
      ImGui::End();
    }

    event_context.update_tick_phase(imgui_render);
    
    // We want GUI last because we want it to be over the top of the scene
    manager_hierarchy.tick_self_and_children(imgui_render, event_context);
    game_hierarchy.tick_self_and_children(imgui_render, event_context);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(window->raw());
  }
}

} // namespace Calgine