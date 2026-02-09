#pragma once

#include "../game_object.h"
#include "root_gameobject.h"
#include "calgine_api.h"

namespace Calgine {
// It might make more sense for GameHierarchy and ManagerHierarchy to both derive from a class called "Hierarchy"... but oh well

//
class CALGINE_API GameHierarchy final
{
private:
  RootGameObject root;

  GameHierarchy();

public:
  static GameHierarchy& get_instance();

  GameObject& get_hierarchy_root();

  GameHierarchy(const GameHierarchy&) = delete;
  GameHierarchy& operator=(const GameHierarchy&) = delete;
};

} // namespace Calgine