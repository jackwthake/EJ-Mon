#include "platform.h"
#include <cstdio>

// Example 1: Conditional includes
#if PLATFORM_DESKTOP
  #include <SDL3/SDL.h>
#elif PLATFORM_ESP32
  // #include <esp_system.h>
  // #include <driver/spi_master.h>
#endif

// Example 2: Platform-specific implementations
void platform_init() {
#if PLATFORM_DESKTOP
  printf("Initializing SDL desktop platform\n");
  SDL_Init(SDL_INIT_VIDEO);
#elif PLATFORM_ESP32
  printf("Initializing ESP32 platform\n");
  // esp_system_init();
#endif
}

// Example 3: Shared interface, different backends
void display_set_pixel(int x, int y, uint32_t color) {
#if PLATFORM_DESKTOP
  // Use SDL renderer
  // SDL_RenderPoint(renderer, x, y);
#elif PLATFORM_ESP32
  // Write directly to TFT framebuffer
  // tft_set_pixel(x, y, color);
#endif
}

// Example 4: Conditional code blocks
void print_platform_info() {
#if PLATFORM_DESKTOP
  printf("Running on: Desktop SDL Simulator\n");
  printf("Display: Windowed with SDL3\n");
#elif PLATFORM_ESP32
  printf("Running on: ESP32\n");
  printf("Display: 960x320 RGB-666 TFT\n");
#endif
}

// Example 5: Using macros for cleaner code
const char* get_platform_name() {
#if PLATFORM_DESKTOP
  return "Desktop";
#elif PLATFORM_ESP32
  return "ESP32";
#endif
}
