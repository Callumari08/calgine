#pragma once

#include "calgine/core/behaviour.h"
#include "calgine/core/renderer/mesh.h"
#include "calgine/core/renderer/render_texture.h"
#include "calgine/core/renderer/shader.h"
#include "calgine_api.h"
#include "calgine_pch.h"

namespace Calgine {

class CALGINE_API MeshRenderer : public Behaviour 
{
public:
  MeshRenderer() = default;
  MeshRenderer(std::shared_ptr<Mesh> _mesh, std::shared_ptr<Shader> _shader, std::shared_ptr<RenderTexture> _texture) 
    : mesh(_mesh), shader(_shader), texture(_texture) {}

  void set_mesh(std::shared_ptr<Mesh> mesh) { this->mesh = mesh; }
  void set_shader(std::shared_ptr<Shader> shader) { this->shader = shader; }

  std::shared_ptr<Mesh> get_mesh() { return mesh; }
  std::shared_ptr<Shader> get_shader() { return shader; }

private:
  std::shared_ptr<Mesh> mesh;
  std::shared_ptr<Shader> shader;
  std::shared_ptr<RenderTexture> texture;

  void render_tick() override;
};
}