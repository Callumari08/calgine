#include "fps_display.h"
#include "calgine/core/time.h"
#include <imgui.h>

namespace Calgine {

void FpsDisplay::imgui_render_tick(EventContext&)
{
  if (!window_active)
    return;

  time_since_last_update += Time::delta_time();
  
  if (time_since_last_update >= update_delay)
  {
    float delta_time = Time::delta_time();
    current_fps = delta_time > 0.0f ? 1.0f / delta_time : 0.0f;
    current_frame_time = delta_time * 1000.0f;
    time_since_last_update = 0.0f;
  }

  ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(200.0f, 100.0f), ImGuiCond_FirstUseEver);
  
  ImGui::Begin("FPS Display", &window_active);
  
  ImGui::Text("FPS: %.1f", current_fps);
  ImGui::Text("Frame Time: %.2f ms", current_frame_time);
  ImGui::SliderFloat("Update Delay", &update_delay, 0.01f, 1.0f, "%.2f s");
  
  ImGui::End();
}

}
