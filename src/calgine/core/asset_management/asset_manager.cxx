#include "asset_manager.h"

#include "calgine/core/renderer/material.h"
#include "calgine/core/renderer/shader.h"
#include "calgine/core/renderer/texture.h"
#include "calgine_pch.h"
#include "calgine/core/log.h"
#include "calgine/core/asset_management/tiny_obj_loader.h"
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_error.h>
#include <SDL3_image/SDL_image.h>
#include <memory>

namespace Calgine {

class Shader;

AssetManager& AssetManager::get_instance()
{
  static AssetManager instance;
  return instance;
}

bool AssetManager::is_valid_file(const AssetType& type, const std::string& file_path) const
{
  SDL_PathInfo path_info;

  if (!SDL_GetPathInfo(file_path.c_str(), &path_info))
  {
    Log::get_engine_logger()->error("Failed to get {} at path {} \n\t SDL Error: {}", 
      get_asset_name(type), file_path, SDL_GetError());
    return false;
  }

  return true;
}

std::shared_ptr<Texture> AssetManager::load_texture(const std::string& name, const std::string& file_path)
{
  assert(name != "" && std::format("`name` not set for texture file: {}", file_path).c_str());

  if (!is_valid_file(texture, file_path))
    handle_asset_load_error(texture, file_path);

  // Texture serialization is almost entirely done by SDL, so we can just use it here.
  SDL_Surface* surface = IMG_Load(file_path.c_str());

  if (!surface)
    handle_asset_load_error(texture, file_path);

  textures.emplace(name, new Texture(surface));

  // Texture only accesses surface in the constructor, so we're fine to free it here.
  //SDL_DestroySurface(surface);

  return textures[name];
}

std::shared_ptr<Model> AssetManager::load_model(const std::string& name, const std::string& file_path)
{
  assert(name != "" && std::format("`name` not set for model file: {}", file_path).c_str());

  if (!is_valid_file(model, file_path))
    handle_asset_load_error(model, file_path);

  tinyobj::attrib_t attribute;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string err;

  // Extract directory from file path for material loading
  std::string mtl_basedir = file_path.substr(0, file_path.find_last_of("/\\") + 1);

  bool valid_model = tinyobj::LoadObj(&attribute, &shapes, &materials, &err, file_path.c_str(), mtl_basedir.c_str(), true);

  if (!err.empty())
  {
    // removes newline char from warning list
    err.pop_back();
    Log::get_engine_logger()->warn("Tiny Obj Loader Warnings:\n{}", err);
  }
  if (!valid_model) handle_asset_load_error(model, file_path);

  std::vector<Vertex> loaded_mesh;
  std::vector<uint32_t> indices;

  for (const tinyobj::shape_t& shape : shapes)
  {
    for (const tinyobj::index_t& index : shape.mesh.indices)
    {
      Vertex vertex{};
      vertex.colour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

      vertex.position.x = attribute.vertices[3 * index.vertex_index + 0];
      vertex.position.y = attribute.vertices[3 * index.vertex_index + 1];
      vertex.position.z = attribute.vertices[3 * index.vertex_index + 2];

      if (!attribute.normals.empty() && index.normal_index >= 0)
      {
        vertex.normal.x = attribute.normals[3 * index.normal_index + 0];
        vertex.normal.y = attribute.normals[3 * index.normal_index + 1];
        vertex.normal.z = attribute.normals[3 * index.normal_index + 2];
      }

      if (!attribute.texcoords.empty() && index.texcoord_index >= 0)
      {
        vertex.uv.x = attribute.texcoords[2 * index.texcoord_index + 0];
        vertex.uv.y = attribute.texcoords[2 * index.texcoord_index + 1];
      }

      indices.emplace_back(loaded_mesh.size());
      loaded_mesh.emplace_back(vertex);
    }
  }

  Mesh mesh(loaded_mesh, indices);

  auto material = std::make_shared<Material>();

  int num_materials = 0;
  for (auto& obj_material : materials)
  {
    num_materials++;

    material->set_colour("diffuse", {obj_material.diffuse[0], obj_material.diffuse[1], obj_material.diffuse[2], 1.0f});

    if (!obj_material.diffuse_texname.empty())
    {
      std::string texture_path = mtl_basedir + obj_material.diffuse_texname;
      std::optional<std::shared_ptr<Texture>> texture = AssetManager::get_texture(obj_material.diffuse_texname, texture_path);
      if (!texture)
        break;
      
      material->set_texture("albedo", *texture);
    }

    if (num_materials == 1)
      break;
  }
  if (num_materials == 0)
  {
    Log::get_engine_logger()->info("`{}` didn't come with any materials.", name);
  }

  models.emplace(name, std::make_shared<Model>(Model{std::move(mesh), material}));

  return models[name];
}

std::shared_ptr<Shader> AssetManager::load_shader(const std::string& name, const std::string fragment_path, const std::string vertex_path)
{
  // not sure if I wanna add support for not adding a certain type of shader file (ie fragment or vertex)

  assert(name != "" && std::format("`name` not set for shader file:").c_str());

  if (!is_valid_file(fragment, fragment_path))
    handle_asset_load_error(fragment, fragment_path);
  if (!is_valid_file(vertex, vertex_path))
    handle_asset_load_error(vertex, vertex_path);
  
  const std::string fragment_source = read_file_to_string(fragment_path);

  if (fragment_source == "")
    handle_asset_load_error(fragment, fragment_path);

  const std::string vertex_source = read_file_to_string(vertex_path);
  if (vertex_source == "")
    handle_asset_load_error(vertex, vertex_path);

  ShaderProgram program{fragment_source, vertex_source};
  shaders.emplace(name, new Shader(program));
  return shaders[name];
}

std::optional<std::shared_ptr<Texture>> AssetManager::get_texture(const std::string& name, const std::string else_file_path)
{
  if (name == "")
  {
    Log::get_engine_logger()->error("Provided texture key was empty!");
    return std::nullopt;
  }

  std::shared_ptr<Texture> texture = nullptr;

  // if texture IS in the map
  if (textures.find(name) != textures.end())
  {
    texture = textures[name];
  }

  if (!texture && else_file_path != "")
  {
    Log::get_engine_logger()->info("`{}` not found in loaded textures, loading from {}", name, else_file_path);
    texture = load_texture(name, else_file_path);
  }
  if (!texture)
  {
    return std::nullopt;
  }

  return texture;
}

std::optional<std::shared_ptr<Model>> AssetManager::get_model(const std::string& name, const std::string else_file_path)
{
  if (name == "")
  {
    Log::get_engine_logger()->error("Provided model key was empty!");
    return std::nullopt;
  }

  std::shared_ptr<Model> model = nullptr;

  // if mesh IS in the map
  if (models.find(name) != models.end())
  {
    model = models[name];
  }

  if (!model && else_file_path != "")
  {
    Log::get_engine_logger()->info("`{}` not found in loaded meshes, loading from {}", name, else_file_path);
    model = load_model(name, else_file_path);
  }
  if (!model)
  {
    return std::nullopt;
  }

  return model;
}

std::optional<std::shared_ptr<Shader>> AssetManager::get_shader(const std::string& name, const std::string else_fragment_path, const std::string else_vertex_path)
{
  if (name == "")
  {
    Log::get_engine_logger()->error("Provided shader key was empty!");
    return std::nullopt;
  }

  std::shared_ptr<Shader> shader = nullptr;

  // if shader IS in the map
  if (shaders.find(name) != shaders.end())
  {
    shader = shaders[name];
  }

  if (!shader && (else_fragment_path + else_vertex_path) != "")
  {
    Log::get_engine_logger()->info("`{}` not found in loaded shaders, loading fragment from {} and vertex from {}", name, else_fragment_path, else_vertex_path);
    shader = load_shader(name, else_fragment_path, else_vertex_path);
  }
  if (!shader)
  {
    return std::nullopt;
  }

  return shader;
}

inline std::string AssetManager::get_asset_name(const AssetType& type)
{
  switch (type)
  {
    case AssetType::texture:
      return "texture";
    case AssetType::model:
      return "model";
    case AssetType::fragment:
      return "fragment shader";
    case AssetType::vertex:
      return "vertex shader";
    default:
      return "";
  }
}

std::string AssetManager::get_app_path()
{
  return static_cast<std::string>(SDL_GetBasePath());
}

std::string AssetManager::get_asset_path(const std::string asset)
{
  return std::format("{}assets/{}", get_app_path(), asset);
}

std::string AssetManager::read_file_to_string(const std::string file_path)
{
  std::ifstream file(file_path);

  if (!file)
  {
    Log::get_engine_logger()->warn("Failed to read file: {}", file_path);
    return "";
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

void AssetManager::handle_asset_load_error(const AssetType& type, const std::string& file_path, std::string custom_msg)
{
  std::string msg = std::format("Handling error with {} at filepath {}.", get_asset_name(type), file_path);
  if (custom_msg != "")
  {
    msg += std::format("\n\t\t{}", custom_msg);
  }

  Log::get_engine_logger()->info(msg);
  // for now, crash. I'll do something smarter later.
  exit(-1);
}


}