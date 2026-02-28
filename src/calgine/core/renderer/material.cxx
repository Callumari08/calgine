#include "material.h"

#include "calgine/core/renderer/render_texture.h"
#include "calgine/core/renderer/texture.h"
#include "calgine/core/renderer/renderer.h"
#include "glad/gl.h"

namespace Calgine {

Material::Material(std::shared_ptr<Shader> shader) : shader(shader)
{
}

void Material::set_shader(const std::shared_ptr<Shader>& shader)
{
  this->shader = shader;
}

void Material::set_texture(const std::string& name, const std::shared_ptr<Texture>& texture)
{
  textures[name] = std::make_unique<RenderTexture>(texture);
}

void Material::set_colour(const std::string& name, const glm::vec4& colour)
{
  colours[name] = colour;
}

void Material::set_float(const std::string& name, const float& value)
{
  floats[name] = value;
}

void Material::bind() const
{
  if (!shader) return;

  GLenum desired_state = settings.cull_back_faces ? GL_BACK : GL_NONE;
  if (desired_state != Renderer::current_cull_state)
  {
    if (desired_state == GL_NONE) 
      glDisable(GL_CULL_FACE);
    else 
    {
      glEnable(GL_CULL_FACE);
      glCullFace(desired_state);
    }

    Renderer::current_cull_state = desired_state;
  }

  shader->bind();

  for (const auto& [name, colour] : colours)
  {
    shader->set_uniform_4f(name, colour.x, colour.y, colour.z, colour.w);
  }

  for (const auto& [name, value] : floats)
  {
    shader->set_uniform_1f(name, value);
  }

  uint32_t slot = 0;
  for (const auto& [name, texture] : textures)
  {
    if (texture)
    {
      texture->bind(slot);
      shader->set_uniform_1i(name, slot++);
    }
  }
}
}