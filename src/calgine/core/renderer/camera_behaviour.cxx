#pragma once

#include "camera_behaviour.h"
#include "calgine/core/renderer/camera.h"

namespace Calgine {

void CameraBehaviour::late_tick() 
{
  camera.update(get_transform());
}

}