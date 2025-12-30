#ifndef BEHAVIOUR_H
#define BEHAVIOUR_H

class Behaviour  
{
private:
  virtual void start();
  virtual void tick();
  virtual void late_tick();

  friend class BehaviourManager;

public:
  Behaviour();
  virtual ~Behaviour();
};

#endif // BEHAVIOUR_H