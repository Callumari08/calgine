#pragma once

#include "calgine/core/behaviour.h"
#include "calgine/core/event_context.h"
#include "calgine/core/renderer/model.h"
#include "calgine_api.h"
#include "calgine_pch.h"

namespace Calgine {

class CALGINE_API MeshRenderer : public Behaviour 
{
public:
  MeshRenderer() = default;
  MeshRenderer(std::shared_ptr<Model> model) 
    : model_ptr(model) {}

  std::shared_ptr<Model> get_model() { return model_ptr; }

  void set_material(std::shared_ptr<Material> material) { model_ptr->material = material; }

private:
  std::shared_ptr<Model> model_ptr;

  void render_tick(EventContext&) override;
};
}