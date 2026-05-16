#include "scene_loader.h"
#include "scene_type_registry.h"
#include "calgine/core/log.h"

namespace Calgine {

GameObject* SceneLoader::instantiate_entity(const MapEntity& entity, GameObject* parent, SceneLoadContext& context)
{
  MapEntityProperties properties(entity.properties);

  // Get required properties
  std::string classname = properties.get_string("classname");
  if (classname.empty())
  {
    Log::get_engine_logger()->warn("Entity missing 'classname' property");
    return nullptr;
  }

  std::string targetname = properties.get_string("targetname");

  // Parse origin (position)
  glm::vec3 origin = properties.get_vector3("origin");
  Transform transform(origin, glm::vec3(0), glm::vec3(1));

  // Instantiate GameObject
  GameObject* game_object = new GameObject(parent, transform, classname);

  // Register in context if it has a targetname
  if (!targetname.empty())
  {
    context.register_object(targetname, game_object);
  }

  // Create behaviour via registry (deferred startup)
  Behaviour* behaviour = SceneTypeRegistry::create_behaviour(classname, game_object);
  if (!behaviour)
  {
    Log::get_engine_logger()->error("Failed to create behaviour: {}", classname);
    delete game_object;
    return nullptr;
  }

  // Load behaviour from map entity properties
  auto loadable = dynamic_cast<SceneLoadableBehaviour*>(behaviour);
  if (loadable)
  {
    loadable->load_from_map_entity(properties, context);
  }

  return game_object;
}

GameObject* SceneLoader::load_scene_from_map(const std::string& map_filepath, GameObject* parent)
{
  // Parse .map file
  std::vector<MapEntity> entities = MapParser::parse(map_filepath);
  if (entities.empty())
  {
    Log::get_engine_logger()->error("No entities found in map file: {}", map_filepath);
    return nullptr;
  }

  // Create temporary loading context
  SceneLoadContext context;

  // Create root GameObject for the scene
  GameObject* scene_root = new GameObject(parent, Transform(glm::vec3(0, 0, 0)), "scene_root");

  // Phase 1 & 2: Discovery and Creation - instantiate all entities
  for (const auto& entity : entities)
  {
    GameObject* game_object = instantiate_entity(entity, scene_root, context);
    if (!game_object)
    {
      Log::get_engine_logger()->warn("Failed to instantiate entity with classname: {}", entity.properties.count("classname") ? entity.properties.at("classname") : "unknown");
    }
  }

  // Phase 3: Linking - resolve cross-references
  context.resolve_deferred_links();

  // Phase 4: Activation - start all behaviours
  scene_root->start_behaviours_recursive();

  return scene_root;
}

}
