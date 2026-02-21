#pragma once

#include "calgine_api.h"
#include "SDL3/SDL_surface.h"

namespace Calgine {

class CALGINE_API Texture
{
public:
  uint32_t get_width() const { return width; }
  uint32_t get_height() const { return height; }
  uint32_t get_channels() const { return channels; }
  const uint8_t* get_data() const { return data.data(); }

private:
  Texture(const SDL_Surface* surface);

  std::vector<uint8_t> data;
  
  uint32_t width = 0;
  uint32_t height = 0;
  uint32_t channels = 0;

  friend class AssetManager;
};

}