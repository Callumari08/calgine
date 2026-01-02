#include "app.h"

#include <SDL3/SDL_video.h>
#include <glad/gl.h>
#include <SDL3/SDL.h>
#include <stdexcept>
#include <iostream>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL.h>
#include "calgine/core/background_managers/hierarchy_manager.h"
#include "calgine/core/game_object.h"
#include "setup/window_handler.h"
#include "useful_funcs.h"

void App::main_loop()
{
  Hierarchy& hierarchy = Hierarchy::get_instance();
  GameObject& root = hierarchy.get_hierarchy_root();

  glClearColor(0.1f, 0.1f, 0.2f, 1.0f);


  // Instead of doing this here, I should implement a scene manager.
  root.tick_self_and_children(TickType::start);

  bool running = true;
  while (running) 
  {
    SDL_Event event;
    while (SDL_PollEvent(&event)) 
    {
      if (event.type == SDL_EVENT_QUIT)
      {
        running = false;
      }
    }

    root.tick_self_and_children(TickType::update);
    root.tick_self_and_children(TickType::late_update);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    SDL_GL_SwapWindow(WindowHandler::get_instance()->get_window());
  }
}

// Should only be used for setting up libraries such as SDL and OpenGL, nothing else.
void App::systems_init()
{
  if (!SDL_Init(SDL_INIT_VIDEO)) 
  {
    throw_sdl_error("SDL_Init Failed: ");
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

  WindowHandler* window_handler = WindowHandler::get_instance();

  window_handler->get_window();
  window_handler->get_gl_context();
  window_handler->set_vsync_state(VsyncState::enabled);

  std::cout << "Initializing GLAD2\n";

  if (!gladLoadGL((GLADloadfunc) SDL_GL_GetProcAddress)) 
  {
    throw std::runtime_error("Failed to load OpenGL via GLAD2");
  } 

  std::cout << "OpenGL Version " << glGetString(GL_VERSION) << std::endl;
}