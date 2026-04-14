#pragma once

#include "calgine_api.h"
#include "calgine_pch.h"
#include "calgine/core/renderer/texture.h"
#include "calgine/core/renderer/shader.h"
#include "calgine/core/renderer/model.h"
#include "calgine/core/input/input_system.h"
#include <toml++/toml.h>

namespace Calgine {

enum AssetType
{
  texture,
  model,
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
  std::shared_ptr<Model> load_model(const std::string& name, const std::string& file_path);
  std::shared_ptr<Shader> load_shader(const std::string& name, const std::string fragment_path, const std::string vertex_path);

  /// Load input action maps from a TOML file
  /// @param file_path Path to the TOML config file (e.g., "assets/config/input.toml")
  /// @return Map of action map name to ActionMap
  /// 
  /// Example TOML format:
  /// @code
  /// [Gameplay]
  /// Move = [
  ///   { type = "keyboard", input = "W", contribution = [0, 1] },
  ///   { type = "keyboard", input = "A", contribution = [-1, 0] },
  /// ]
  /// Jump = [
  ///   { type = "keyboard", input = "Space", contribution = [true] },
  /// ]
  /// @endcode
  ///
  /// Supported input types: "keyboard", "mouse_button", "mouse_move", "mouse_wheel"
  /// Contribution array: [value] for button/axis, [x, y] for vec2, [x, y, z] for vec3
  std::unordered_map<std::string, ActionMap> load_input_config(const std::string& file_path);

  // getting allows you to optionally point to an asset path if it doesn't already exist in the map.
  // one should generally use get rather than load, unless they specifically want to overwrite data in the map.

  std::optional<std::shared_ptr<Texture>> get_texture(const std::string& name, const std::string else_file_path = "");
  std::optional<std::shared_ptr<Model>> get_model(const std::string& name, const std::string else_file_path = "");
  std::optional<std::shared_ptr<Shader>> get_shader(const std::string& name, const std::string else_fragment_path = "", const std::string else_vertex_path = "");
  //std::optional<std::shared_ptr<Material>> get_material(const std::shared_ptr<Mesh>, const )

 

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

  // Input config parsing helpers
  InputMapping parse_input_mapping(const toml::table& mapping_table);
  RawInputEventType string_to_event_type(const std::string& type_str);
  std::variant<SDL_Scancode, Uint8> parse_matcher(RawInputEventType type, const std::string& input_str);
  std::variant<InputButton, float, glm::vec2, glm::vec3> parse_contribution(ControlType control_type, const toml::array& contribution_array);
  SDL_Scancode string_to_scancode(const std::string& key_name);
  Uint8 string_to_mouse_button(const std::string& button_name);

  std::unordered_map<std::string, std::shared_ptr<Texture>> textures;
  std::unordered_map<std::string, std::shared_ptr<Model>> models;
  std::unordered_map<std::string, std::shared_ptr<Shader>> shaders;
};

}