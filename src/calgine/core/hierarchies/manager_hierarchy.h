#pragma once

#include "calgine_api.h"
#include "root_gameobject.h"
#include "calgine/core/game_object.h"

namespace Calgine {

class CALGINE_API ManagerHierarchy final
{
public:
  static ManagerHierarchy& get_instance();

  static inline GameObject& get() { return ManagerHierarchy::get_instance().get_hierarchy_root(); };

  GameObject& get_hierarchy_root();

  ManagerHierarchy(const ManagerHierarchy&) = delete;
  ManagerHierarchy& operator=(const ManagerHierarchy&) = delete;
private:
  RootGameObject root;

  ManagerHierarchy();
};

}