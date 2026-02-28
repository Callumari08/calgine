#pragma once

#include "camera_behaviour.h"
#include "calgine_api.h"

namespace Calgine {

class CameraBehaviour;

class CALGINE_API CameraManager
{
public:
  static CameraManager& get_instance();

  CameraManager(const CameraManager&) = delete;
  CameraManager& operator=(const CameraManager&) = delete;

  void add_camera(CameraBehaviour* camera);
  void set_active_camera(CameraBehaviour* camera);
  inline CameraBehaviour* get_active_camera() const { return active_camera; }

private:
  CameraManager();

  std::vector<CameraBehaviour*> cameras;
  CameraBehaviour* active_camera;
};

}