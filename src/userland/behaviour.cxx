#include "behaviour.h"
#include "../background_managers/behaviour_manager.h"

Behaviour::Behaviour()
{
  BehaviourManager::get_instance().register_behaviour(this);
}

Behaviour::~Behaviour() 
{
  BehaviourManager::get_instance().deregister_behaviour(this);
}

void Behaviour::start() {}
void Behaviour::tick() {}
void Behaviour::late_tick() {}