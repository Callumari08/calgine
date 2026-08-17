#pragma once

#include "behaviour_registry.h"

namespace Calgine {

static std::unordered_map<std::string, BehaviourFactory>& registry()
{
  static std::unordered_map<std::string, BehaviourFactory> registry;
  return registry;
}

static std::mutex& registry_mutex()
{
  static std::mutex mutex;
  return mutex;
}

void BehaviourRegistry::register_behaviour(const std::string& name, BehaviourFactory factory) {
  std::lock_guard<std::mutex> lk(registry_mutex());
  registry()[name] = std::move(factory);
}

BehaviourFactory BehaviourRegistry::get_factory(const std::string& name) {
  std::lock_guard<std::mutex> lk(registry_mutex());
  auto it = registry().find(name);
  if (it == registry().end()) return nullptr;
  return it->second;
}

std::vector<std::string> BehaviourRegistry::list_registered() {
  std::lock_guard<std::mutex> lk(registry_mutex());
  std::vector<std::string> out; out.reserve(registry().size());
  for (auto &kv : registry()) out.push_back(kv.first);
  return out;
}

}