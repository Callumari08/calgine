#pragma once

#include <type_traits>
#include <concepts>

class GameObject;

/**
 * @brief Gives access to tick updates for users.
 *
 * @code {.cpp}
 * //TODO: Example 
 * @endcode 
 */
class Behaviour  
{
public:
  GameObject* get_game_object();

  void attach_owner(GameObject* _game_object);
private:
  GameObject* game_object;

  virtual void start_tick();
  virtual void update_tick();
  virtual void late_tick();

  virtual void on_attach_owner();
  virtual void on_destroy();

  template<typename T>
  requires std::derived_from<T, Behaviour>
  static T* create() 
  {
    static_assert(std::is_default_constructible_v<T>, "Behaviour must be default-constructable.");

    return new T();
  }

  friend class GameObject;

protected:
  Behaviour() = default;
public:
  virtual ~Behaviour();
};