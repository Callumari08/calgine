#include "game_object.h"
#include "calgine/core/behaviour.h"
#include <memory>

void GameObject::tick_self_and_children(TickType tick_type)
{
  for (auto& [type, behaviour] : behaviours)
  {
    switch (tick_type) 
    {
      case TickType::update: 
        behaviour->update_tick();
        break;
      case TickType::late_update: 
        behaviour->late_tick();
        break;
      // We call start_tick last because start is only called once, which skips a check every frame, except for the first.
      case TickType::start: 
        behaviour->start_tick();
        break;
    }
  }

  for (auto& go : children)
  {
    go->tick_self_and_children(tick_type);
  }
}

