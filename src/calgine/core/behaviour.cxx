#include "behaviour.h"
#include <cassert>
#include <iostream>

Behaviour::~Behaviour() 
{
  std::cout << "Behaviour deconstruct\n";
  game_object = nullptr;
}

void Behaviour::start_tick() {}
void Behaviour::update_tick() {}
void Behaviour::late_tick() {}

void Behaviour::on_attach_owner() {}
void Behaviour::on_destroy() {}

void Behaviour::attach_owner(GameObject* _game_object)
{
  assert(game_object == nullptr && "Behaviour already attached!");
  game_object = _game_object;
  on_attach_owner();
}

GameObject* Behaviour::get_game_object()
{
  assert(game_object && "Behaviour not attached yet");
  return game_object;
}