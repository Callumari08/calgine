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

  SDL_Surface* surface = IMG_Load(file_path.c_str());

  if (!surface)
    handle_asset_load_error(texture, file_path);

  textures.emplace(name, new Texture(surface));

  SDL_DestroySurface(surface);

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

std::unordered_map<std::string, ActionMap> AssetManager::load_input_config(const std::string& file_path)
{
  std::unordered_map<std::string, ActionMap> action_maps;

  if (!is_valid_file(texture, file_path))
  {
    Log::get_engine_logger()->error("Input config file not found: {}", file_path);
    return action_maps;
  }

  try
  {
    auto config = toml::parse_file(file_path);

    // Iterate over each action map in the config
    for (auto& [map_name, map_value] : config)
    {
      if (!map_value.is_table())
      {
        Log::get_engine_logger()->warn("Input config: '{}' is not a table, skipping", map_name);
        continue;
      }

      auto& map_table = *map_value.as_table();
      ActionMap action_map;

      // Extract actions from this map
      for (auto& [key, value] : map_table)
      {
        if (!value.is_array())
        {
          Log::get_engine_logger()->warn("Input config: '{}' action is not an array, skipping", std::string(key));
          continue;
        }

        InputAction action;
        action.name = std::string(key);

        // Iterate over mappings in the action
        for (auto& mapping_value : *value.as_array())
        {
          if (!mapping_value.is_table())
          {
            Log::get_engine_logger()->warn("Input config: mapping in '{}' is not a table, skipping", key);
            continue;
          }

          try
          {
            InputMapping mapping = parse_input_mapping(*mapping_value.as_table());
            action.mappings.push_back(mapping);
          }
          catch (const std::exception& e)
          {
            Log::get_engine_logger()->warn("Input config: failed to parse mapping: {}", e.what());
            continue;
          }
        }

        // Determine control type from first mapping's contribution
        if (!action.mappings.empty())
        {
          const auto& first_contribution = action.mappings[0].contribution;
          if (std::holds_alternative<InputButton>(first_contribution))
            action.control_type = ControlType::button;
          else if (std::holds_alternative<float>(first_contribution))
            action.control_type = ControlType::vector1;
          else if (std::holds_alternative<glm::vec2>(first_contribution))
            action.control_type = ControlType::vector2;
          else if (std::holds_alternative<glm::vec3>(first_contribution))
            action.control_type = ControlType::vector3;
        }

        action_map.map[action.name] = action;
      }

      action_maps[std::string(map_name)] = action_map;
    }
  }
  catch (const toml::parse_error& e)
  {
    Log::get_engine_logger()->error("Failed to parse input config file: {}\n{}", file_path, e.what());
  }
  catch (const std::exception& e)
  {
    Log::get_engine_logger()->error("Error loading input config: {}", e.what());
  }

  return action_maps;
}

InputMapping AssetManager::parse_input_mapping(const toml::table& mapping_table)
{
  InputMapping mapping;

  // Parse type
  if (auto type_value = mapping_table.get("type"))
  {
    if (type_value->is_string())
      mapping.type = string_to_event_type(std::string(*type_value->as_string()));
    else
      throw std::runtime_error("'type' field must be a string");
  }
  else
    throw std::runtime_error("'type' field is required");

  // Parse input
  if (auto input_value = mapping_table.get("input"))
  {
    if (input_value->is_string())
      mapping.matcher = parse_matcher(mapping.type, std::string(*input_value->as_string()));
    else
      throw std::runtime_error("'input' field must be a string");
  }
  else
    throw std::runtime_error("'input' field is required");

  // Parse contribution
  if (auto contribution_value = mapping_table.get("contribution"))
  {
    if (!contribution_value->is_array())
      throw std::runtime_error("'contribution' field must be an array");
    
    // Guess control type from contribution array size or content
    // We'll pass a dummy type and let parse_contribution infer it
    auto contrib_array = *contribution_value->as_array();
    
    if (contrib_array.size() == 1)
    {
      // Could be button or axis
      mapping.contribution = parse_contribution(ControlType::vector1, contrib_array);
    }
    else if (contrib_array.size() == 2)
    {
      mapping.contribution = parse_contribution(ControlType::vector2, contrib_array);
    }
    else if (contrib_array.size() == 3)
    {
      mapping.contribution = parse_contribution(ControlType::vector3, contrib_array);
    }
    else
      throw std::runtime_error("'contribution' array must have 1-3 elements");
  }
  else
    throw std::runtime_error("'contribution' field is required");

  return mapping;
}

RawInputEventType AssetManager::string_to_event_type(const std::string& type_str)
{
  if (type_str == "keyboard")
    return RawInputEventType::keyboard;
  else if (type_str == "mouse_button")
    return RawInputEventType::mouse_button;
  else if (type_str == "mouse_move")
    return RawInputEventType::mouse_move;
  else if (type_str == "mouse_wheel")
    return RawInputEventType::mouse_wheel;
  else
    throw std::runtime_error(std::format("Unknown input type: {}", type_str));
}

