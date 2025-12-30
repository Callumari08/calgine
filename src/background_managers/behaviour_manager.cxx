#include "behaviour_manager.h"
#include <algorithm>
#include <vector>

BehaviourManager& BehaviourManager::get_instance()
{
  static BehaviourManager instance;
  return instance;
}

void BehaviourManager::register_behaviour(Behaviour* behaviour)
{
  behaviours.push_back(behaviour);
}

void BehaviourManager::deregister_behaviour(Behaviour* behaviour)
{
  behaviours.erase(std::remove(behaviours.begin(), behaviours.end(), behaviour));
}

void BehaviourManager::start_all()
{
  for (Behaviour* behaviour : behaviours)
  {
    behaviour->start();
  }
}

void BehaviourManager::update_tick()
{
  for (Behaviour* behaviour : behaviours)
  {
    behaviour->tick();
  }
}

void BehaviourManager::update_late_tick()
{
  for (Behaviour* behaviour : behaviours)
  {
    behaviour->late_tick();
  }
}