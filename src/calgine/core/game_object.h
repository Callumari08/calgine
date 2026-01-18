#pragma once

#include "calgine/core/log.h"
#include "calgine_pch.h"
#include "calgine_api.h"

#include "behaviour.h"

namespace Calgine {

/**
 * @brief Enumeration of tick update phases in the game loop.
 * 
 * @details
 * These tick types determine when GameObject behaviours are updated during the game loop.
 * Each phase serves a specific purpose in the update cycle.
 */
enum TickType
{
  preloop,      ///< Executed once before the main loop starts
  update,       ///< Main update phase, executed every frame
  late_update,  ///< Executed after update, useful for camera and final calculations
};

/**
 * @brief GameObjects are objects within the Hierarchy that have multiple @link Behaviour @endlink 
 * subclasses attached to them which execute code.
 * 
 * @details
 * GameObject is the fundamental building block of Calgine applications. GameObjects form a
 * hierarchical tree structure where each GameObject can have multiple child GameObjects and
 * multiple attached Behaviour components that define its functionality.
 * 
 * ## Key Features
 * - **Hierarchical Structure**: GameObjects can have parent and children relationships
 * - **Component System**: Attach multiple Behaviour subclasses to add functionality
 * - **Lifecycle Management**: Automatic memory management with deferred deletion
 * - **Enable/Disable**: Control whether a GameObject and its behaviours are active
 * 
 * ## Usage Example
 * @code{.cpp}
 * // Create a root GameObject
 * GameObject root(nullptr);
 * 
 * // Add a custom behaviour
 * root.add_behaviour<MyCustomBehaviour>();
 * 
 * // Create a child GameObject
 * GameObject& child = root.instantiate_child();
 * child.set_name("ChildObject");
 * 
 * // Access a behaviour
 * auto* behaviour = root.get_behaviour<MyCustomBehaviour>();
 * @endcode
 * 
 * @warning Do not directly delete GameObjects. Use destroy() instead to ensure proper cleanup.
 * @see Behaviour, TickType
 */
class CALGINE_API GameObject
{
private:
  /// Debug counter tracking total number of GameObjects
  static uint32_t num_game_objects;

  /// Flag indicating if this GameObject has been destroyed
  bool destroyed = false;
  
  /// Flag indicating if this GameObject is currently active
  bool enabled = true;

  /// Debug name for this GameObject
  std::string name = "";
  
  /// Map of behaviour type to behaviour instance
  std::unordered_map<std::type_index, std::unique_ptr<Behaviour>> behaviours;
  
  /// Collection of child GameObjects
  std::vector<std::unique_ptr<GameObject>> children;
  
  /// Pointer to parent GameObject, or nullptr if root
  GameObject* parent = nullptr;

  /// Queue to hold objects whose memory must be released AFTER their member functions finish
  static inline std::vector<std::unique_ptr<GameObject>> s_pending_deletes;

public:
  /**
   * @brief Constructs a new GameObject with an optional parent.
   * 
   * @param _parent Pointer to the parent GameObject, or nullptr for root objects
   * 
   * @details
   * Creates a new GameObject and automatically assigns it a unique debug name.
   * If a parent is provided, this GameObject becomes part of that parent's hierarchy.
   */
  explicit GameObject(GameObject* _parent);
  
  /**
   * @brief Destroys the GameObject.
   * 
   * @details
   * The destructor is called automatically when the GameObject is deleted.
   * Prefer using destroy() for explicit cleanup.
   */
  virtual ~GameObject();

  /**
   * @brief frees all deleted game objects allocated memory.
   * @warning must be called after all update ticks, once per frame. do NOT call in gamecode, this is to be part of app.h only.
   */
  static void process_pending_deletes();

  /**
   * @brief Marks this GameObject and all its children for destruction.
   * 
   * @details
   * Calls on_destroy() on all attached behaviours, recursively destroys all children,
   * and detaches from parent. The actual memory deallocation happens later via
   * process_pending_deletes() to ensure safe cleanup during iteration.
   * 
   * @note This function is idempotent - calling it multiple times has no additional effect.
   * @see process_pending_deletes()
   */
  void destroy();

