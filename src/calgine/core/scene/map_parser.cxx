#include "map_parser.h"
#include "calgine/core/log.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace Calgine {

std::string MapParser::trim(const std::string& str)
{
  size_t start = str.find_first_not_of(" \t\r\n");
  if (start == std::string::npos) return "";
  size_t end = str.find_last_not_of(" \t\r\n");
  return str.substr(start, end - start + 1);
}

std::unordered_map<std::string, std::string> MapParser::parse_entity(const std::string& content, size_t& pos)
{
  std::unordered_map<std::string, std::string> properties;
  
  // Find opening brace
  size_t open_brace = content.find('{', pos);
  if (open_brace == std::string::npos) 
  {
    pos = content.length();  // Advance to end
    return properties;
  }
  
  // Find closing brace
  size_t close_brace = content.find('}', open_brace);
  if (close_brace == std::string::npos) 
  {
    pos = content.length();  // Advance to end
    return properties;
  }
  
  std::string entity_content = content.substr(open_brace + 1, close_brace - open_brace - 1);
  pos = close_brace + 1;
  
  std::istringstream stream(entity_content);
  std::string line;
  
  while (std::getline(stream, line))
  {
    line = trim(line);
    if (line.empty() || line[0] == '/' || line[0] == ';') continue;
    
    // Parse key/value pair: "key" "value"
    size_t first_quote = line.find('"');
    if (first_quote == std::string::npos) continue;
    
    size_t second_quote = line.find('"', first_quote + 1);
    if (second_quote == std::string::npos) continue;
    
    std::string key = line.substr(first_quote + 1, second_quote - first_quote - 1);
    
    size_t third_quote = line.find('"', second_quote + 1);
    if (third_quote == std::string::npos) continue;
    
    size_t fourth_quote = line.find('"', third_quote + 1);
    if (fourth_quote == std::string::npos) continue;
    
    std::string value = line.substr(third_quote + 1, fourth_quote - third_quote - 1);
    
    properties[key] = value;
  }
  
  return properties;
}

std::vector<MapEntity> MapParser::parse(const std::string& file_path)
{
  std::vector<MapEntity> entities;
  
  std::ifstream file(file_path);
  if (!file.is_open())
  {
    Log::get_engine_logger()->error("Failed to open map file: {}", file_path);
    return entities;
  }
  
  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string content = buffer.str();
  file.close();
  
  size_t pos = 0;
  while (pos < content.length())
  {
    auto properties = parse_entity(content, pos);
    if (!properties.empty())
    {
      entities.push_back({properties});
    }
  }
  
  return entities;
}

}
