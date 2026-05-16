#pragma once

#include "calgine/core/behaviour.h"
#include "calgine/core/scene/scene_load_context.h"
#include "calgine/core/transform.h"
#include "calgine_api.h"

namespace Calgine {

class CALGINE_API SceneManager : public Behaviour
{
public:
  GameObject& create_scene_root(const std::string& scene_name, const Transform& transform = Transform::zero());
  bool unload_active_scene();
  void initialize_active_scene();

  GameObject* get_active_scene_root() const { return active_scene_root; }
  SceneLoadContext& get_load_context() { return load_context; }

private:
  GameObject* active_scene_root = nullptr;
  SceneLoadContext load_context;
};

} // namespace Calgine
