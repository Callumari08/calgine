#pragma once

#include "calgine_api.h"
#include "calgine/core/renderer/mesh.h"
#include "calgine/core/renderer/material.h"
#include "glad/gl.h"
#include <glm/fwd.hpp>

namespace Calgine {

struct CALGINE_API BatchRenderCommand
{
  const Mesh* mesh;
  const Material* material;
  glm::mat4 model_matrix;
};

struct CALGINE_API LineVertex
{
  glm::vec3 position;
  glm::vec4 colour;
};

class Shader;

class CALGINE_API Renderer
{
public:
  static Renderer& get_instance();

  static GLenum current_cull_state;

  void submit(const Mesh* mesh, const Material* material, const glm::mat4 model_matrix);
  void submit(const BatchRenderCommand cmd);

  /**
   * @brief Queues a world-space line to be drawn this frame (after meshes). Useful for debug drawing.
   */
  void submit_line(const glm::vec3& from, const glm::vec3& to, const glm::vec4& colour);


private:
  Renderer() = default;

  void begin_frame();
  void end_frame();
  void flush();
  void flush_lines();

  /** @brief Frees GL objects owned by the renderer. Must be called while the GL context is alive. */
  void release_gl_resources();

  std::vector<BatchRenderCommand> command_queue;

  std::vector<LineVertex> line_vertices;
  std::unique_ptr<Shader> line_shader;
  GLuint line_vao = 0;
  GLuint line_vbo = 0;
  size_t line_vbo_capacity = 0;
  glm::mat4 view_matrix;
  glm::mat4 projection_matrix;

  bool had_camera_last_frame = true;

  friend class App;
};

}