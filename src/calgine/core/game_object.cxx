#include "game_object.h"
#include "calgine/core/behaviour.h"
#include "calgine/core/event_context.h"
#include "calgine/core/transform.h"
#include "behaviour_serialization/behaviour_registry.h"

namespace Calgine {

uint32_t GameObject::num_game_objects = 0;

GameObject::GameObject(GameObject* _parent, const Transform _transform, const std::string _name) 
  : transform(_transform), name(_name), parent(_parent)
{
  num_game_objects++;
}
GameObject::~GameObject()
{
  children.clear();
  behaviours.clear();
}

void GameObject::destroy()
{
  if (destroyed) return;
  destroyed = true;

  // Each child's destroy() detaches it from our children vector, so iterate over a snapshot
  // instead of the vector itself (which would skip children and read past the end).
  std::vector<GameObject*> children_to_destroy;
  children_to_destroy.reserve(children.size());
  for (std::unique_ptr<GameObject>& child : children)
  {
    if (child) children_to_destroy.push_back(child.get());
  }

  for (GameObject* child : children_to_destroy)
  {
    child->destroy();
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

void GameObject::tick_self_and_children(const TickType tick_type, EventContext& event_context)
{
  if (!is_enabled())
    return;

  for (auto& [type, behaviour] : behaviours)
  {
    if (!behaviour || !behaviour->is_started())
      continue;

    switch (tick_type) 
    {
      case TickType::fixed_update:
        behaviour->fixed_update_tick(event_context);
        break;
      case TickType::update:
        behaviour->update_tick(event_context);
        break;
      case TickType::late_update: 
        behaviour->late_tick(event_context);
        break;
      case TickType::render:
        behaviour->render_tick(event_context);
        break;
      case TickType::imgui_render:
        behaviour->imgui_render_tick(event_context);
        break;
      case TickType::final:
        behaviour->final_tick(event_context);
        break;
      case TickType::preloop:
        behaviour->preloop_tick(event_context);
        break;
      default:
        return;
    }
  }

  // Use index-based iteration to handle children vector reallocation during ticking
  for (size_t i = 0; i < children.size(); ++i)
  {
    children[i]->tick_self_and_children(tick_type, event_context);
  }
}


void GameObject::set_parent(GameObject* _parent, bool keep_world_transform)
{
  if (parent == _parent)
    return;

  if (!parent)
  {
    // Only hierarchy roots have no parent, and nothing owns them through a children vector.
    Log::get_engine_logger()->error("Cannot reparent '{}': it has no parent (hierarchy roots cannot be moved).", get_name());
    return;
  }

  if (!_parent)
  {
    Log::get_engine_logger()->error("Cannot reparent '{}' to nullptr. Use destroy() to remove it.", get_name());
    return;
  }

  if (_parent->is_descendant_of(this))
  {
    Log::get_engine_logger()->error("Cannot parent '{}' to itself or one of its descendants.", get_name());
    return;
  }

  // Capture world matrix before the parent changes.
  const glm::mat4 world_matrix = get_world_matrix();

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

  std::unique_ptr<GameObject> owned_self = std::move(*it);
  siblings.erase(it);

  parent = _parent;
  parent->children.emplace_back(std::move(owned_self));

  if (keep_world_transform)
  {
    transform = Transform::from_matrix(glm::inverse(parent->get_world_matrix()) * world_matrix);
  }
}

bool GameObject::is_descendant_of(const GameObject* other) const
{
  for (const GameObject* current = this; current; current = current->parent)
  {
    if (current == other)
      return true;
  }
  return false;
}

glm::mat4 GameObject::get_world_matrix() const
{
  if (parent)
    return parent->get_world_matrix() * transform.to_matrix();

  return transform.to_matrix();
}

Transform GameObject::get_world_transform() const
{
  if (!parent)
    return transform;

  return Transform::from_matrix(get_world_matrix());
}

glm::vec3 GameObject::get_world_position() const
{
  return glm::vec3(get_world_matrix()[3]);
}

glm::quat GameObject::get_world_rotation() const
{
  if (!parent)
    return transform.get_rotation_quat();

  return get_world_transform().get_rotation_quat();
}

glm::vec3 GameObject::get_world_scale() const
{
  return get_world_transform().scale;
}

void GameObject::set_world_position(const glm::vec3& position)
{
  if (!parent)
  {
    transform.position = position;
    return;
  }

  transform.position = glm::vec3(glm::inverse(parent->get_world_matrix()) * glm::vec4(position, 1.0f));
}

void GameObject::set_world_rotation(const glm::quat& rotation)
{
  if (!parent)
  {
    transform.set_rotation_quat(rotation);
    return;
  }

  const glm::quat parent_rotation = parent->get_world_rotation();
  transform.set_rotation_quat(glm::inverse(parent_rotation) * rotation);
}

void GameObject::set_world_transform(const Transform& world_transform)
{
  if (!parent)
  {
    transform = world_transform;
    return;
  }

  transform = Transform::from_matrix(glm::inverse(parent->get_world_matrix()) * world_transform.to_matrix());
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

void GameObject::start_behaviours_recursive()
{
  // Start behaviours in this GameObject depth-first
  for (auto& [type, behaviour] : behaviours)
  {
    if (behaviour && !behaviour->is_started())
    {
      behaviour->set_started(true);
      behaviour->start_tick();
    }
  }

  // Recursively start behaviours in children
  for (auto& child : children)
  {
    child->start_behaviours_recursive();
  }
}

Behaviour* GameObject::add_behaviour_by_name(const std::string& type_name)
{
  BehaviourFactory factory = BehaviourRegistry::get_factory(type_name);
  if (!factory)
  {
    Log::get_engine_logger()->error("Unknown behaviour type: {}", type_name);
    return nullptr;
  }
  
  // Call factory with nullptr for serialization data (not deserializing)
  Behaviour* behaviour = factory(this, nullptr);
  if (!behaviour)
  {
    Log::get_engine_logger()->error("Failed to create behaviour by name: {}", type_name);
    return nullptr;
  }

  // Add to behaviours map with the actual type
  auto [it, inserted] = behaviours.emplace(
    std::type_index(typeid(*behaviour)),
    nullptr
  );
  
  if (!inserted)
  {
    Log::get_engine_logger()->warn("Behaviour of type already exists on GameObject: {}", get_name());
    delete behaviour;
    return nullptr;
  }

  it->second.reset(behaviour);
  return behaviour;
}

} // namespace Calgine