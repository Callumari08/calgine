#pragma once

#include "calgine/core/renderer/camera_behaviour.h"
namespace Calgine {


// For now, Only support one camera
class CameraManager
{
  CameraBehaviour* active_camera;
};
}