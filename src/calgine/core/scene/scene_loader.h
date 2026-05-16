#pragma once

#include "calgine/core/game_object.h"
namespace Calgine {

class SceneLoader
{
  void load_scene_from_map(const std::string& map_filepath);

  void initialize_active_scene();

  void unload_active_scene();

  GameObject& get_active_scene_root() const;
};
}