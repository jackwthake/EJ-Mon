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


  static constexpr int DISPLAY_WIDTH = 960;
  static constexpr int DISPLAY_HEIGHT = 320;

#elif PLATFORM_ESP32
  #define IF_DESKTOP(x)
  #define IF_ESP32(x) x

  #pragma message("Compiling for ESP32 platform")

  static constexpr int DISPLAY_WIDTH = 320;
  static constexpr int DISPLAY_HEIGHT = 960;

  static constexpr int SCREEN_BUFFER_WIDTH = 960;
  static constexpr int SCREEN_BUFFER_HEIGHT = 400;
#else
  #error "No platform defined"
#endif

