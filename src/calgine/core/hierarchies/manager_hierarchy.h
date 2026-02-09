#pragma once

#include "calgine_api.h"
#include "root_gameobject.h"
#include "calgine/core/game_object.h"

namespace Calgine {

class CALGINE_API ManagerHierarchy final
{
private:
  RootGameObject root;

  ManagerHierarchy();

public:
  static ManagerHierarchy& get_instance();

  GameObject& get_hierarchy_root();

  ManagerHierarchy(const ManagerHierarchy&) = delete;
  ManagerHierarchy& operator=(const ManagerHierarchy&) = delete;
};

}