#include "calgine_pch.h"

#include "behaviour.h"
#include "game_object.h"


namespace Calgine {

Behaviour::~Behaviour() 
{
  game_object = nullptr;
}

void Behaviour::preloop_tick() {}
void Behaviour::start_tick() {}
void Behaviour::update_tick() {}
void Behaviour::late_tick() {}
void Behaviour::render_tick() {}
void Behaviour::imgui_render_tick() {}

void Behaviour::on_destroy() {}

bool Behaviour::attach_owner(GameObject* _game_object)
{
  if (game_object != nullptr)
  {
    Log::get_engine_logger()->error("Behaviour already attached!");
    return false;
  }
  game_object = _game_object;
  start_tick();

  return true;
}

GameObject* Behaviour::get_game_object() 
{
  assert(game_object && "Behaviour not attached yet");
  return game_object;
}

Transform& Behaviour::get_transform() 
{
  return get_game_object()->get_transform();
}

} // namespace Calgine