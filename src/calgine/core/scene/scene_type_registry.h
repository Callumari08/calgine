#pragma once

#include "calgine/core/game_object.h"
namespace Calgine {

class SceneTypeRegistry
{
public:
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
private:
  using BehaviourFactory = std::function<Behaviour*(GameObject*)>;

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