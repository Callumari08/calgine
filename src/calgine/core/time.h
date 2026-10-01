#pragma once

#include <cstdint>
#include "calgine_api.h"

namespace Calgine {

class CALGINE_API Time
{
public:
  static Time& get_instance();

  static inline double time() { return Time::get_instance().get_time(); }
  static inline float delta_time() { return Time::get_instance().get_delta_time(); }
  static inline float fixed_delta_time() { return Time::get_instance().get_fixed_delta_time(); }
  /**
   * @brief How far the current frame is between the last fixed step and the next one, from 0 to 1.
   * Used to blend (interpolate) physics poses for rendering.
   */
  static inline float fixed_alpha() { return Time::get_instance().get_fixed_alpha(); }

  float get_time() const { return t; }
  float get_delta_time() const { return dt; }
  float get_fixed_delta_time() const { return fixed_dt; }
  float get_fixed_alpha() const { return fixed_dt > 0.0f ? accumulated_time / fixed_dt : 0.0f; }

  /** @brief Total fixed steps skipped because a frame needed more than the per-frame cap. */
  uint64_t get_dropped_fixed_steps() const { return dropped_fixed_steps; }
  /** @brief Fixed steps skipped in the most recent frame. Non-zero means the simulation is running slower than real time. */
  int get_last_dropped_fixed_steps() const { return last_dropped_fixed_steps; }

  /**
   * @brief Returns how many fixed steps to run this frame and removes that time from the accumulator.
   *
   * @param max_steps Upper limit per frame (0 = unlimited). Steps over the limit are dropped rather than
   *        carried into the next frame, so a slow frame can't snowball into ever slower frames.
   */
  int consume_fixed_timesteps(int max_steps = 0);

private:
  void init();
  void update();

  double t = 0.0f;
  float dt = 1.0f;
  float fixed_dt = 1.0f / 60.0f;
  float accumulated_time = 0.0f;
  uint64_t dropped_fixed_steps = 0;
  int last_dropped_fixed_steps = 0;

  float previous_time = 0.0f;

  uint64_t frequency = 0;
  uint64_t previous_counter = 0;

  friend class App;
};

}