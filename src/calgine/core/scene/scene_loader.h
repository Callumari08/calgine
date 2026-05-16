#pragma once

#include "calgine/core/game_object.h"
#include "scene_load_context.h"
#include "scene_loadable_behaviour.h"
#include "map_parser.h"
#include "calgine_api.h"

namespace Calgine {

class CALGINE_API SceneLoader
{
public:
  // Load scene from .map file - creates GameObjects and Behaviours
  static GameObject* load_scene_from_map(const std::string& map_filepath, GameObject* parent = nullptr);

private:
  // Instantiate a single entity from map properties
  static GameObject* instantiate_entity(const MapEntity& entity, GameObject* parent, SceneLoadContext& context);
};

}