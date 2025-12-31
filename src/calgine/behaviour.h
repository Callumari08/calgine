#ifndef BEHAVIOUR_H
#define BEHAVIOUR_H

#include <type_traits>
#include <utility>
/**
 * @brief Gives access to tick updates for users.
 *
 * @code {.cpp}
 * 
 * @endcode 
 */
class Behaviour  
{
private:
  virtual void start();
  virtual void tick();
  virtual void late_tick();

  friend class BehaviourManager;
  
  static void register_instance(Behaviour* b);

protected:
  Behaviour() = default;

public:
  virtual ~Behaviour();
  
  template<typename T, typename... Args>
    static T* create(Args&&... args) {
      static_assert(std::is_base_of_v<Behaviour, T>, "T must derive from `Behaviour`");

      T* obj = new T(std::forward<Args>(args)...);
      register_instance(obj);
      return obj;
    }
};

#endif // BEHAVIOUR_H