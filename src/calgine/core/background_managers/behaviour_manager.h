#pragma once

#include <vector>
#include "../behaviour.h"

// Manages all userland code, and acts as a shell for the main thread to call each tick function.
class BehaviourManager 
{
private:
  std::vector<Behaviour*> behaviours;

  BehaviourManager() = default;

public:
  static BehaviourManager& get_instance();
  void register_behaviour(Behaviour* behaviour);
  void deregister_behaviour(Behaviour* behaviour);

  void start_all();
  void update_tick();
  void update_late_tick();

  BehaviourManager(const BehaviourManager&) = delete;
  BehaviourManager& operator=(const BehaviourManager&) = delete;
};