  /**
   * @brief Detaches a child GameObject from this parent.
   * 
   * @param child Pointer to the child GameObject to detach
   * @return std::unique_ptr<GameObject> Ownership of the detached child, or nullptr if not found
   * 
   * @details
   * Removes the specified child from the children collection and returns ownership.
   * The child's parent pointer is NOT modified by this function.
   */
  std::unique_ptr<GameObject> detach_child(GameObject* child);

  /**
   * @brief Immediately removes and deletes a child GameObject.
   * 
   * @param child Pointer to the child GameObject to remove
   * @return bool True if the child was found and removed, false otherwise
   * 
   * @warning Only use this if the caller is the external owner of the child's lifetime.
   * For safe removal during iteration, use destroy() instead.
   */
  bool remove_child_immediate(GameObject* child);

  GameObject(const GameObject&) = delete;
  GameObject& operator=(const GameObject&) = delete;

  GameObject(GameObject&&) = default;
  GameObject& operator=(GameObject&&) = default;

  /**
   * @brief Checks if this GameObject is currently enabled.
   * 
   * @return bool True if enabled, false otherwise
   * 
   * @details
   * When a GameObject is disabled, its behaviours will not receive tick updates.
   */
  bool is_enabled() const;
  
  /**
   * @brief Sets the active state of this GameObject.
   * 
   * @param _enabled True to enable, false to disable
   * 
   * @details
   * Disabling a GameObject prevents its behaviours from receiving tick updates
   * but does not affect its children or destroy it.
   */
  void set_active(const bool _enabled); 

  /**
   * @brief Adds a new behaviour component to this GameObject.
   * 
   * @tparam T_behaviour The type of Behaviour to add (must derive from Behaviour)
   * @return bool True if the behaviour was successfully added, false if it already exists
   * 
   * @details
   * Creates and attaches a new instance of the specified Behaviour type.
   * Each GameObject can only have one instance of each behaviour type.
   * After creation, the behaviour's start_tick() method is called.
   * 
   * @code{.cpp}
   * GameObject obj(nullptr);
   * if (obj.add_behaviour<MyBehaviour>()) {
   *     Log::get_app_logger()->info("Behaviour added successfully");
   * }
   * @endcode
   * 
   * @warning The behaviour type must be default-constructible.
   * @see get_behaviour(), has_behaviour()
   */
  template<typename T_behaviour>
  requires std::derived_from<T_behaviour, Behaviour>
  bool add_behaviour();

  /**
   * @brief Retrieves a behaviour component from this GameObject.
   * 
   * @tparam T_behaviour The type of Behaviour to retrieve (must derive from Behaviour)
   * @return T_behaviour* Pointer to the behaviour if found, nullptr otherwise
   * 
   * @details
   * Returns a pointer to the behaviour of the specified type if it exists on this GameObject.
   * 
   * @code{.cpp}
   * auto* behaviour = obj.get_behaviour<MyBehaviour>();
   * if (behaviour) {
   *     behaviour->do_something();
   * }
   * @endcode
   * 
   * @see add_behaviour(), has_behaviour()
   */
  template<typename T_behaviour>
  requires std::derived_from<T_behaviour, Behaviour>
  T_behaviour* get_behaviour() const;

  /**
   * @brief Checks if this GameObject has a specific behaviour component.
   * 
   * @tparam T_behaviour The type of Behaviour to check for (must derive from Behaviour)
   * @return bool True if the behaviour exists, false otherwise
   * 
   * @details
   * Use this to check for the presence of a behaviour before attempting to use it.
   * 
   * @code{.cpp}
   * if (obj.has_behaviour<MyBehaviour>()) {
   *     // Safe to get and use the behaviour
   *     obj.get_behaviour<MyBehaviour>()->do_something();
   * }
   * @endcode
   * 
   * @see add_behaviour(), get_behaviour()
   */
  template<typename T_behaviour>
  requires std::derived_from<T_behaviour, Behaviour>
  bool has_behaviour() const;

