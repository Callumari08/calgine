#pragma once

#include "calgine_pch.h"
#include "calgine_api.h"

namespace Calgine {

template<typename EventType>
class CALGINE_API EventDispatcher
{
public:
  template<typename ListenerType>
  static void subscribe(ListenerType* listener, void (ListenerType::*method)(const EventType&))
  {
    listeners.emplace_back({
      listener,
      [listener, method](const EventType& event) {
        (listener->*method)(event);
      }
    });
  }

  static void unsubscribe(void* listener)
  {
    std::erase_if(listeners, [listener](const ListenerNode& node) {
      return node.listener_ptr == listener;
    });
  }

  static void dispatch(const EventType& event)
  {
    for (auto& node : listeners)
    {
      node.callback(event);
    }
  }

  static void queue_event(const EventType& event)
  {
    pending_events.push(event);
  }

  static void process_queue()
  {
    while (!pending_events.empty())
    {
      dispatch(pending_events.front());
      pending_events.pop();
    }
  }

private:
  struct ListenerNode 
  {
    void* listener_ptr;
    std::function<void(const EventType&)> callback;
  };

  static inline std::vector<ListenerNode> listeners;
  static inline std::queue<EventType> pending_events;
};

} // namespace Calgine