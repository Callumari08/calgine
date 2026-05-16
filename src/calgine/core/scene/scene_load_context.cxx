#include "scene_load_context.h"

#include "calgine/core/log.h"

namespace Calgine {

bool SceneLoadContext::register_object(const std::string& id, GameObject& game_object)
{
  if (id.empty())
    return true;

  auto [it, inserted] = game_objects_by_id.emplace(id, &game_object);
  if (!inserted)
  {
    Log::get_engine_logger()->error("SceneLoadContext already contains an object with id `{}`", id);
    return false;
  }

  return true;
}

GameObject* SceneLoadContext::find_object(const std::string& id) const
{
  const auto it = game_objects_by_id.find(id);
  if (it == game_objects_by_id.end())
    return nullptr;

  return it->second;
}

void SceneLoadContext::defer(std::function<void()> deferred_link)
{
  deferred_links.emplace_back(std::move(deferred_link));
}

void SceneLoadContext::resolve_deferred_links()
{
  std::vector<std::function<void()>> pending_links = std::move(deferred_links);
  deferred_links.clear();

  for (auto& deferred_link : pending_links)
  {
    deferred_link();
  }
}

void SceneLoadContext::clear()
{
  game_objects_by_id.clear();
  deferred_links.clear();
}

} // namespace Calgine
