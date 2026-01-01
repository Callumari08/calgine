#include "behaviour.h"
#include "background_managers/behaviour_manager.h"

Behaviour::~Behaviour() 
{
  BehaviourManager::get_instance().deregister_behaviour(this);
}

void Behaviour::register_instance(Behaviour* behaviour) {
    BehaviourManager::get_instance().register_behaviour(behaviour);
}

void Behaviour::start() {}
void Behaviour::tick() {}
void Behaviour::late_tick() {}