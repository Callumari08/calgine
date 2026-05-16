#include "scene_type_registry.h"

namespace Calgine {

std::unordered_map<std::string, SceneTypeRegistry::BehaviourFactory> SceneTypeRegistry::behaviour_factories;

}
