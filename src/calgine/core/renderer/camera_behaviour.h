#pragma once

#include "calgine/core/behaviour.h"
#include "calgine/core/renderer/camera.h"
#include "calgine/core/renderer/camera_manager.h"

class CameraManager;

namespace Calgine {

struct CameraSettings
{
  float fov = 65.0f;
  float aspect_ratio = 16.0f / 9.0f;
  float near_plane = 0.1f;
  float far_plane = 1000.0f;
};

class CameraBehaviour : public Behaviour
{
public:
  CameraBehaviour();
  
  inline bool is_active() const { return camera_active; }

  inline const CameraSettings get_settings() const { return settings; }
  void apply_settings(const CameraSettings settings);

  inline Camera& get_raw_camera() { return  camera; }

private:
  Camera camera;
  CameraSettings settings;
  bool camera_active = false;

  void inline set_camera_active(bool active_state) { camera_active = active_state; }

  void start_tick() override;
  void late_tick() override;

  friend class CameraManager;
};

}