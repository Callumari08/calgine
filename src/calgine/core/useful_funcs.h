#pragma once

#include <SDL3/SDL.h>
#include <stdexcept>

inline void throw_sdl_error(std::string message) {
  throw std::runtime_error(message + std::string(SDL_GetError()));
}

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