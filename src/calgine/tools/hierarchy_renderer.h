#pragma once

#include "calgine/core/behaviour.h"
#include "calgine/core/game_object.h"
#include "calgine_api.h"

namespace Calgine {

class CALGINE_API HierarchyRenderer : public Behaviour
{
private:
  bool window_active = true;
  bool start = true;
  GameObject* selected = nullptr;

  void render_tree_node(GameObject& go, GameObject* parent);

public:
  void imgui_render_tick() override;
  
  GameObject* get_selected() const { return selected; }
  void set_selected(GameObject* go) { selected = go; }
};

} // namespace Calgine
