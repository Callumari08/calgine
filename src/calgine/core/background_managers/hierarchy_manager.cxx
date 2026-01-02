#include "hierarchy_manager.h"
#include "calgine/core/game_object.h"

Hierarchy::Hierarchy() : root() {}

Hierarchy& Hierarchy::get_instance()
{
  static_assert(std::is_default_constructible_v<RootGameObject>,
              "RootGameObject not default-constructible here");
  static Hierarchy instance;
  return instance;
}

GameObject& Hierarchy::get_hierarchy_root()
{
  return root;
}