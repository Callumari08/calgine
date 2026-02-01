#pragma once

#include "../game_object.h"
#include "calgine/core/transform.h"
#include "calgine_api.h"

namespace Calgine {

class RootGameObject : public GameObject
{
public:
  RootGameObject() : GameObject(nullptr, Transform::zero()) {}
  std::string get_name() override
  {
    return "Root";
  }
private:
  using GameObject::set_parent;
};

class CALGINE_API Hierarchy final
{
private:
  RootGameObject root;

  Hierarchy();

public:
  static Hierarchy& get_instance();

  GameObject& get_hierarchy_root();

  Hierarchy(const Hierarchy&) = delete;
  Hierarchy& operator=(const Hierarchy&) = delete;
};

} // namespace Calgine