std::variant<SDL_Scancode, Uint8> AssetManager::parse_matcher(RawInputEventType type, const std::string& input_str)
{
  if (type == RawInputEventType::keyboard)
  {
    return string_to_scancode(input_str);
  }
  else if (type == RawInputEventType::mouse_button)
  {
    return string_to_mouse_button(input_str);
  }
  else if (type == RawInputEventType::mouse_move || type == RawInputEventType::mouse_wheel)
  {
    // mouse_move and mouse_wheel don't need specific matchers, use dummy value
    return SDL_SCANCODE_UNKNOWN;
  }
  else
    throw std::runtime_error(std::format("Cannot parse matcher for type: {}", static_cast<int>(type)));
}

std::variant<InputButton, float, glm::vec2, glm::vec3> AssetManager::parse_contribution(ControlType control_type, const toml::array& contribution_array)
{
  if (contribution_array.size() == 1)
  {
    if (auto val = contribution_array.get(0))
    {
      if (val->is_boolean())
        return InputButton{val->as_boolean()->get()};
      else if (val->is_floating_point())
        return static_cast<float>(val->as_floating_point()->get());
      else if (val->is_integer())
        return static_cast<float>(val->as_integer()->get());
    }
    throw std::runtime_error("Failed to parse single-element contribution");
  }
  else if (contribution_array.size() == 2)
  {
    float x = 0.f, y = 0.f;
    if (auto val = contribution_array.get(0))
    {
      if (val->is_floating_point())
        x = static_cast<float>(val->as_floating_point()->get());
      else if (val->is_integer())
        x = static_cast<float>(val->as_integer()->get());
    }
    if (auto val = contribution_array.get(1))
    {
      if (val->is_floating_point())
        y = static_cast<float>(val->as_floating_point()->get());
      else if (val->is_integer())
        y = static_cast<float>(val->as_integer()->get());
    }
    return glm::vec2(x, y);
  }
  else if (contribution_array.size() == 3)
  {
    float x = 0.f, y = 0.f, z = 0.f;
    if (auto val = contribution_array.get(0))
    {
      if (val->is_floating_point())
        x = static_cast<float>(val->as_floating_point()->get());
      else if (val->is_integer())
        x = static_cast<float>(val->as_integer()->get());
    }
    if (auto val = contribution_array.get(1))
    {
      if (val->is_floating_point())
        y = static_cast<float>(val->as_floating_point()->get());
      else if (val->is_integer())
        y = static_cast<float>(val->as_integer()->get());
    }
    if (auto val = contribution_array.get(2))
    {
      if (val->is_floating_point())
        z = static_cast<float>(val->as_floating_point()->get());
      else if (val->is_integer())
        z = static_cast<float>(val->as_integer()->get());
    }
    return glm::vec3(x, y, z);
  }
  else
    throw std::runtime_error("Contribution array size out of range");
}

SDL_Scancode AssetManager::string_to_scancode(const std::string& key_name)
{
  // Map common key names to scancodes
  static const std::map<std::string, SDL_Scancode> key_map = {
    {"W", SDL_SCANCODE_W}, {"A", SDL_SCANCODE_A}, {"S", SDL_SCANCODE_S}, {"D", SDL_SCANCODE_D},
    {"E", SDL_SCANCODE_E}, {"F", SDL_SCANCODE_F}, {"Q", SDL_SCANCODE_Q}, {"R", SDL_SCANCODE_R},
    {"Space", SDL_SCANCODE_SPACE}, {"Enter", SDL_SCANCODE_RETURN}, {"Escape", SDL_SCANCODE_ESCAPE},
    {"Shift", SDL_SCANCODE_LSHIFT}, {"Ctrl", SDL_SCANCODE_LCTRL}, {"Alt", SDL_SCANCODE_LALT},
    {"Up", SDL_SCANCODE_UP}, {"Down", SDL_SCANCODE_DOWN}, {"Left", SDL_SCANCODE_LEFT}, {"Right", SDL_SCANCODE_RIGHT},
    {"Tab", SDL_SCANCODE_TAB}, {"Backspace", SDL_SCANCODE_BACKSPACE},
  };

  auto it = key_map.find(key_name);
  if (it != key_map.end())
    return it->second;
  
  throw std::runtime_error(std::format("Unknown key name: {}", key_name));
}

Uint8 AssetManager::string_to_mouse_button(const std::string& button_name)
{
  if (button_name == "left")
    return SDL_BUTTON_LEFT;
  else if (button_name == "right")
    return SDL_BUTTON_RIGHT;
  else if (button_name == "middle")
    return SDL_BUTTON_MIDDLE;
  else
    throw std::runtime_error(std::format("Unknown mouse button: {}", button_name));
}

void AssetManager::clear()
{
  textures.clear();
  models.clear();
  shaders.clear();
}


}