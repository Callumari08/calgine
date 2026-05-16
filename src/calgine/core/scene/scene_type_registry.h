#pragma once

#include "calgine/core/game_object.h"
#include "calgine_api.h"

namespace Calgine {

class CALGINE_API SceneTypeRegistry
{
public:
  using BehaviourFactory = std::function<Behaviour*(GameObject*)>;

  static void discover_cpp_behaviours()
  {
    if (behaviour_factories.empty())
    {
      Log::get_engine_logger()->error("No behaviours registered. Did static initializers run?");
    }
  }

  static Behaviour* create_behaviour(const std::string& type_name, GameObject* owner)
  {
    auto iterator = behaviour_factories.find(type_name);
    if (iterator == behaviour_factories.end()) {
      Log::get_engine_logger()->error("Unknown Behaviour Type: {}", type_name);
      return nullptr;
    }
    return iterator->second(owner);
  }

  static void register_behaviour(const std::string& type_name, BehaviourFactory factory)
  {
    if (behaviour_factories.count(type_name)) {
      Log::get_engine_logger()->warn("Behaviour type already registered: {}", type_name);
      return;
    }
    behaviour_factories[type_name] = factory;
  }

private:
  static std::unordered_map<std::string, BehaviourFactory> behaviour_factories;
};

// Helper macro to register behaviours at startup (one line per behaviour)
#define REGISTER_BEHAVIOUR_STATIC_INIT(BehaviourClass) \
  static struct BehaviourClass##_Registration { \
    BehaviourClass##_Registration() { \
      SceneTypeRegistry::register_behaviour( \
        #BehaviourClass, \
        [](GameObject* owner) -> Behaviour* { \
          return owner->add_behaviour<BehaviourClass>(); \
        }); \
    } \
  } BehaviourClass##_reg;
}