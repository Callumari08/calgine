#pragma once

#include <unordered_map>
#include <typeindex>
#include <memory>

class Behaviour;

class GameObject final
{
private:
  std::unordered_map<std::type_index, std::unique_ptr<Behaviour>> behaviours;
public:
  GameObject() = default;
  ~GameObject() = default;

  GameObject(const GameObject&) = delete;
  GameObject& operator=(const GameObject&) = delete;

  template<typename T_behaviour, typename... Args>
  requires std::derived_from<T_behaviour, Behaviour>
  bool add_behaviour(Args&&... args);

  template<typename T_behaviour>
  requires std::derived_from<T_behaviour, Behaviour>
  T_behaviour* get_behaviour() const;

  template<typename T_behaviour>
  requires std::derived_from<T_behaviour, Behaviour>
  bool has_behaviour() const;
};
