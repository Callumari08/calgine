#pragma once

#include <glm/vec3.hpp>

namespace Calgine {

class MapEntityProperties
{
private:
  const std::unordered_map<std::string, std::string>& props;

public:
  MapEntityProperties(const std::unordered_map<std::string, std::string>& p) : props(p) {}
  
  // Check if a property exists
  bool has(const std::string& key) const 
  {
    return props.count(key) > 0;
  }
  
  // String property with default value
  std::string get_string(const std::string& key, const std::string& default_val = "") const 
  {
    auto iterator = props.find(key);
    return iterator != props.end() ? iterator->second : default_val;
  }
  
  // Float property with default value
  float get_float(const std::string& key, float default_val = 0.0f) const 
  {
    auto iterator = props.find(key);
    return iterator != props.end() ? std::stof(iterator->second) : default_val;
  }
  
  // Integer property with default value
  int get_int(const std::string& key, int default_val = 0) const 
  {
    auto iterator = props.find(key);
    return iterator != props.end() ? std::stoi(iterator->second) : default_val;
  }

  glm::vec3 get_vector3(const std::string& key, glm::vec3 default_val = {0, 0, 0}) const 
  {
  auto it = props.find(key);

  if (it == props.end())
    return default_val;

  std::stringstream ss(it->second);

  float x, y, z;

  if (ss >> x >> y >> z) {
    return {x, y, z};
  }

  return default_val;
}
  
};

}