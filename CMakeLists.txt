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
  )

  target_include_directories(ej-mon PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
  )

  # Compiler flags for desktop
  target_compile_options(ej-mon PRIVATE
    -Wall
    -Wextra
    -Wpedantic
  )

  # Define platform macros
  target_compile_definitions(ej-mon PRIVATE
    PLATFORM_DESKTOP=1
    PLATFORM_ESP32=0
  )

elseif(TARGET_PLATFORM_UPPER STREQUAL "ESP32")
  message(STATUS "Building for ESP32")
  # ESP32-IDF integration will go here
  # This will be added when you have the toolchain set up

  # Main executable - uses main.cpp for ESP32
  # add_executable(ej-mon
  #   src/main.cpp
  #   src/can/can_parser.cpp
  # )

  # Define platform macros for ESP32
  # target_compile_definitions(ej-mon PRIVATE
  #   PLATFORM_DESKTOP=0
  #   PLATFORM_ESP32=1
  # )

  message(FATAL_ERROR "ESP32 build not yet implemented")

else()
  message(FATAL_ERROR "Unknown TARGET_PLATFORM: ${TARGET_PLATFORM}")
endif()
