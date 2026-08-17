#pragma once

#include "calgine/core/behaviour.h"
#include "calgine/core/event_context.h"
#include "calgine_api.h"

namespace Calgine {

class CALGINE_API FpsDisplay : public Behaviour
{
public:
  void imgui_render_tick(EventContext&) override;
  
  void set_update_delay(float delay) { update_delay = delay; }
  float get_update_delay() const { return update_delay; }

private:
  bool window_active = true;
  float update_delay = 0.2f;
  float time_since_last_update = 0.0f;
  float current_fps = 0.0f;
  float current_frame_time = 0.0f;
};

CALGINE_REGISTER_BEHAVIOUR(FpsDisplay, "fps_display");

}
