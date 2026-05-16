#pragma once

#include "calgine_pch.h"
#include "calgine/core/game_object.h"

namespace Calgine {

class SceneLoadContext
{
private:
  // Map targetname (scene ID) to GameObject
  std::unordered_map<std::string, GameObject*> targetname_to_object;
  
  // Callbacks queued during loading, executed after all objects created
  std::vector<std::function<void()>> deferred_links;

public:
  // Register object by its targetname (unique ID in .map file)
  void register_object(const std::string& target_name, GameObject* gameobject) {
    if (targetname_to_object.count(target_name)) {
      Log::get_engine_logger()->warn("Duplicate targetname: {}, overriding with new GameObject.", target_name);
    }
    targetname_to_object[target_name] = gameobject;
  }
  
  // Look up object by its targetname
  GameObject* get_object(const std::string& target_name) const {
    auto it = targetname_to_object.find(target_name);
    if (it == targetname_to_object.end()) {
      Log::get_engine_logger()->error("Could not find target name: {}", target_name);
      return nullptr;
    }
    return it->second;
  }
  
  bool check_has_object(const std::string& targetname) const {
    return targetname_to_object.count(targetname) > 0;
  }
  
  // Queue a callback to run after scene is built
  void defer_link(std::function<void()> callback) {
    deferred_links.push_back(callback);
  }
  
  // Execute all deferred callbacks (called after all objects created)
  void resolve_deferred_links() {
    for (const auto& callback : deferred_links) {
      try {
        callback();
      } catch (const std::exception& e) {
        Log::get_engine_logger()->error("Deferred link failed: {}", e.what());
        // Continue with other links (best-effort)
      }
    }
    deferred_links.clear();
  }
};

}