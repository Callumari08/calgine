#pragma once

#include "calgine/core/event_context.h"
#include "calgine/core/tick_type.h"
#include "calgine/core/log.h"
#include "calgine/core/transform.h"
#include "calgine_pch.h"
#include "calgine_api.h"

#include "behaviour.h"
#include <cstdint>

namespace Calgine {

/**
 * @brief GameObjects are objects within the @link Heirarchy @endlink that have multiple @link Behaviour @endlink 
 * subclasses attached to them which execute code.
 */
class CALGINE_API GameObject
{
private:
  // use for debugging purpouses only
  static uint32_t num_game_objects;

  bool destroyed = false;
  
  bool enabled = true;

  Transform transform;
  std::string name = "";
  std::unordered_map<std::type_index, std::unique_ptr<Behaviour>> behaviours;
  std::vector<std::unique_ptr<GameObject>> children;
  GameObject* parent = nullptr;

  // queue to hold objects whose memory must be released AFTER their member functions finish
  static inline std::vector<std::unique_ptr<GameObject>> pending_deletes;

  void tick_self_and_children(const TickType tick_type, EventContext& event_context);
  
  template<typename T_behaviour, typename... Args>
  requires std::derived_from<T_behaviour, Behaviour>
  T_behaviour* add_behaviour_impl(bool start_immediately, Args&&... args);

  static void process_pending_deletes();

public:
  explicit GameObject(GameObject* parent, const Transform transform, const std::string name);
  virtual ~GameObject();

  static inline uint32_t get_num_game_objects() { return num_game_objects; }

  void destroy();

  std::unique_ptr<GameObject> detach_child(GameObject* child);

  // convenience: immediately erase and delete child (use only if caller is external owner)
  bool remove_child_immediate(GameObject* child);

  GameObject(const GameObject&) = delete;
  GameObject& operator=(const GameObject&) = delete;

  GameObject(GameObject&&) = default;
  GameObject& operator=(GameObject&&) = default;

  inline bool is_enabled() const { return enabled; }
  void set_active(const bool _enabled);
  
  inline Transform& get_transform() { return transform; }

  template<typename T_behaviour, typename... Args>
  requires std::derived_from<T_behaviour, Behaviour>
  T_behaviour* add_behaviour(Args&&... args);

  template<typename T_behaviour, typename... Args>
  requires std::derived_from<T_behaviour, Behaviour>
  T_behaviour* add_behaviour_deferred(Args&&... args);

  template<typename T_behaviour>
  requires std::derived_from<T_behaviour, Behaviour>
  T_behaviour* get_behaviour() const;

  template<typename T_behaviour>
  requires std::derived_from<T_behaviour, Behaviour>
  bool has_behaviour() const;

  template<typename T = GameObject, typename... Args>
  requires std::derived_from<T, GameObject>
  T& instantiate_child(Args&&... args)
  {
    auto child = std::make_unique<T>(this, std::forward<Args>(args)...);
    children.emplace_back(std::move(child));
    return *static_cast<T*>(children.back().get());
  }


  void set_parent(GameObject* parent);

  std::optional<std::reference_wrapper<GameObject>> get_parent() const;

  /**
   * @brief Get the name of the GameObject
   * @warning ONLY use name for debugging purpouses, do not try to implement a `Gameobject* find(std::string name)` feature. This will end badly.
   *
   * @return std::string 
   */
  virtual std::string get_name()
  {
    return name;
  }

  void start_behaviours_recursive();
  void set_name(std::string _name);

  friend class App;

  // Iterator for depth-first traversal
  class Iterator {
  private:
    std::stack<GameObject*> stack;

  public:
    Iterator(GameObject* root) {
      if (root) stack.push(root);
    }

    Iterator() {}

    GameObject& operator*() { return *stack.top(); }
    GameObject* operator->() { return stack.top(); }

    Iterator& operator++() {
      GameObject* current = stack.top();
      stack.pop();

      auto& children = current->children;
      for (auto it = children.rbegin(); it != children.rend(); ++it) {
        stack.push(it->get());
      }
      return *this;
    }

    bool operator!=(const Iterator& other) const {
      return stack.size() != other.stack.size();
    }
  };

  Iterator begin() { return Iterator(this); }
  Iterator end() { return Iterator(); }
};

// Templates

template<typename T_behaviour, typename... Args>
requires std::derived_from<T_behaviour, Behaviour>
T_behaviour* GameObject::add_behaviour(Args&&... args)
{
  return add_behaviour_impl<T_behaviour>(true, std::forward<Args>(args)...);
}

template<typename T_behaviour, typename... Args>
requires std::derived_from<T_behaviour, Behaviour>
T_behaviour* GameObject::add_behaviour_deferred(Args&&... args)
{
  return add_behaviour_impl<T_behaviour>(false, std::forward<Args>(args)...);
}

template<typename T_behaviour, typename... Args>
requires std::derived_from<T_behaviour, Behaviour>
T_behaviour* GameObject::add_behaviour_impl(const bool start_immediately, Args&&... args)
{
  auto [it, inserted] = behaviours.emplace(
    typeid(T_behaviour),
    nullptr
  );
  if (!inserted) 
  {
    Log::get_engine_logger()->warn("Behaviour already exists on GameObject: {}", get_name());
    return nullptr;
  }
  std::unique_ptr<Behaviour> owned;
   
  
  owned.reset(Behaviour::create<T_behaviour>(std::forward<Args>(args)...));
  
  if(!owned->attach_owner(this, start_immediately))
  {
    behaviours.erase(it);
    return nullptr;
  }
  
  it->second = std::move(owned);
  return static_cast<T_behaviour*>(it->second.get());
}

template<typename T_behaviour>
requires std::derived_from<T_behaviour, Behaviour>
T_behaviour* GameObject::get_behaviour() const
{
  auto it = behaviours.find(typeid(T_behaviour));
  if (it == behaviours.end())
    return nullptr;

  return static_cast<T_behaviour*>(it->second.get());
}

template<typename T_behaviour>
requires std::derived_from<T_behaviour, Behaviour>
bool GameObject::has_behaviour() const
{
  return behaviours.contains(typeid(T_behaviour));
}

} // namespace Calgine