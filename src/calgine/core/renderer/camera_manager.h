#pragma once

#include "calgine/core/renderer/camera_behaviour.h"

namespace Calgine {

class CameraBehaviour;

class CameraManager
{
public:
  static CameraManager& get_instance();

  CameraManager(const CameraManager&) = delete;
  CameraManager& operator=(const CameraManager&) = delete;

  void add_camera(CameraBehaviour* camera);
  void set_active_camera(CameraBehaviour* camera);

private:
  CameraManager();

  std::vector<CameraBehaviour*> cameras;
  CameraBehaviour* active_camera;
};

}