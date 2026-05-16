#include "scene_type_registry.h"

#include "calgine/core/log.h"

namespace Calgine {

SceneTypeRegistry::SceneTypeRegistry()
{
  register_game_object<GameObject>("GameObject");
}

SceneTypeRegistry& SceneTypeRegistry::get_instance()
{
  static SceneTypeRegistry instance;
  return instance;
}

void SceneTypeRegistry::register_game_object(std::string type_name, SceneGameObjectFactory factory)
{
  if (type_name.empty())
  {
    Log::get_engine_logger()->error("Cannot register a scene GameObject factory with an empty type name");
    return;
  }

  if (!factory)
  {
    Log::get_engine_logger()->error("Cannot register scene GameObject factory `{}` with an empty callback", type_name);
    return;
  }

  game_object_factories.insert_or_assign(std::move(type_name), std::move(factory));
}

void SceneTypeRegistry::register_behaviour(std::string type_name, SceneBehaviourFactory factory)
{
  if (type_name.empty())
  {
    Log::get_engine_logger()->error("Cannot register a scene Behaviour factory with an empty type name");
    return;
  }

  if (!factory)
  {
    Log::get_engine_logger()->error("Cannot register scene Behaviour factory `{}` with an empty callback", type_name);
    return;
  }

  behaviour_factories.insert_or_assign(std::move(type_name), std::move(factory));
}

GameObject* SceneTypeRegistry::create_game_object(
  const std::string& type_name,
  GameObject& parent,
  const Transform& transform,
  const std::string& name) const
{
  const auto it = game_object_factories.find(type_name);
  if (it == game_object_factories.end())
  {
    Log::get_engine_logger()->error("No scene GameObject factory registered for type `{}`", type_name);
    return nullptr;
  }

  return it->second(parent, transform, name);
}

Behaviour* SceneTypeRegistry::create_behaviour(
  const std::string& type_name,
  GameObject& owner,
  const rapidjson::Value& data,
  SceneLoadContext& load_context,
  const bool defer_start) const
{
  const auto it = behaviour_factories.find(type_name);
  if (it == behaviour_factories.end())
  {
    Log::get_engine_logger()->error("No scene Behaviour factory registered for type `{}`", type_name);
    return nullptr;
  }

  return it->second(owner, data, load_context, defer_start);
}

} // namespace Calgine
