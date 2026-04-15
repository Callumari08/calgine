#pragma once

#include "../game_object.h"
#include "root_gameobject.h"
#include "calgine_api.h"

namespace Calgine {
// It might make more sense for GameHierarchy and ManagerHierarchy to both derive from a class called "Hierarchy"... but oh well

//
class CALGINE_API GameHierarchy final
{
public:
  static GameHierarchy& get_instance();

  static inline GameObject& get() { return GameHierarchy::get_instance().get_hierarchy_root(); };

  GameObject& get_hierarchy_root();

  GameHierarchy(const GameHierarchy&) = delete;
  GameHierarchy& operator=(const GameHierarchy&) = delete;

private:
  RootGameObject root;

  GameHierarchy();
};

}