#include "game_object.h"
#include "behaviour.h"
#include "useful_funcs.h"

#include <iostream>

template<typename T_behaviour, typename... Args>
requires std::derived_from<T_behaviour, Behaviour>
bool GameObject::add_behaviour(Args&&... args)
{
    auto [it, inserted] = behaviours.emplace(
      typeid(T_behaviour),
      nullptr
    );

    if (!inserted) 
    {
      std::cout << "Type: `" << type_name<T_behaviour>() << "` already exists on this GameObject\n";
      return false;
    }

    std::unique_ptr<Behaviour> owned;

    try 
    {
      owned.reset(Behaviour::create<T_behaviour>(std::forward<Args>(args)...));
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