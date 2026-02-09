#pragma once

#include "calgine_api.h"

namespace Calgine {

enum EventType 
{
  game,
  manager,
  all,
};

class CALGINE_API Event
{
public:
  Event(EventType event_type) : type(event_type)
  {

  }

  inline void mark_handled() { handled = true; }
  inline bool is_handled() const { return handled; }

  inline EventType get_type() { return type; }

protected:
  bool handled = false;
  EventType type;
};

}