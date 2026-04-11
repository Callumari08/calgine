#pragma once

#include "calgine_api.h"

namespace Calgine {

enum CALGINE_API TickType
{
  any,
  preloop,
  fixed_update,
  update,
  late_update,
  render,
  imgui_render,
  final,
};
}