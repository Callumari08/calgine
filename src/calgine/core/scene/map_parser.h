#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include "calgine_api.h"

namespace Calgine {

struct MapEntity
{
  std::unordered_map<std::string, std::string> properties;
};

class CALGINE_API MapParser
{
public:
  static std::vector<MapEntity> parse(const std::string& file_path);

private:
  static std::string trim(const std::string& str);
  static std::unordered_map<std::string, std::string> parse_entity(const std::string& content, size_t& pos);
};

}
