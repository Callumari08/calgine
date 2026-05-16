#include "scene_manager.h"

#include "calgine/core/hierarchies/game_hierarchy.h"

namespace Calgine {

GameObject& SceneManager::create_scene_root(const std::string& scene_name, const Transform& transform)
{
  unload_active_scene();
  load_context.clear();

  active_scene_root = &GameHierarchy::get().instantiate_child(transform, scene_name);
  return *active_scene_root;
}

bool SceneManager::unload_active_scene()
{
  if (!active_scene_root)
    return false;

  active_scene_root->destroy();
  active_scene_root = nullptr;
  load_context.clear();
  return true;
}

void SceneManager::initialize_active_scene()
{
  if (!active_scene_root)
    return;

  load_context.resolve_deferred_links();
  active_scene_root->start_behaviours_recursive();
}

} // namespace Calgine
