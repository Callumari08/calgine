#include "game_object.h"
#include "calgine/core/behaviour.h"
#include <cassert>
#include <memory>
#include <algorithm>

GameObject::GameObject(GameObject* _parent) : parent(_parent) {}

GameObject::~GameObject()
{
  for (auto& child : children)
  {
    child->set_parent(nullptr);
  }

  assert(parent == nullptr && "GameObject destroyed while still parented");
}

void GameObject::tick_self_and_children(TickType tick_type)
{
  for (auto& [type, behaviour] : behaviours)
  {
    switch (tick_type) 
    {
      case TickType::update: 
        behaviour->update_tick();
        break;
      case TickType::late_update: 
        behaviour->late_tick();
        break;
      // We call start_tick last because start is only called once, which skips a check every frame, except for the first.
      case TickType::start: 
        behaviour->start_tick();
        break;
    }
  }

  for (auto& go : children)
  {
    go->tick_self_and_children(tick_type);
  }
}

void GameObject::set_parent(GameObject* _parent)
{
  if (parent == _parent)
    return;

  std::unique_ptr<GameObject> owned_self;

  if (parent)
  {
    auto& siblings = parent->children;

    auto it = std::find_if(
      siblings.begin(),
      siblings.end(),
          [this](const std::unique_ptr<GameObject>& child)
          {
            return child.get() == this;
          }
      );

    assert(it != siblings.end());

    owned_self = std::move(*it);
    siblings.erase(it);
  }

    parent = _parent;

  if (parent)
  {
    parent->children.push_back(std::move(owned_self));
  }
}

std::optional<std::reference_wrapper<GameObject>> GameObject::get_parent() const
{
  if (parent) 
    return *parent;

  return std::nullopt;
}