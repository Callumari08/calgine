#pragma once

#include "calgine_pch.h"
#include "calgine_api.h"

namespace Calgine {

class GameObject;

/**
 * @brief Base class for all behaviour components that can be attached to GameObjects.
 *
 * @details
 * Behaviour provides the component-based functionality for GameObjects. By inheriting from
 * Behaviour and overriding its virtual tick methods, you can create custom game logic that
 * responds to different phases of the game loop.
 * 
 * ## Lifecycle
 * 
 * When a Behaviour is added to a GameObject:
 * 1. The Behaviour is constructed
 * 2. attach_owner() is called internally to link it to the GameObject
 * 3. start_tick() is called once
 * 4. update_tick() and late_tick() are called every frame while enabled
 * 5. on_destroy() is called when the GameObject is destroyed
 * 
 * ## Tick Methods
 * 
 * Override these protected virtual methods to implement your game logic:
 * - **preloop_tick()**: Called once before the main loop starts (only for behaviours added before the loop)
 * - **start_tick()**: Called once when the behaviour is first attached
 * - **update_tick()**: Called every frame during the update phase
 * - **late_tick()**: Called every frame after update, useful for cameras and follow logic
 * - **on_destroy()**: Called when the GameObject is being destroyed
 * 
 * ## Usage Example
 * 
 * @code{.cpp}
 * class RotatingBehaviour : public Calgine::Behaviour 
 * {
 * protected:
 *     void update_tick() override
 *     {
 *         // Access the owning GameObject
 *         GameObject* obj = get_game_object();
 *         
 *         // Log using the convenience macro
 *         print("Updating rotation for {}", obj->get_name());
 *         
 *         // Your update logic here
 *         rotation_angle += delta_time;
 *     }
 *     
 *     void on_destroy() override
 *     {
 *         print("Cleaning up RotatingBehaviour");
 *     }
 *     
 * private:
 *     float rotation_angle = 0.0f;
 * };
 * 
 * // Usage
 * GameObject obj(nullptr);
 * obj.add_behaviour<RotatingBehaviour>();
 * @endcode
 * 
 * ## Logging Macros
 * 
 * Three convenience macros are provided for logging within behaviours:
 * - **print(...)**: Info level logging
 * - **print_warning(...)**: Warning level logging  
 * - **print_error(...)**: Error level logging
 * 
 * @note Behaviour subclasses must be default-constructible.
 * @see GameObject, TickType
 */
class CALGINE_API Behaviour  
{
/// Convenience macro for info-level logging in behaviours
#define print(...) Log::get_app_logger()->info(__VA_ARGS__)
/// Convenience macro for warning-level logging in behaviours
#define print_warning(...) Log::get_app_logger()->warn(__VA_ARGS__)
/// Convenience macro for error-level logging in behaviours
#define print_error(...) Log::get_app_logger()->error(__VA_ARGS__)

public:
  /**
   * @brief Virtual destructor for proper cleanup of derived classes.
   */
  virtual ~Behaviour();
  
  /**
   * @brief Returns a pointer to the GameObject that this behaviour is attached to.
   * 
   * @return GameObject* Pointer to the owning GameObject
   * 
   * @details
   * Use this to access the GameObject's properties and methods from within the behaviour.
   * 
   * @code{.cpp}
   * void MyBehaviour::update_tick() {
   *     GameObject* obj = get_game_object();
   *     print("GameObject name: {}", obj->get_name());
   * }
   * @endcode
   * 
   * @warning Asserts if called before the behaviour is attached to a GameObject.
   */
  GameObject* get_game_object();

private:
  /// Pointer to the GameObject this behaviour is attached to
  GameObject* game_object;

  /**
   * @brief Factory method to create behaviour instances.
   * 
   * @tparam T The Behaviour type to create
   * @return T* Pointer to the newly created behaviour
   * 
   * @details
   * This template method ensures that only default-constructible behaviours can be created.
   * Called internally by GameObject::add_behaviour().
   */
  template<typename T>
  requires std::derived_from<T, Behaviour>
  static T* create() 
  {
    static_assert(std::is_default_constructible_v<T>, "Behaviour must be default-constructable.");

    return new T();
  }

  /**
   * @brief Attaches this behaviour to a GameObject.
   * 
   * @param _game_object Pointer to the GameObject to attach to
   * 
   * @details
   * Called internally when a behaviour is added to a GameObject.
   * Sets the game_object pointer and calls start_tick().
   * 
   * @warning Asserts if the behaviour is already attached to a GameObject.
   */
  void attach_owner(GameObject* _game_object);

  friend class GameObject;

protected:
  /**
   * @brief Protected default constructor.
   * 
   * @details
   * Behaviours can only be created through GameObject::add_behaviour().
   * Derived classes should use the default constructor or provide their own.
   */
  Behaviour() = default;

  /**
   * @brief Called once before the main loop starts.
   * 
   * @details
   * Override this method to perform initialization that should happen before the game loop begins.
   * Only called for behaviours that exist before the main loop starts.
   * Behaviours added during runtime will not have this method called.
   * 
   * @note This is called before start_tick().
   */
  virtual void preloop_tick();
  
  /**
   * @brief Called once when this behaviour is attached to a GameObject.
   * 
   * @details
   * Override this method to perform initialization specific to this behaviour instance.
   * This is called immediately after the behaviour is created and attached to a GameObject.
   * 
   * @code{.cpp}
   * void MyBehaviour::start_tick() {
   *     print("MyBehaviour started on {}", get_game_object()->get_name());
   *     // Initialize your behaviour here
   * }
   * @endcode
   */
  virtual void start_tick();
  
  /**
   * @brief Called every frame during the update phase.
   * 
   * @details
   * Override this method to implement the main logic of your behaviour.
   * This is called once per frame for all enabled GameObjects during the update tick.
   * 
   * @code{.cpp}
   * void MyBehaviour::update_tick() {
   *     // Main game logic here
   *     process_input();
   *     update_physics();
   * }
   * @endcode
   * 
   * @see late_tick()
   */
  virtual void update_tick();
  
  /**
   * @brief Called every frame after the update phase.
   * 
   * @details
   * Override this method to implement logic that should run after all update_tick() calls.
   * This is useful for cameras that need to follow objects, or any logic that depends
   * on the results of update_tick().
   * 
   * @code{.cpp}
   * void CameraBehaviour::late_tick() {
   *     // Update camera position based on target's final position
   *     follow_target();
   * }
   * @endcode
   * 
   * @see update_tick()
   */
  virtual void late_tick();

  /**
   * @brief Called when the GameObject this behaviour is attached to is being destroyed.
   * 
   * @details
   * Override this method to perform cleanup when the GameObject is destroyed.
   * At this point, the GameObject and all its properties are still valid.
   * 
   * @code{.cpp}
   * void MyBehaviour::on_destroy() {
   *     print("Cleaning up resources");
   *     // Release resources, save state, etc.
   * }
   * @endcode
   * 
   * @note You can safely access get_game_object() in this method.
   */
  virtual void on_destroy();
};

} // namespace Calgine