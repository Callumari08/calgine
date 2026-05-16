#pragma once

#include "calgine/core/game_object.h"
#include "calgine_pch.h"
#include "calgine_api.h"

namespace Calgine {

class CALGINE_API SceneLoadContext
{
public:
  bool register_object(const std::string& id, GameObject& game_object);
  GameObject* find_object(const std::string& id) const;

  void defer(std::function<void()> deferred_link);
  void resolve_deferred_links();
  void clear();

private:
  std::unordered_map<std::string, GameObject*> game_objects_by_id;
  std::vector<std::function<void()>> deferred_links;
};

} // namespace Calgine
