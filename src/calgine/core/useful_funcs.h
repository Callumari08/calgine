#pragma once

#include "calgine/core/log.h"
#include <SDL3/SDL.h>

// Having a file called "useful_funcs.h" is obviously stupid, but to be honest I don't know what else to do...
// Could just call it "utilities.h", which is kind of the equivalent of sweeping trash under rug

inline std::string convert_GLubyte_ptr_to_str(const unsigned char* str)
{
  std::string value(reinterpret_cast<const char*>(str));

  return value;
}

namespace Calgine {

/**
 * @brief prints the latest SDL error.
 * 
 * @param message 
 */
inline void throw_sdl_error(std::string message) {
  Log::get_engine_logger()->error(message);
  throw std::runtime_error(message + std::string(SDL_GetError()));
}

} // namespace Calgine