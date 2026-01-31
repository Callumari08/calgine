#include "camera_behaviour.h"
#include "calgine/core/renderer/camera.h"
#include "calgine/core/renderer/camera_manager.h"

namespace Calgine {

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

  camera.set_fov(settings.fov, false);
  camera.set_aspect_ratio(settings.aspect_ratio, false);
  camera.set_near_plane(settings.near_plane, false);
  camera.set_far_plane(settings.far_plane, true);
}

}