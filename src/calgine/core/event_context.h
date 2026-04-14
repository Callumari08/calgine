#pragma once

#include "event_data.h"
#include "calgine/core/tick_type.h"
#include "log.h"
#include "calgine_api.h"
#include "calgine_pch.h"


namespace Calgine {

class CALGINE_API EventContext
{
public:
  template<typename EventT>
  requires std::derived_from<EventT, EventData>
  void submit(const EventT& event)
  {
    if (event.submit_tick == TickType::any)
    {
      Log::get_engine_logger()->warn("Event was published with TickType any! Ignoring event. (Did you forget to set submit_tick?)");
      return;
    }

    std::type_index type_index = std::type_index(typeid(EventT));
    active_events[type_index].emplace_back(std::make_unique<EventT>(event));
  }

  template<typename EventT>
  requires std::derived_from<EventT, EventData>
  std::vector<const EventT*> get_events() const
  {
    std::vector<const EventT*> result;
    auto iterator = active_events.find(std::type_index(typeid(EventT)));
    if (iterator == active_events.end())
      return result;
    for (const auto& element : iterator->second)
    {
      if (EventT* casted = dynamic_cast<EventT*>(element.get()))
        result.push_back(casted);
    }

    return result;
  }

private:
  void update_tick_phase(const TickType current_phase);

  std::unordered_map<std::type_index, std::vector<std::unique_ptr<EventData>>> active_events;

  TickType last_tick_phase = preloop;

  friend class App;
};

}