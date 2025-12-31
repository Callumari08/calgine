#ifndef USEFUL_FUNCS
#define USEFUL_FUNCS

#include <SDL3/SDL.h>
#include <stdexcept>

inline void throw_sdl_error(std::string message) {
  throw std::runtime_error(message + std::string(SDL_GetError()));
}
#endif