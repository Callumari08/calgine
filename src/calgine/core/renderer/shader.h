#pragma once

#include "calgine_pch.h"

namespace Calgine {

class Shader
{
public:
  Shader(const std::string& vertex_source, const std::string& fragment_source);
  ~Shader();

  void bind();
  void unbind();

private:
  uint32_t renderer_id = 0;
};

}