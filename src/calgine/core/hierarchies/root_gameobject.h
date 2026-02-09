#pragma once

#include "calgine/core/game_object.h"

namespace Calgine {

class RootGameObject : public GameObject
{
public:
  RootGameObject(std::string root_name) : GameObject(nullptr, Transform::zero(), root_name)
  {

  }

private:
  using GameObject::set_parent;
};

}