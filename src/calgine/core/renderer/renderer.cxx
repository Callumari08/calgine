#include "renderer.h"
#include "calgine/core/renderer/camera/camera_behaviour.h"
#include "calgine/core/renderer/camera/camera_manager.h"

namespace Calgine {

GLenum Renderer::current_cull_state = GL_NONE;

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
  line_vertices.clear();
}

void Renderer::submit_line(const glm::vec3& from, const glm::vec3& to, const glm::vec4& colour)
{
  line_vertices.push_back(LineVertex {from, colour});
  line_vertices.push_back(LineVertex {to, colour});
}

void Renderer::submit(const Mesh* mesh, const Material* material, const glm::mat4 model_matrix)
{
  command_queue.emplace_back(BatchRenderCommand {mesh, material, model_matrix});
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
    command.material->bind();
    command.material->get_shader()->set_uniform_mat4("u_model", command.model_matrix);
    command.material->get_shader()->set_uniform_mat4("u_view", view_matrix);
    command.material->get_shader()->set_uniform_mat4("u_proj", projection_matrix);

    uint32_t slot = 0;
    for (const auto& [name, texture] : command.material->get_textures())
    {
      texture->bind(slot);
      command.material->get_shader()->set_uniform_1i(name, slot);

      slot++;
    }
    
    command.mesh->bind();
    command.mesh->draw();
  }

  command_queue.clear();

  flush_lines();
}

void Renderer::flush_lines()
{
  if (line_vertices.empty() || !had_camera_last_frame)
  {
    line_vertices.clear();
    return;
  }

  // Lazily create GL objects the first time lines are drawn (needs a live GL context).
  if (!line_shader)
    line_shader = std::make_unique<Shader>(ShaderProgram { LINE_FRAGMENT_SHADER, LINE_VERTEX_SHADER });

  if (line_vao == 0)
  {
    glGenVertexArrays(1, &line_vao);
    glGenBuffers(1, &line_vbo);

    glBindVertexArray(line_vao);
    glBindBuffer(GL_ARRAY_BUFFER, line_vbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (const void*)offsetof(LineVertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (const void*)offsetof(LineVertex, colour));
  }

  glBindVertexArray(line_vao);
  glBindBuffer(GL_ARRAY_BUFFER, line_vbo);

  const size_t byte_size = line_vertices.size() * sizeof(LineVertex);
  if (byte_size > line_vbo_capacity)
  {
    line_vbo_capacity = byte_size * 2;
    glBufferData(GL_ARRAY_BUFFER, line_vbo_capacity, nullptr, GL_DYNAMIC_DRAW);
  }
  glBufferSubData(GL_ARRAY_BUFFER, 0, byte_size, line_vertices.data());

  line_shader->bind();
  line_shader->set_uniform_mat4("u_view", view_matrix);
  line_shader->set_uniform_mat4("u_proj", projection_matrix);

  glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(line_vertices.size()));

  glBindVertexArray(0);
  line_vertices.clear();
}

void Renderer::release_gl_resources()
{
  line_shader.reset();

  if (line_vbo)
    glDeleteBuffers(1, &line_vbo);
  if (line_vao)
    glDeleteVertexArrays(1, &line_vao);

  line_vbo = 0;
  line_vao = 0;
  line_vbo_capacity = 0;
  line_vertices.clear();
}


}