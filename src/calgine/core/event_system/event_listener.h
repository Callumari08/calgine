#pragma once

#include "calgine/core/event_system/event_dispatcher.h"
#include "calgine_api.h"

namespace Calgine {

class CALGINE_API EventListener
{
protected:
  std::vector<std::function<void()>> event_unsubscribers;

  template<typename EventType, typename ListenerType>
  void subscribe_event(ListenerType* listener, void (ListenerType::*method) (const EventType&))
  {
    EventDispatcher<EventType>::subscribe(listener, method);

    event_unsubscribers.emplace_back([listener] () {
      EventDispatcher<EventType>::unsubscribe(listener);
    });
  }

  virtual ~EventListener()
  {
    for (std::function<void ()> unsubscribe : event_unsubscribers)
    {
      unsubscribe();
    }
  }
};

}