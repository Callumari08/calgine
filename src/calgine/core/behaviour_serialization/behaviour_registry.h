#pragma once

#include <calgine_pch.h>
#include "data_serialization_interface.h"
#include "calgine_api.h"

namespace Calgine {

class GameObject;
class Behaviour;
using BehaviourFactory = std::function<Behaviour*(GameObject*, DataSerializationInterface const*)>;
class CALGINE_API BehaviourRegistry
{
public:
  static void register_behaviour(const std::string& name, const BehaviourFactory factory);
  static BehaviourFactory get_factory(const std::string& name);
  static std::vector<std::string> list_registered();
};

}