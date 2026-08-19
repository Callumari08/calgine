#include "camera_behaviour.h"
#include "camera.h"
#include "camera_manager.h"
#include "calgine/core/transform.h"
#include "imgui.h"
#include "calgine/core/game_object.h"
#include "calgine/core/behaviour_serialization/behaviour_register_macro.h"

namespace Calgine {

CameraBehaviour::CameraBehaviour(bool show_debug) : camera(65.0f, 16.0f / 9.0f, 0.1f, 1000.0f), debug(show_debug) {}

void CameraBehaviour::start_tick()
{
  CameraManager::get_instance().add_camera(this);
}

void CameraBehaviour::late_tick(EventContext&) 
{
  camera.update(get_transform());
}

void CameraBehaviour::apply_settings(const CameraSettings applied_settings)
{
  settings = applied_settings;

  camera.update_projection(applied_settings.fov, applied_settings.aspect_ratio, 
    applied_settings.near_plane, applied_settings.far_plane);
}

void CameraBehaviour::imgui_render_tick(EventContext&)
{
  if (!debug)
    return;
  
  ImGui::Begin("Camera Settings");

  ImGui::DragFloat3("Position", &get_transform().position.x, 0.1f);

  float fov_change = settings.fov;
  ImGui::SliderFloat("FOV", &fov_change, 0.0f, 120.0f);
  if (fov_change != settings.fov)
  {
    settings.fov = fov_change;
    camera.update_projection(settings.fov, settings.aspect_ratio, settings.near_plane, settings.far_plane);
  }

  ImGui::End();
}

CALGINE_REGISTER_BEHAVIOUR(CameraBehaviour, "camera");

}