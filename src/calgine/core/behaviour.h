#pragma once

#include <utility>
/**
 * @brief Gives access to tick updates for users.
 *
 * @code {.cpp}
 * //TODO: Example 
 * @endcode 
 */
class Behaviour  
{
private:
  virtual void start_tick();
  virtual void update_tick();
  virtual void late_tick();

  template<typename T, typename... Args>
  requires std::derived_from<T, Behaviour>
  static T* create(Args&&... args) 
  {
    T* obj = new T(std::forward<Args>(args)...);
    return obj;
  }

  friend class BehaviourManager;
  friend class GameObject;

protected:
  Behaviour() = default;

public:
  virtual ~Behaviour();
};