#pragma once

#include "calgine/core/behaviour.h"
#include "calgine/core/game_object.h"
#include "calgine/core/scene/scene_load_context.h"
#include "calgine/core/scene/scene_loadable_behaviour.h"
#include "calgine/core/transform.h"
#include "calgine_pch.h"
#include "calgine_api.h"

#include <rapidjson/fwd.h>

namespace Calgine {

using SceneGameObjectFactory = std::function<GameObject*(GameObject&, const Transform&, const std::string&)>;
using SceneBehaviourFactory = std::function<Behaviour*(GameObject&, const rapidjson::Value&, SceneLoadContext&, bool)>;

class CALGINE_API SceneTypeRegistry
{
public:
  static SceneTypeRegistry& get_instance();
  static SceneTypeRegistry& get() { return get_instance(); }

  void register_game_object(std::string type_name, SceneGameObjectFactory factory);

  template<typename T_game_object>
  requires std::derived_from<T_game_object, GameObject>
  void register_game_object(const std::string& type_name)
  {
    register_game_object(type_name,
      [](GameObject& parent, const Transform& transform, const std::string& name) -> GameObject*
      {
        return &parent.instantiate_child<T_game_object>(transform, name);
      });
  }

  void register_behaviour(std::string type_name, SceneBehaviourFactory factory);

  template<typename T_behaviour>
  requires std::derived_from<T_behaviour, Behaviour>
  void register_behaviour(const std::string& type_name)
  {
    register_behaviour(type_name,
      [](GameObject& owner, const rapidjson::Value& data, SceneLoadContext& load_context, const bool defer_start) -> Behaviour*
      {
        T_behaviour* behaviour = defer_start
          ? owner.add_behaviour_deferred<T_behaviour>()
          : owner.add_behaviour<T_behaviour>();

        if (!behaviour)
          return nullptr;

        if constexpr (std::derived_from<T_behaviour, SceneLoadableBehaviour>)
        {
          behaviour->load_from_json(data, load_context);
        }

        return behaviour;
      });
  }

  GameObject* create_game_object(
    const std::string& type_name,
    GameObject& parent,
    const Transform& transform,
    const std::string& name) const;

  Behaviour* create_behaviour(
    const std::string& type_name,
    GameObject& owner,
    const rapidjson::Value& data,
    SceneLoadContext& load_context,
    bool defer_start = true) const;

private:
  SceneTypeRegistry();

  std::unordered_map<std::string, SceneGameObjectFactory> game_object_factories;
  std::unordered_map<std::string, SceneBehaviourFactory> behaviour_factories;
};

} // namespace Calgine
