#pragma once

#include "behaviour_registry.h"
#include "behaviour_factory_helpers.h"

#define CALGINE_REGISTER_BEHAVIOUR(BehaviourClass, nameStr) \
  namespace { \
    struct BehaviourClass##_AutoReg { \
      BehaviourClass##_AutoReg() { \
        Calgine::BehaviourRegistry::register_behaviour(nameStr, Calgine::make_behaviour_factory<BehaviourClass>()); \
      } \
    }; \
    static BehaviourClass##_AutoReg BehaviourClass##_autoRegInstance; \
  }