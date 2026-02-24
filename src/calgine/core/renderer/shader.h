#pragma once

#include "calgine_api.h"
#include "calgine_pch.h"
#include <glad/gl.h>
#include <glm/glm.hpp>

namespace Calgine {

constexpr const char* DEFAULT_VERTEX_SHADER = R"(#version 450 core
layout(location = 0) in vec4 a_colour;
layout(location = 1) in vec3 a_position;
layout(location = 2) in vec3 a_normal;
layout(location = 3) in vec2 a_texcoord;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;

out vec4 v_colour;
out vec2 v_texcoord;

void main()
{
  v_colour = a_colour;
  v_texcoord = a_texcoord;
  gl_Position = u_proj * u_view * u_model * vec4(a_position, 1.0);
}
)";

constexpr const char* DEFAULT_FRAGMENT_SHADER = R"(#version 450 core
in vec4 v_colour;
in vec2 v_texcoord;
uniform sampler2D u_texture;
out vec4 o_color;

void main()
{
  o_color = texture(u_texture, v_texcoord) * v_colour;
}
)";

struct ShaderProgram
{
  std::string fragment;
  std::string vertex;
  // TODO: add compute
};

class CALGINE_API Shader
{
public:
  Shader(const ShaderProgram& program);
  ~Shader();

  void bind() const;
  void unbind() const;

  void set_uniform_1i(const std::string& name, int value) const;
  void set_uniform_1f(const std::string& name, float value) const;
  void set_uniform_2f(const std::string& name, float v0, float v1) const;
  void set_uniform_3f(const std::string& name, float v0, float v1, float v2) const;
  void set_uniform_4f(const std::string& name, float v0, float v1, float v2, float v3) const;
  void set_uniform_mat4(const std::string& name, const glm::mat4& matrix) const;

private:
  uint32_t renderer_id = 0;
  mutable std::unordered_map<std::string, GLint> uniform_cache;

  GLint get_uniform_location(const std::string& name) const;
};

}