  /**
   * @brief Creates and instantiates a new child GameObject.
   * 
   * @tparam T The GameObject type to instantiate (must derive from GameObject)
   * @tparam Args Types of additional constructor arguments
   * @param args Additional arguments to forward to the child's constructor
   * @return T& Reference to the newly created child GameObject
   * 
   * @details
   * Creates a child GameObject of the specified type and adds it to this GameObject's
   * children collection. The child's parent is automatically set to this GameObject.
   * 
   * @code{.cpp}
   * GameObject root(nullptr);
   * GameObject& child = root.instantiate_child();
   * child.set_name("MyChild");
   * 
   * // Creating a custom GameObject type with additional constructor parameters
   * CustomGameObject& custom = root.instantiate_child<CustomGameObject>(arg1, arg2);
   * @endcode
   * 
   * @note The returned reference remains valid until the child is destroyed or detached.
   */
  template<typename T = GameObject, typename... Args>
  requires std::derived_from<T, GameObject>
  T& instantiate_child(Args&&... args)
  {
    auto child = std::make_unique<T>(this, std::forward<Args>(args)...);
    T& ref = *child;
    children.emplace_back(std::move(child));
    return ref;
  }

  /**
   * @brief Executes tick updates on this GameObject and all its children.
   * 
   * @param tick_type The type of tick update to execute (preloop, update, or late_update)
   * 
   * @details
   * Recursively calls the appropriate tick function on all enabled behaviours attached
   * to this GameObject and its children, based on the specified tick type.
   * 
   * @warning This is typically called by the App class. Do not call manually in game code
   * unless you have a specific reason to do so.
   * 
   * @see TickType
   */
  void tick_self_and_children(TickType tick_type);

  /**
   * @brief Sets or changes the parent of this GameObject.
   * 
   * @param _parent Pointer to the new parent GameObject
   * 
   * @details
   * Changes this GameObject's parent in the hierarchy. This affects the GameObject's
   * position in the scene tree and which parent receives tick updates first.
   * 
   * @warning Use with caution as it modifies the hierarchy structure.
   */
  void set_parent(GameObject* _parent);

  /**
   * @brief Retrieves the parent GameObject.
   * 
   * @return std::optional<std::reference_wrapper<GameObject>> Optional reference to the parent,
   *         or std::nullopt if this is a root GameObject
   * 
   * @details
   * Returns an optional reference to the parent GameObject. If this GameObject is a root
   * (has no parent), returns std::nullopt.
   * 
   * @code{.cpp}
   * if (auto parent = obj.get_parent()) {
   *     parent->get().set_name("ParentObject");
   * }
   * @endcode
   */
  std::optional<std::reference_wrapper<GameObject>> get_parent() const;

  /**
   * @brief Get the name of the GameObject.
   * 
   * @return std::string The GameObject's debug name
   * 
   * @warning ONLY use name for debugging purposes. Do not implement a GameObject* find(std::string name)
   * feature, as this would encourage poor design patterns and create performance issues.
   * 
   * @see set_name()
   */
  virtual std::string get_name()
  {
    return name;
  }

  /**
   * @brief Sets the debug name of this GameObject.
   * 
   * @param _name The new name for this GameObject
   * 
   * @details
   * Changes the GameObject's debug name. This is useful for debugging and logging purposes.
   * 
   * @see get_name()
   */
  void set_name(std::string _name);
};

// Templates

template<typename T_behaviour>
requires std::derived_from<T_behaviour, Behaviour>
bool GameObject::add_behaviour()
{
    auto [it, inserted] = behaviours.emplace(
      typeid(T_behaviour),
      nullptr
    );

    if (!inserted) 
    {
      Log::get_engine_logger()->warn("Behaviour already exists on GameObject: {}", get_name());
      return false;
    }

    std::unique_ptr<Behaviour> owned;

    try 
    {
      owned.reset(Behaviour::create<T_behaviour>());
      owned->attach_owner(this);
    }
    catch (...) {
    behaviours.erase(it);
    throw;
  }

  it->second = std::move(owned);
  return true;
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