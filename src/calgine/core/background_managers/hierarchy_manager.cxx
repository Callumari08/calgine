#include "hierarchy_manager.h"
#include "calgine/core/game_object.h"

Hierarchy& Hierarchy::get_instance()
{
  static Hierarchy instance;
  return instance;
}

GameObject& Hierarchy::get_hierarchy_root()
{
  return root;
}