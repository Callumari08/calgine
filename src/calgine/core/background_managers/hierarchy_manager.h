#pragma once

#include "../game_object.h"

class RootGameObject : public GameObject
{
public:
  RootGameObject() : GameObject(nullptr) {}
  std::string get_name() override
  {
    return "Root";
  }
private:
  using GameObject::set_parent;
};

class Hierarchy final
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

