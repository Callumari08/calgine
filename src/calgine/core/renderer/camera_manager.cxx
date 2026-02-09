#include "camera_manager.h"
#include "calgine/core/renderer/camera_behaviour.h"

namespace Calgine {

CameraManager::CameraManager() : active_camera(nullptr)
{

}

CameraManager& CameraManager::get_instance()
{
  static CameraManager instance;
  return instance;
}

void CameraManager::set_active_camera(CameraBehaviour* unactive_camera)
{
  for (CameraBehaviour* camera : cameras)
  {
    if (camera == unactive_camera)
    {
      if (active_camera)
        active_camera->set_camera_active(false);
      active_camera = camera;
      camera->set_camera_active(true);
      return;
    }
  }

  Log::get_engine_logger()->warn("Attempted to activate a camera that was not registered with the Camera Manager!");
}

void CameraManager::add_camera(CameraBehaviour* unadded_camera)
{
  if (cameras.size() == 0)
  {
    Log::get_engine_logger()->info("No active cameras, setting added camera to active camera");
    cameras.emplace_back(unadded_camera);
    set_active_camera(unadded_camera);
    return;
  }

  for (CameraBehaviour* camera : cameras)
  {
    if (camera == unadded_camera)
    {
      Log::get_engine_logger()->warn("Attempted to add camera that was already registered with Camera Manager!");
      return;
    }
  }

  cameras.emplace_back(unadded_camera);
}

}