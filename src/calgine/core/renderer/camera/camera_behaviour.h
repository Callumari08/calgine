#pragma once

#include "calgine/core/behaviour.h"
#include "camera.h"
#include "camera_manager.h"
#include "calgine_api.h"

class CameraManager;

namespace Calgine {

struct CameraSettings
{
  float fov = 65.0f;
  float aspect_ratio = 16.0f / 9.0f;
  float near_plane = 0.1f;
  float far_plane = 1000.0f;
};

class CALGINE_API CameraBehaviour : public Behaviour
{
public:
  CameraBehaviour(bool show_debug = false);
  
  inline bool is_active() const { return camera_active; }

  inline const CameraSettings get_settings() const { return settings; }
  void apply_settings(const CameraSettings settings);

  Camera& get_raw_camera() { return  camera; }

private:
  Camera camera;
  CameraSettings settings;
  bool camera_active = false;
  bool debug = false;

  void set_camera_active(bool active_state) { camera_active = active_state; }

  void start_tick() override;
  void late_tick(EventContext&) override;

  void imgui_render_tick(EventContext&) override;

  friend class CameraManager;
};

}