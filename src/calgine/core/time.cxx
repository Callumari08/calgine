#include "time.h"
#include "SDL3/SDL_timer.h"

namespace Calgine {

Time& Time::get_instance()
{
  static Time instance;
  return instance;
}

void Time::init()
{
  frequency = SDL_GetPerformanceFrequency();
  previous_counter = SDL_GetPerformanceCounter();
}

void Time::update()
{
  uint64_t current_counter = SDL_GetPerformanceCounter();
  uint64_t elapsed = current_counter - previous_counter;
  previous_counter = current_counter;

  float delta = static_cast<float>(elapsed) / static_cast<float>(frequency);
  delta = std::min(delta, 0.25f);

  dt = delta;
  t += delta;
  accumulated_time += delta;
}

int Time::consume_fixed_timesteps(int max_steps)
{
  int timesteps = static_cast<int>(accumulated_time / fixed_dt);
  if (timesteps > 0)
    accumulated_time -= timesteps * fixed_dt;

  last_dropped_fixed_steps = 0;
  if (max_steps > 0 && timesteps > max_steps)
  {
    // Drop the excess instead of queueing it: the simulation falls behind real time,
    // but frames stay short enough for the game to remain responsive.
    last_dropped_fixed_steps = timesteps - max_steps;
    dropped_fixed_steps += static_cast<uint64_t>(last_dropped_fixed_steps);
    timesteps = max_steps;
  }

  return timesteps;
}

}