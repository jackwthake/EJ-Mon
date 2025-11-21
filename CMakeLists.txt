cmake_minimum_required(VERSION 3.20)

project(ej-mon VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Determine target platform
if(NOT DEFINED TARGET_PLATFORM)
  set(TARGET_PLATFORM "DESKTOP" CACHE STRING "Target platform: DESKTOP or ESP32")
endif()

# Normalize platform name to uppercase
string(TOUPPER "${TARGET_PLATFORM}" TARGET_PLATFORM_UPPER)

# Platform-specific configuration
if(TARGET_PLATFORM_UPPER STREQUAL "DESKTOP")
  message(STATUS "Building for DESKTOP (SDL simulator)")

  # SDL configuration
  set(SDL_SHARED OFF CACHE BOOL "Build SDL as shared library" FORCE)
  set(SDL_STATIC ON CACHE BOOL "Build SDL as static library" FORCE)
  set(SDL_TEST_LIBRARY OFF CACHE BOOL "Build SDL test library" FORCE)

  # Disable optional X11 features that may not be installed
  set(SDL_X11_XTEST OFF CACHE BOOL "Disable XTEST extension" FORCE)

  add_subdirectory(third-party/SDL)

  # shader-works configuration - always build in Release mode for performance
  set(SHADER_WORKS_USE_THREADS OFF CACHE BOOL "Disable threading in shader-works" FORCE)
  set(SHADER_WORKS_BUILD_EXAMPLES OFF CACHE BOOL "Don't build shader-works examples" FORCE)
  set(SHADER_WORKS_BUILD_DEMOS OFF CACHE BOOL "Don't build shader-works demos" FORCE)

  # Force Release mode for shader-works subdirectory
  set(CMAKE_BUILD_TYPE_SAVED ${CMAKE_BUILD_TYPE})
  set(CMAKE_BUILD_TYPE Release)
  add_subdirectory(lib/shader-works)
  set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE_SAVED})

  # Main executable - uses test_main.cpp for desktop
  add_executable(ej-mon
    src/test_main.cpp
    src/can/can_parser.cpp
    src/graphics/gfx.cpp
    src/graphics/gfx_sdl.cpp
    src/graphics/framebuffer.cpp
    src/graphics/sprite.cpp
    src/gui.cpp
  )

  target_link_libraries(ej-mon PRIVATE
    SDL3::SDL3-static
    shader-works
  )

  target_include_directories(ej-mon PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    /home/jwt/.arduino15/packages/esp32/hardware/esp32/3.3.3/cores/esp32
    /home/jwt/.arduino15/packages/esp32/hardware/esp32/3.3.3/variants/esp32s3
    /home/jwt/.arduino15/packages/esp32/hardware/esp32/3.3.3/libraries/*/src
    /home/jwt/.arduino15/packages/esp32/hardware/esp32/3.3.3/libraries/*
    /home/jwt/Arduino/libraries/Adafruit_FT6206_Library
    /home/jwt/Arduino/libraries/Adafruit_CST8XX_Library
    /home/jwt/Arduino/libraries/Adafruit_BusIO
    /home/jwt/Arduino/libraries/Adafruit_BusIO/src
    /home/jwt/Arduino/libraries/Adafruit_GFX_Library
    /home/jwt/Arduino/libraries/Adafruit_GFX_Library/src
    /home/jwt/Arduino/libraries/GFX_Library_for_Arduino/src
    /home/jwt/Arduino/libraries/GFX_Library_for_Arduino/src/databus
  )

  # Compiler flags for desktop
  target_compile_options(ej-mon PRIVATE
    -Wall
    -Wextra
    -Wpedantic
  )

  # Define platform macros
  target_compile_definitions(ej-mon PRIVATE
    ARDUINO=10819
    ARDUINO_ESP32S3_DEV
    ARDUINO_ARCH_ESP32
    PLATFORM_DESKTOP=1
    PLATFORM_ESP32=0
  )

elseif(TARGET_PLATFORM_UPPER STREQUAL "ESP32")
  message(STATUS "Building for ESP32-S3 (Adafruit Qualia RGB666)")
  message(STATUS "")
  message(STATUS "ESP32 build requires Arduino-ESP32 toolchain.")
  message(STATUS "Use the provided build-esp32.sh script instead:")
  message(STATUS "  ./build-esp32.sh")
  message(STATUS "")
  message(STATUS "To flash via TinyUF2 bootloader:")
  message(STATUS "  1. Double-tap reset to enter bootloader (drive appears as QUALIAS3BOOT)")
  message(STATUS "  2. Copy build-esp32/ej-mon.uf2 to the drive")
  message(STATUS "")
  message(FATAL_ERROR "Use ./build-esp32.sh for ESP32 builds")

else()
  message(FATAL_ERROR "Unknown TARGET_PLATFORM: ${TARGET_PLATFORM}")
endif()
