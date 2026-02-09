#pragma once

#include "calgine_pch.h"
#include "calgine_api.h"

namespace Calgine {

class CALGINE_API EventManager
{
private:
  static inline std::vector<std::function<void()>> queue_processors;

public:
  template<typename EventType>
  static void register_queue_processor();

  static void process_all_queues()
  {
    for (auto& processor : queue_processors)
    {
      processor();
    }
  }
};

} // namespace Calgine

// Implementation in header to avoid forward declaration issues
#include "event_dispatcher.h"

namespace Calgine {

template<typename EventType>
inline void EventManager::register_queue_processor()
{
  queue_processors.push_back([]() {
    EventDispatcher<EventType>::process_queue();
  });
}

} // namespace Calgine

