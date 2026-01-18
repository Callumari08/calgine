#include "app.h"

#include <SDL3/SDL_video.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL.h>
#include <glad/gl.h>
#include "SDL3/SDL_events.h"
#include "calgine/core/background_managers/hierarchy_manager.h"
#include "calgine/core/game_object.h"
#include "setup/window_handler.h"
#include "useful_funcs.h"
#include "log.h"

namespace Calgine {

/*App* App::create()
{
}*/

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

    render_windows();

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

void App::render_windows()
{
  for (auto& window : WindowHandler::get_instance()->get_windows())
  {
    if (window->should_close())
      continue;

    SDL_GL_MakeCurrent(window->raw(), window->get_context());

    glViewport(0, 0, window->width(), window->height());

    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // render tick here

    SDL_GL_SwapWindow(window->raw());
  }
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

  std::string gl_version = convert_GLubyte_ptr_to_str(glGetString(GL_VERSION));
  Log::get_engine_logger()->info("OpenGL Version {}", gl_version);
  
  std::string renderer = convert_GLubyte_ptr_to_str(glGetString(GL_RENDERER));
  Log::get_engine_logger()->info("Renderer Device: {}", renderer);
}

} // namespace Calgine