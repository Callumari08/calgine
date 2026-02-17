#include "renderer.h"
#include "calgine/core/renderer/camera_behaviour.h"
#include "calgine/core/renderer/camera_manager.h"

namespace Calgine {

Renderer& Renderer::get_instance()
{
  static Renderer instance;
  return instance;
}

void Renderer::begin_frame()
{
  CameraBehaviour* active_camera = CameraManager::get_instance().get_active_camera();
  if (!active_camera)
  {
    if (had_camera_last_frame)
    {
      Log::get_engine_logger()->warn("No active camera in scene! Stopping render until a camera is active.");
      had_camera_last_frame = false;
    }

    return;
  }
  if (!had_camera_last_frame)
  {
    Log::get_engine_logger()->info("There is now an active camera in the scene. Resuming render.");
    had_camera_last_frame = true;
  }

  view_matrix = active_camera->get_raw_camera().get_view_matrix();
  projection_matrix = active_camera->get_raw_camera().get_projection_matrix();

  command_queue.clear();
}

void Renderer::submit(const Mesh& mesh, const Shader& shader, const glm::mat4 model_matrix)
{
  command_queue.emplace_back(BatchRenderCommand {&mesh, &shader, model_matrix});
}

void Renderer::submit(const BatchRenderCommand cmd)
{
  command_queue.emplace_back(cmd);
}

void Renderer::end_frame()
{
  flush();
}

void Renderer::flush()
{
  for (const BatchRenderCommand& command : command_queue)
  {
    command.shader->bind();
    command.shader->set_uniform_mat4("u_model", command.model_matrix);
    command.shader->set_uniform_mat4("u_view", view_matrix);
    command.shader->set_uniform_mat4("u_proj", projection_matrix);
    
    command.mesh->bind();
    command.mesh->draw();
  }
}


}