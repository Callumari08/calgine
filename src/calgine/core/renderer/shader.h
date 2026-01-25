#pragma once

#include "calgine_pch.h"
#include "glad/gl.h"
#include <glm/glm.hpp>
#include <unordered_map>

namespace Calgine {

constexpr const char* DEFAULT_VERTEX_SHADER = R"(#version 450 core
layout(location = 0) in vec3 a_Position;
out vec3 v_Position;

void main()
{
  v_Position = a_Position;
  gl_Position = vec4(a_Position, 1.0);
}
)";

constexpr const char* DEFAULT_FRAGMENT_SHADER = R"(#version 450 core
layout(location = 0) out vec4 o_color;
in vec3 v_Position;

void main()
{
  o_color = vec4(v_Position, 1.0);
}
)";


class Shader
{
public:
  Shader(const std::string& vertex_source, const std::string& fragment_source);
  ~Shader();

  void bind() const;
  void unbind() const;

  void set_uniform_1i(const std::string& name, int value);
  void set_uniform_1f(const std::string& name, float value);
  void set_uniform_2f(const std::string& name, float v0, float v1);
  void set_uniform_3f(const std::string& name, float v0, float v1, float v2);
  void set_uniform_4f(const std::string& name, float v0, float v1, float v2, float v3);
  void set_uniform_mat4(const std::string& name, const glm::mat4& matrix);

private:
  uint32_t renderer_id = 0;
  std::unordered_map<std::string, GLint> uniform_cache;

  GLint get_uniform_location(const std::string& name);
};

}