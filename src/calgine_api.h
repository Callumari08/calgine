#pragma once

#ifdef _WIN32
  #ifdef CALGINE_EXPORTS
    #define CALGINE_API __declspec(dllexport)
  #else
    #define CALGINE_API __declspec(dllimport)
  #endif
#else
  #define CALGINE_API __attribute__((visibility("default")))
#endif
