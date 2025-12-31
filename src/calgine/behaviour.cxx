#include "behaviour.h"
#include "background_managers/behaviour_manager.h"
#include <iostream>

Behaviour::~Behaviour() 
{
  BehaviourManager::get_instance().deregister_behaviour(this);
}

void Behaviour::register_instance(Behaviour* b) {
    BehaviourManager::get_instance().register_behaviour(b);
}

void Behaviour::start() {}
void Behaviour::tick() {}
void Behaviour::late_tick() {}