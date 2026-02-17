#pragma once

#include "calgine_api.h"
#include "calgine/core/renderer/mesh.h"
#include "calgine/core/renderer/shader.h"
#include <glm/fwd.hpp>

namespace Calgine {

struct CALGINE_API BatchRenderCommand
{
  const Mesh* mesh;
  const Shader* shader;
  glm::mat4 model_matrix;
};

class CALGINE_API Renderer
{
public:
  static Renderer& get_instance();

  void submit(const Mesh& mesh, const Shader& shader, const glm::mat4 model_matrix);
  void submit(const BatchRenderCommand cmd);

private:
  Renderer() = default;

  void begin_frame();
  void end_frame();
  void flush();

  std::vector<BatchRenderCommand> command_queue;
  glm::mat4 view_matrix;
  glm::mat4 projection_matrix;

  bool had_camera_last_frame = true;

  friend class App;
};

}