#pragma once
#include "behaviour_registry.h"
#include "data_serialization_interface.h"
#include "calgine_pch.h"

namespace Calgine {

class GameObject;
class Behaviour;

// trait: does T have void deserialize(const DataSerializationInterface*) ?
template<typename T, typename = void>
struct has_deserialize_from_paramview : std::false_type {};

template<typename T>
struct has_deserialize_from_paramview<T, std::void_t<
  decltype(std::declval<T>().deserialize(std::declval<const DataSerializationInterface*>()))>>
  : std::true_type {};

template<typename T>
BehaviourFactory make_behaviour_factory() {
  return [](GameObject* owner, const DataSerializationInterface* params) -> Behaviour* {
    T* b = owner->template add_behaviour<T>();
    if constexpr (has_deserialize_from_paramview<T>::value) {
      b->deserialize(params);
    }
    return b;
  };
}

} 