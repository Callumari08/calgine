#pragma once

#include "calgine_api.h"
#include "calgine_pch.h"
#include "calgine/core/renderer/texture.h"
#include "calgine/core/renderer/mesh.h"
#include "calgine/core/renderer/shader.h"

namespace Calgine {

enum AssetType
{
  texture,
  mesh,
  fragment,
  vertex,
};

class CALGINE_API AssetManager
{
public:
  static AssetManager& get_instance();

  // load overwrites the map
  // load asserts that the file in the path exists.
  std::shared_ptr<Texture> load_texture(const std::string& name, const std::string& file_path);
  std::shared_ptr<Mesh> load_mesh(const std::string& name, const std::string& file_path);
  std::shared_ptr<Shader> load_shader(const std::string& name, const std::string fragment_path, const std::string vertex_path);

  // getting allows you to optionally point to an asset path if it doesn't already exist in the map.
  // one should generally use get rather than load, unless they specifically want to overwrite data in the map.

  std::optional<std::shared_ptr<Texture>> get_texture(const std::string& name, const std::string else_file_path = "");
  std::optional<std::shared_ptr<Mesh>> get_mesh(const std::string& name, const std::string else_file_path = "");
  std::optional<std::shared_ptr<Shader>> get_shader(const std::string& name, const std::string else_fragment_path = "", const std::string else_vertex_path = "");

  //void unload_texture(const std::string name);
  //void unload_mesh(const std::string name);
  //void unload_shader(const std::string name);

  static std::string get_app_path();
  static std::string get_asset_path(const std::string asset);
  static std::string read_file_to_string(const std::string file_path);

private:
  static inline std::string get_asset_name(const AssetType& type);
  bool is_valid_file(const AssetType& type, const std::string& file_path) const;

  void handle_asset_load_error(const AssetType& type, const std::string& file_path, std::string custom_msg = "");

  std::unordered_map<std::string, std::shared_ptr<Texture>> textures;
  std::unordered_map<std::string, std::shared_ptr<Mesh>> meshes;
  std::unordered_map<std::string, std::shared_ptr<Shader>> shaders;
};

}