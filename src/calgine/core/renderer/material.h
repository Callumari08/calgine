#pragma once

#include "calgine/core/renderer/render_texture.h"
#include "calgine_api.h"
#include "calgine_pch.h"
#include "calgine/core/renderer/shader.h"
#include "glm/fwd.hpp"

namespace Calgine {

struct CALGINE_API MaterialSettings
{
  bool cull_back_faces = true;
};

class CALGINE_API Material
{
public:
  Material(std::shared_ptr<Shader> shader = nullptr);
  ~Material() = default;
  Material(const Material&) = delete;
  Material& operator=(const Material&) = delete;

  void bind() const;
  void set_shader(const std::shared_ptr<Shader>& shader);
  void set_texture(const std::string& name, const std::shared_ptr<Texture>& texture);
  void set_colour(const std::string& name, const glm::vec4& colour);
  void set_float(const std::string& name, const float& value);

  std::shared_ptr<Shader> get_shader() const { return shader; }
  const std::unordered_map<std::string, std::unique_ptr<RenderTexture>>& get_textures() const { return textures; }

  MaterialSettings settings;
private:
  std::shared_ptr<Shader> shader;
  
  std::unordered_map<std::string, std::unique_ptr<RenderTexture>> textures;
  std::unordered_map<std::string, glm::vec4> colours;
  std::unordered_map<std::string, float> floats;
};
}