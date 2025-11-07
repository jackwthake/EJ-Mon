#pragma once

// Platform detection macros
// These are set by CMake via target_compile_definitions

#ifndef PLATFORM_DESKTOP
#define PLATFORM_DESKTOP 0
#endif

#ifndef PLATFORM_ESP32
#define PLATFORM_ESP32 0
#endif

// Conditional compilation helpers
#if PLATFORM_DESKTOP
  #define IF_DESKTOP(x) x
  #define IF_ESP32(x)
#elif PLATFORM_ESP32
  #define IF_DESKTOP(x)
  #define IF_ESP32(x) x
#else
  #error "No platform defined"
#endif

constexpr int DISPLAY_WIDTH = 960;
constexpr int DISPLAY_HEIGHT = 320;