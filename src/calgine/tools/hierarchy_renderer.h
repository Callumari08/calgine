#pragma once

#include "calgine/core/behaviour.h"
#include "calgine/core/event_context.h"
#include "calgine/core/game_object.h"
#include "calgine_api.h"

namespace Calgine {

class CALGINE_API HierarchyRenderer : public Behaviour
{
public:
  HierarchyRenderer(GameObject& hierarchy_to_render, std::string window_title) : hierarchy(hierarchy_to_render), title(window_title) {}
  void imgui_render_tick(EventContext&) override;
  
  GameObject* get_selected() const { return selected; }
  void set_selected(GameObject* go) { selected = go; }

private:
  bool window_active = true;
  bool start = true;
  GameObject* selected = nullptr;

  std::string title;
  GameObject& hierarchy;

  void render_tree_node(GameObject& go, GameObject* parent);
};

CALGINE_REGISTER_BEHAVIOUR(HierarchyRenderer, "hierarchy_renderer");

}
