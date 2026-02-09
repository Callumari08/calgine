#include "hierarchy_renderer.h"
#include "calgine/core/game_object.h"
#include "calgine/core/hierarchies/game_hierarchy.h"
#include <imgui.h>

namespace Calgine {

void HierarchyRenderer::render_tree_node(GameObject& go, GameObject* parent)
{
  if (go.get_parent().has_value() && &go.get_parent()->get() != parent)
    return;

  auto& root = GameHierarchy::get_instance().get_hierarchy_root();
  bool has_children = false;
  for (GameObject& child : root)
  {
    if (child.get_parent().has_value() && &child.get_parent()->get() == &go)
    {
      has_children = true;
      break;
    }
  }

  if (has_children)
  {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;
    if (selected == &go)
      flags |= ImGuiTreeNodeFlags_Selected;

    if (ImGui::TreeNodeEx(go.get_name().c_str(), flags))
    {
      if (ImGui::IsItemClicked())
        selected = &go;

      for (GameObject& child : root)
      {
        if (child.get_parent().has_value() && &child.get_parent()->get() == &go)
        {
          render_tree_node(child, &go);
        }
      }
      ImGui::TreePop();
    }
  }
  else
  {
    bool is_selected = (selected == &go);
    if (ImGui::Selectable(std::format("  {}", go.get_name()).c_str(), is_selected))
      selected = &go;
  }
}

void HierarchyRenderer::imgui_render_tick()
{
  if (!window_active)
    return;

  if (start)
  {
    ImVec2 size = ImVec2(300.0f, 500.0f);
    ImGui::SetNextWindowSize(size);

    start = false;
  }
  ImGui::Begin("Game Hierarchy", &window_active);
  
  ImGui::BeginChild("Scrolling");
  GameObject& root = GameHierarchy::get_instance().get_hierarchy_root();
  for (GameObject& go : root)
  {
    if (!go.get_parent().has_value())
    {
      render_tree_node(go, nullptr);
    }
  }
  ImGui::EndChild();
  ImGui::End();
}

} // namespace Calgine
