#include "camera_behaviour.h"
#include "calgine/core/renderer/camera.h"
#include "calgine/core/renderer/camera_manager.h"

namespace Calgine {

CameraBehaviour::CameraBehaviour() : camera(65.0f, 16.0f / 9.0f, 0.1f, 1000.0f) {}

void CameraBehaviour::start_tick()
{
  CameraManager::get_instance().add_camera(this);
}

void CameraBehaviour::late_tick() 
{
  camera.update(get_transform());
}

void CameraBehaviour::apply_settings(const CameraSettings applied_settings)
{
  settings = applied_settings;

  camera.update_projection(applied_settings.fov, applied_settings.aspect_ratio, 
    applied_settings.near_plane, applied_settings.far_plane);
}

}