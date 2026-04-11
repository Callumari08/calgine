#include "event_context.h"

namespace Calgine {

void EventContext::update_tick_phase(const TickType current_phase)
{
  for (auto& [type_index, events] : active_events)
  {
    auto iterator = std::remove_if(events.begin(), events.end(),
      [current_phase](const std::unique_ptr<EventData>& event) {
        return event->submit_tick == current_phase ||
          (current_phase == TickType::final && event->submit_tick == TickType::preloop);
      });
    events.erase(iterator, events.end());
  }
   
  last_tick_phase = current_phase;
}
}