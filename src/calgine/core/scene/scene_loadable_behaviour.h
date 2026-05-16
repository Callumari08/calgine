#pragma once

#include "calgine_api.h"

#include <rapidjson/fwd.h>

namespace Calgine {

class SceneLoadContext;

class CALGINE_API SceneLoadableBehaviour
{
public:
  virtual ~SceneLoadableBehaviour() = default;
  virtual void load_from_json(const rapidjson::Value& data, SceneLoadContext& load_context) = 0;
};

} // namespace Calgine
