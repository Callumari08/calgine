#include "game_hierarchy.h"
#include "calgine/core/game_object.h"

namespace Calgine {

GameHierarchy::GameHierarchy() : root("GameRoot") 
{
}

GameHierarchy& GameHierarchy::get_instance()
{
  static GameHierarchy instance;
  return instance;
}

GameObject& GameHierarchy::get_hierarchy_root()
{
  return root;
}

} // namespace Calgine