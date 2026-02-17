#include "game_object.h"
#include "calgine/core/behaviour.h"
#include "calgine/core/transform.h"

namespace Calgine {

uint32_t GameObject::num_game_objects = 0;

GameObject::GameObject(GameObject* _parent, const Transform _transform, const std::string _name) 
  : transform(_transform), name(_name), parent(_parent)
{
  num_game_objects++;
}
GameObject::~GameObject()
{
  Log::get_engine_logger()->warn("GameObject Deleting; name of: {}", name);
}

void GameObject::destroy()
{
  if (destroyed) return;
  destroyed = true;

  for (std::unique_ptr<GameObject>& child : children)
  {
    if (child) child->destroy();
  }

  for (auto& [type, behaviour] : behaviours)
  {
    if (behaviour) behaviour->on_destroy();
  }

  name.clear();

  --num_game_objects;

  if (parent)
  {
    // claims ownership
    std::unique_ptr<GameObject> owner = parent->detach_child(this);

    if (owner) 
      pending_deletes.emplace_back(std::move(owner));

    parent = nullptr;
  }
}

std::unique_ptr<GameObject> GameObject::detach_child(GameObject* child)
{
  if (!child) return nullptr;

  auto it = std::find_if(
    children.begin(), children.end(),
    [child](const std::unique_ptr<GameObject>& p) { return p.get() == child; }
  );

  if (it == children.end()) return nullptr;

  // take ownership out of the parent's vector
  std::unique_ptr<GameObject> result = std::move(*it);
  // erase the slot (vector shrinks)
  children.erase(it);

  // sever back-pointer (child still lives until result destroyed)
  if (result) result->parent = nullptr;

  return result;
}

bool GameObject::remove_child_immediate(GameObject* child)
{
  if (!child) return false;

  auto it = std::find_if(
    children.begin(), children.end(),
    [child](const std::unique_ptr<GameObject>& ptr) { return ptr.get() == child; }
  );

  if (it == children.end()) return false;

  (*it)->parent = nullptr;
  children.erase(it);

  return true;
}

void GameObject::process_pending_deletes()
{
  pending_deletes.clear();
}

void GameObject::set_active(const bool _enabled)
{
  if (_enabled == enabled)
    return;

  if (_enabled && !parent->is_enabled())
  {
    Log::get_app_logger()->warn("Attempted to enable child of a disabled parent");
    return;
  }

  enabled = _enabled;

  for (auto& go : children)
  {
    go->set_active(_enabled);
  }
}

void GameObject::tick_self_and_children(const TickType tick_type)
{
  if (!is_enabled())
    return;

  for (auto& [type, behaviour] : behaviours)
  {
    if (!behaviour)
      continue;

    switch (tick_type) 
    {
      case TickType::update:
        behaviour->update_tick();
        break;
      case TickType::late_update: 
        behaviour->late_tick();
        break;
      case TickType::render:
        behaviour->render_tick();
        break;
      case TickType::imgui_render:
        behaviour->imgui_render_tick();
        break;
      case TickType::preloop:
        behaviour->preloop_tick();
        break;
    }
  }

  // Use index-based iteration to handle children vector reallocation during ticking
  for (size_t i = 0; i < children.size(); ++i)
  {
    children[i]->tick_self_and_children(tick_type);
  }
}


void GameObject::set_parent(GameObject* _parent)
{
  if (parent == _parent)
    return;

  std::unique_ptr<GameObject> owned_self;

  if (parent)
  {
    std::vector<std::unique_ptr<GameObject>>& siblings = parent->children;

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
    parent->children.emplace_back(std::move(owned_self));
  }
}

std::optional<std::reference_wrapper<GameObject>> GameObject::get_parent() const
{
  if (parent) 
    return *parent;

  return std::nullopt;
}

void GameObject::set_name(std::string _name)
{
  name = _name;
}

} // namespace Calgine