#pragma once

#include "calgine/core/behaviour.h"
#include "calgine/core/renderer/camera.h"
namespace Calgine {

struct CameraSettings
{
  
};

class CameraBehaviour final : public Behaviour
{
public:
  CameraBehaviour();
  Camera& get_camera() { return camera; }
  
  inline void set_camera_active(bool active_state) { camera_active = active_state; }
  inline bool is_active() { return camera_active; }

private:
  Camera camera;
  CameraSettings settings;
  bool camera_active = false;

  void late_tick() override;
};

}