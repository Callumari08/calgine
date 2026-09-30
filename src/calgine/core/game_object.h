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
  
  /**
   * @brief The local transform, relative to the parent GameObject.
   */
  inline Transform& get_transform() { return transform; }
  inline const Transform& get_transform() const { return transform; }

  // ---- World space ----
  // World values are computed on demand from the parent chain.

  /** @brief parent world matrix * local matrix. */
  glm::mat4 get_world_matrix() const;
  /** @brief World transform decomposed from get_world_matrix(). */
  Transform get_world_transform() const;
  glm::vec3 get_world_position() const;
  glm::quat get_world_rotation() const;
  glm::vec3 get_world_scale() const;

  /** @brief Moves this object so its world position is @p position (children follow). */
  void set_world_position(const glm::vec3& position);
  /** @brief Rotates this object so its world rotation is @p rotation (children follow). */
  void set_world_rotation(const glm::quat& rotation);
  /** @brief Sets position, rotation and scale in world space. */
  void set_world_transform(const Transform& world_transform);

  template<typename T_behaviour, typename... Args>
  requires std::derived_from<T_behaviour, Behaviour>
  T_behaviour* add_behaviour(Args&&... args);

  template<typename T_behaviour>
  requires std::derived_from<T_behaviour, Behaviour>
  T_behaviour* get_behaviour() const;

  template<typename T_behaviour>
  requires std::derived_from<T_behaviour, Behaviour>
  bool has_behaviour() const;

  // Deferred startup: add behaviour but don't call start_tick() until start_behaviours_recursive() is called
  template<typename T_behaviour, typename... Args>
  requires std::derived_from<T_behaviour, Behaviour>
  T_behaviour* add_behaviour_deferred(Args&&... args);

  // Start all behaviours in this GameObject and children (called after all objects created)
  void start_behaviours_recursive();

  // Add a behaviour by its registered type name (for runtime instantiation)
  Behaviour* add_behaviour_by_name(const std::string& type_name);

  template<typename T = GameObject, typename... Args>
  requires std::derived_from<T, GameObject>
  T& instantiate_child(Args&&... args)
  {
    auto child = std::make_unique<T>(this, std::forward<Args>(args)...);
    children.emplace_back(std::move(child));
    return *static_cast<T*>(children.back().get());
  }


  /**
   * @brief Moves this GameObject under a new parent.
   *
   * @param parent The new parent. Must not be this object or one of its descendants.
   * @param keep_world_transform If true (default), the local transform is recalculated so the
   *        object stays where it is in the world. If false, the local transform is kept as-is.
   */
  void set_parent(GameObject* parent, bool keep_world_transform = true);

  /** @brief True if this object is @p other or is somewhere below it in the hierarchy. */
  bool is_descendant_of(const GameObject* other) const;

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
  
  if(!owned->attach_owner(this))
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

template<typename T_behaviour, typename... Args>
requires std::derived_from<T_behaviour, Behaviour>
T_behaviour* GameObject::add_behaviour_deferred(Args&&... args)
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
  
  if(!owned->attach_owner(this))
  {
    behaviours.erase(it);
    return nullptr;
  }
  
  it->second = std::move(owned);
  T_behaviour* behaviour = static_cast<T_behaviour*>(it->second.get());
  
  // Mark as not started - start_tick() will be called later by start_behaviours_recursive()
  behaviour->set_started(false);
  
  return behaviour;
}

} // namespace Calgine