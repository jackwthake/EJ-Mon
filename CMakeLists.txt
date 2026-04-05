cmake_minimum_required(VERSION 3.20)
project(ej-mon-desktop)

# Require C++17 for modern features
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Disable SDL3 features we don't need
set(SDL_X11_XTEST OFF CACHE BOOL "" FORCE)

# Add SDL3 as a subdirectory
add_subdirectory(third-party/SDL)

# Define PLATFORM_DESKTOP macro for all targets
add_compile_definitions(PLATFORM_DESKTOP)

# Source files for the desktop application
set(SOURCES
  src/GraphicsController/DisplayDriver.cpp
  src/GraphicsController/gui.cpp
  src/GraphicsController/sprite.cpp
  src/GraphicsController/main.cpp
  src/can/can_parser.cpp
)

# Create the executable target
add_executable(ej-mon-desktop
  ${SOURCES}
)

# Include directories
target_include_directories(ej-mon-desktop PRIVATE
  src/GraphicsController
  src/log
  src/can
  src
  third-party/SDL/include
)

# Link against SDL3
target_link_libraries(ej-mon-desktop PRIVATE SDL3::SDL3)

# Print build information
message(STATUS "EJ-Mon Desktop Build Configuration")
message(STATUS "  Platform: DESKTOP (SDL3)")
message(STATUS "  SDL3 include dir: ${SDL3_INCLUDE_DIRS}")
message(STATUS "  Target: ej-mon-desktop")
