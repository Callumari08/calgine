#pragma once

namespace Calgine {

class Time
{
public:
  static Time& get_instance();

  static inline double time() { return Time::get_instance().get_time(); }
  static inline float delta_time() { return Time::get_instance().get_delta_time(); }
  static inline float fixed_delta_time() { return Time::get_instance().get_fixed_delta_time(); }

  float get_time() const { return t; }
  float get_delta_time() const { return dt; }
  float get_fixed_delta_time() const { return fixed_dt; }
  
  int consume_fixed_timesteps();

private:
  void init();
  void update();

  double t = 0.0f;
  float dt = 1.0f;
  float fixed_dt = 1.0f / 60.0f;
  float accumulated_time = 0.0f;

  float previous_time = 0.0f;

  uint64_t frequency = 0;
  uint64_t previous_counter = 0;

  friend class App;
};

}