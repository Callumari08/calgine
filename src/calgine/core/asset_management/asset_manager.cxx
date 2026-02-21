#include "asset_manager.h"
#include "calgine/core/log.h"
#include "calgine/core/asset_management/tiny_obj_loader.h"
#include "calgine/core/renderer/mesh.h"
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_error.h>
#include <SDL3_image/SDL_image.h>

namespace Calgine {

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

std::shared_ptr<Mesh> AssetManager::load_mesh(const std::string& name, const std::string& file_path)
{
  assert(name != "" && std::format("`name` not set for mesh file: {}", file_path).c_str());

  tinyobj::attrib_t attribute;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string err;

  bool valid_mesh = tinyobj::LoadObj(&attribute, &shapes, &materials, &err, file_path.c_str(), NULL, true);

  if (!err.empty())
  {
    // removes newline char from warning list
    err.pop_back();
    Log::get_engine_logger()->warn("Tiny Obj Loader Warnings:\n{}", err);
  }
  if (!valid_mesh) handle_asset_load_error(mesh, file_path);

  std::vector<Vertex> loaded_mesh;
  std::vector<uint32_t> indices;

  for (const tinyobj::shape_t& shape : shapes)
  {
    for (const tinyobj::index_t& index : shape.mesh.indices)
    {
      Vertex vertex{};

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

  meshes.emplace(name, new Mesh(loaded_mesh, indices));
  return meshes[name];
}

std::shared_ptr<Shader> AssetManager::load_shader(const std::string& name, const std::string& file_path)
{
  assert(name != "" && std::format("`name` not set for shader file: {}", file_path).c_str());

  assert(false && "Unimplemented");
}

inline std::string AssetManager::get_asset_name(const AssetType& type)
{
  switch (type)
  {
    case AssetType::texture:
      return "texture";
    case AssetType::mesh:
      return "mesh";
    case AssetType::shader:
      return "shader";
    default:
      return "";
  }
}

std::string AssetManager::get_app_path()
{
  return (std::string) SDL_GetBasePath();
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