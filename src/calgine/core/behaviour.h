#pragma once

#include "calgine/core/transform.h"
#include "calgine/core/log.h"
#include "calgine_pch.h"
#include "calgine_api.h"

namespace Calgine {

class GameObject;

/**
 * @brief Gives access to tick updates for users.
 *
 * @code {.cpp}
 * //TODO: Example 
 * @endcode 
 */
class CALGINE_API Behaviour  
{
#define print(...) Log::get_app_logger()->info(__VA_ARGS__)
#define print_warning(...) Log::get_app_logger()->warn(__VA_ARGS__)
#define print_error(...) Log::get_app_logger()->error(__VA_ARGS__)

public:
  virtual ~Behaviour();
  /**
    * @brief Returns a pointer to the @link GameObject @endlink that this behaviour is attached to.  
    * 
    * @return @link GameObject @endlink* 
    */
  GameObject* get_game_object();
  Transform& get_transform();

private:
  GameObject* game_object;

  template<typename T>
  requires std::derived_from<T, Behaviour>
  static T* create() 
  {
    static_assert(std::is_default_constructible_v<T>, "Behaviour must be default-constructable.");

    return new T();
  }

  void attach_owner(GameObject* _game_object);

  friend class GameObject;

protected:
  Behaviour() = default;

    /**
    * @brief Gets called before the main loop (see @link App @endlink).
    * Behaviours that are instantiated after the mainloop has started will not have this function called.
    */
  virtual void preloop_tick();
  /**
    * @brief Gets called when this gameobject is instantitated.
    */
  virtual void start_tick();
  /**
    * @brief Gets called every frame, before late_tick().
    */
  virtual void update_tick();
  /**
    * @brief Gets called every frame, after update().
    */
  virtual void late_tick();

  virtual void imgui_render_tick();

  /**
    * @brief Gets called when game_object is being destroyed.
    * @note You can safely assume that game_object has not changed by this point.
    */
  virtual void on_destroy();
};

} // namespace Calgine