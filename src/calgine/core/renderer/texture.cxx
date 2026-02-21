#include "texture.h"

#include "SDL3/SDL_pixels.h"

namespace Calgine {

Texture::Texture(const SDL_Surface* surface)
{
  width = surface->w;
  height = surface->h;
  
  const SDL_PixelFormatDetails* details = SDL_GetPixelFormatDetails(surface->format);
  if (!details)
  {
    channels = 4; // default to RGBA
  }
  else
  {
    channels = details->bytes_per_pixel;
  }

  size_t row_size = width * channels;
  data.reserve(height * row_size);
  
  const uint8_t* pixels = (const uint8_t*)surface->pixels;
  for (uint32_t y = 0; y < height; ++y)
  {
    data.insert(data.end(), pixels, pixels + row_size);
    pixels += surface->pitch;
  }
};

}