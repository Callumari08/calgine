#pragma once

#include "tick_type.h"


namespace Calgine {

struct CALGINE_API EventData
{
  TickType submit_tick = any;
  virtual ~EventData() = default;
};
}