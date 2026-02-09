#include "manager_hierarchy.h"
#include "calgine/core/game_object.h"

namespace Calgine {

ManagerHierarchy::ManagerHierarchy() : root("ManagerRoot") 
{
}

ManagerHierarchy& ManagerHierarchy::get_instance()
{
  static ManagerHierarchy instance;
  return instance;
}

GameObject& ManagerHierarchy::get_hierarchy_root()
{
  return root;
}

}