
#include "mesh_renderer.h"

#include "calgine/core/event_context.h"
#include "calgine/core/renderer/renderer.h"
#include "calgine/core/game_object.h"
#include "calgine/core/behaviour_serialization/behaviour_register_macro.h"

namespace Calgine {

  void MeshRenderer::render_tick(EventContext&)
  {
    if (!model_ptr || !model_ptr->material || !model_ptr->material->get_shader()) return;

    /*const Material& mat = *model_ptr->material;
    const auto& textures = mat.get_textures();

    RenderTexture* texture = nullptr;

    // if has albedo
    if (textures.find("albedo") != textures.end())
    {
      texture = textures.at("albedo").get();
    }*/

    // TODO: update the renderer to support multiple textures (ie. normal maps, albedo etc)
    Renderer::get_instance().submit(&model_ptr->mesh, model_ptr->material.get(), get_game_object()->get_transform().to_matrix());
  }

CALGINE_REGISTER_BEHAVIOUR(MeshRenderer, "mesh_renderer");

}