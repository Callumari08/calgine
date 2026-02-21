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

  size_t size = width * height * channels;
  data.assign((uint8_t*) surface->pixels, (uint8_t*) surface->pixels + size);
};

}