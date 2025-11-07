#include "gfx.hpp"
#include "gfx_backend.hpp"

#include "../platform.h"

Graphics* create_graphics() {
#if PLATFORM_DESKTOP
  return new GraphicsSDL(DISPLAY_WIDTH, DISPLAY_HEIGHT);
#elif PLATFORM_ESP32
  return new GraphicsESP32(DISPLAY_WIDTH, DISPLAY_HEIGHT);
#else
  #error "No graphics backend for this platform"
#endif
}