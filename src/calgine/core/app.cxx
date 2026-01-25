#include "app.h"

#include <SDL3/SDL_video.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL.h>
#include <cassert>
#include <cstdint>
#include <glad/gl.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
#include "SDL3/SDL_events.h"
#include "calgine/core/background_managers/hierarchy_manager.h"
#include "calgine/core/game_object.h"
#include "calgine/core/renderer/buffers/index_buffer.h"
#include "calgine/core/renderer/buffers/vertex_buffer.h"
#include "calgine/core/renderer/vertex.h"
#include "window/window_handler.h"
#include "useful_funcs.h"
#include "log.h"
#include "renderer/shader.h"
#include <glm/glm.hpp>

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

// Should only be used for setting up libraries such as SDL and OpenGL, nothing else.
void App::systems_init()
{
  // Loggers
  Log::init(settings.app_name);

  // SDL
  if (!SDL_Init(SDL_INIT_VIDEO)) 
  {
    throw_sdl_error("SDL_Init Failed: ");
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
    std::string msg = "Failed to load OpenGL via GLAD2 (is there a window?)";
    Log::get_engine_logger()->error(msg);
    throw std::runtime_error(msg);
  } 

  std::string gl_version = (const char*)glGetString(GL_VERSION);
  Log::get_engine_logger()->info("OpenGL Version {}", gl_version);
  
  std::string renderer = (const char*)glGetString(GL_RENDERER);
  Log::get_engine_logger()->info("Renderer Device: {}", renderer);

  init_buffers();
  init_imgui();
}

void App::init_buffers()
{
  Vertex vertices[] = {
      {
          {1.0f, 0.0f, 0.0f, 1.0f},
          {-0.5f, -0.5f, 0.0f},
          {0.0f,  0.0f,  1.0f},
          {0.0f,  0.0f}
      },
      {
          {0.0f, 1.0f, 0.0f, 1.0f},
          { 0.5f, -0.5f, 0.0f},
          { 0.0f,  0.0f, 1.0f},
          { 1.0f,  0.0f}
      },
      {
          {0.0f, 0.0f, 1.0f, 1.0f},
          { 0.0f,  0.5f, 0.0f},
          { 0.0f,  0.0f, 1.0f},
          { 0.5f,  1.0f}
      }
  };
  vertex_array = std::make_unique<VertexArray>();
  vertex_array->bind();

  vertex_buffer = std::make_unique<VertexBuffer>(vertices);
  vertex_buffer->bind();

  vertex_array->set_layout();

  uint32_t indices[] = { 0, 1, 2 };
  index_buffer = std::make_unique<IndexBuffer>(
      indices,
      sizeof(indices) / sizeof(uint32_t)
  );

  index_buffer->bind();

  std::string vertex_source = R"(
      #version 450 core
      layout(location = 0) in vec3 a_Position;
      out vec3 v_Position;
      void main()
      {
          v_Position = a_Position;
          gl_Position = vec4(a_Position, 1.0);
      }
  )";
  
  std::string fragment_source = R"(
      #version 450 core
      layout(location = 0) out vec4 o_color;
      in vec3 v_Position;
      void main()
      {
          o_color = vec4(v_Position, 1.0);
      }
  )";
  shader = std::make_unique<Shader>(vertex_source, fragment_source);
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
  Hierarchy& hierarchy = Hierarchy::get_instance();
  GameObject& root = hierarchy.get_hierarchy_root();


  // Instead of doing this here, I should implement a scene manager.
  //
  // (Later comment) I actually think it'd be a good idea to keep root for as long as the program runs,
  // and the GameObjects directly below root is the scene, root can have its own behaviours that can
  // manage the scenes. 
  root.tick_self_and_children(TickType::preloop);

  bool running = true;
  while (running) 
  {
    handle_sdl_events(running);

    root.tick_self_and_children(TickType::update);
    root.tick_self_and_children(TickType::late_update);

    render_windows(root);

    GameObject::process_pending_deletes();

    WindowHandler* window_handler = WindowHandler::get_instance();

    window_handler->cleanup_closed_windows();
    
    if (window_handler->get_windows().empty())
      running = false;
  }
}

void App::handle_sdl_events(bool& running)
{
  SDL_Event event;
  while (SDL_PollEvent(&event)) 
  {
    ImGui_ImplSDL3_ProcessEvent(&event);

    switch (event.type) 
    {
      case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
      {
        if (Window* window = WindowHandler::get_instance()->get_window(event.window.windowID))
          window->request_close();
        break;
      }
      case SDL_EVENT_QUIT:
        running = false;
        break;

      default: break;
    }
  }
}

// This exists because right now, we can only do OpenGL stuff on one window, this will (probably) change.
void App::do_stuff_on_single_window()
{
  shader->bind();
  vertex_array->bind();
  glDrawElements(GL_TRIANGLES, index_buffer->get_count(), GL_UNSIGNED_INT, nullptr);

}

void App::render_windows(GameObject& root)
{
  for (auto& window : WindowHandler::get_instance()->get_windows())
  {
    if (window->should_close())
      continue;

    SDL_GL_MakeCurrent(window->raw(), window->get_context());

    glViewport(0, 0, window->width(), window->height());
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!window->is_imgui_initialized())
      window->initialize_imgui();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    do_stuff_on_single_window();

    root.tick_self_and_children(TickType::imgui_render);

    // render tick here

    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(window->raw());
  }
}





} // namespace Calgine