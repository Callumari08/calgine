#pragma once

#include "calgine_api.h"
#include "calgine_pch.h"
#include "calgine/core/renderer/texture.h"

namespace Calgine {

class CALGINE_API RenderTexture
{
public:
  RenderTexture(const std::shared_ptr<Texture>& texture);
  ~RenderTexture();

  void bind(uint32_t slot = 0) const;
  void unbind() const;

private:
  uint32_t renderer_id = 0;
  std::shared_ptr<Texture> texture_ptr;
};

}