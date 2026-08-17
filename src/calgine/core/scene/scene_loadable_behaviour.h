#pragma once

#include "map_entity_properties.h"
#include "scene_load_context.h"

namespace Calgine {

class SceneLoadableBehaviour
{
public:
  virtual ~SceneLoadableBehaviour() = default;
  
  // Called by scene loader to populate behaviour from map entity properties
  virtual void load_from_map_entity(const MapEntityProperties& properties, SceneLoadContext& context) = 0;
};

}