#pragma once

#include "calgine/core/renderer/mesh.h"
#include "calgine/core/renderer/material.h"

namespace Calgine {

struct CALGINE_API Model
{
  Mesh mesh;
  std::shared_ptr<Material> material;
};
}