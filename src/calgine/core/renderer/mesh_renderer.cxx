
#include "mesh_renderer.h"

#include "calgine/core/game_object.h"
#include "calgine/core/renderer/renderer.h"

namespace Calgine {

  void MeshRenderer::render_tick()
  {
    if (!mesh || !shader) return;

    Renderer::get_instance().submit(mesh.get(), shader.get(), 
      texture.get(), get_game_object()->get_transform().to_matrix());
  }
}