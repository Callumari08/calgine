#pragma once

#include "calgine_pch.h"

namespace Calgine {

class DataSerializationInterface
{
  virtual ~DataSerializationInterface() = default;
  virtual bool has(const std::string& key) const = 0;
  virtual std::optional<std::string> get_string(const std::string& key) const = 0;
  virtual std::optional<int> get_int(const std::string& key) const = 0;
  virtual std::vector<DataSerializationInterface*> get_array(const std::string& key) const = 0;
  virtual bool is_string() const = 0;
  virtual std::string as_string() const = 0;
};
}