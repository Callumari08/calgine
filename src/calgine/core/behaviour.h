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
  static void register_instance(Behaviour* b);

  virtual void start();
  virtual void tick();
  virtual void late_tick();

  friend class BehaviourManager;
  friend class GameObject;

protected:
  Behaviour() = default;

public:
  virtual ~Behaviour();
  
  template<typename T, typename... Args>
  requires std::derived_from<T, Behaviour>
    static T* create(Args&&... args) {

      T* obj = new T(std::forward<Args>(args)...);
      register_instance(obj);
      return obj;
    }
};