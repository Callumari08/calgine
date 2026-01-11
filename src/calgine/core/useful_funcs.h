#pragma once

#include "calgine/core/log.h"
#include <SDL3/SDL.h>

/**
 * @brief allows for the name of an arbritrary type to be casted to a string, irrespective of the target machine/compiler. 
 * 
 * @tparam T 
 * @return constexpr std::string_view 
 */
template<typename T>
constexpr std::string_view type_name()
{
#if defined(__clang__)
    std::string_view p = __PRETTY_FUNCTION__;
    return p.substr(p.find("T = ") + 4, p.rfind(']') - (p.find("T = ") + 4));
#elif defined(__GNUC__)
    std::string_view p = __PRETTY_FUNCTION__;
    return p.substr(p.find("with T = ") + 9, p.rfind(';') - (p.find("with T = ") + 9));
#elif defined(_MSC_VER)
    std::string_view p = __FUNCSIG__;
    return p.substr(p.find("<") + 1, p.rfind(">") - (p.find("<") + 1));
#else
    return "unknown";
#endif
}

inline std::string convert_unsigned_char_ptr_to_str(const unsigned char* str)
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