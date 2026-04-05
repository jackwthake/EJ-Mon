#pragma once

#include "gfx_types.hpp"

// Platform selection: PLATFORM_ESP32 or PLATFORM_DESKTOP (set by build system)
#ifdef PLATFORM_ESP32
  #include <Arduino_GFX_Library.h>
#else // PLATFORM_DESKTOP
  #include <SDL3/SDL.h>
  #include <cstdint>
  #include <cstdlib>
#endif


// Wrapper class for HDC458002C40 display with RGB interface and I2C expander
// Supports both ESP32 (hardware RGB panel via Arduino_GFX) and Desktop (SDL3 window)
#ifdef PLATFORM_ESP32
class Display_HDC458002C40 : public Arduino_RGB_Display {
#else
class Display_HDC458002C40 {
#endif

public:
  Display_HDC458002C40();
  ~Display_HDC458002C40();
  
  void begin(void);
  void present(void);
  
  // Override of Arduino_RGB_Display functions (ESP32 only)
  #ifdef PLATFORM_ESP32
  inline void draw16bitRGBBitmap(int16_t x, int16_t y, uint16_t *bitmap, int16_t w, int16_t h) override {
    Arduino_RGB_Display::draw16bitRGBBitmap(-TFT_COL_OFFSET + x, TFT_COL_OFFSET + y, bitmap, w, h);
  }
  #endif

  inline uint16_t *getFramebuffer() { return back_buf; };
  inline int get_width() const { return SCREEN_BUF_WIDTH; };
  inline int get_height() const { return SCREEN_BUF_HEIGHT; };
  
  // Drawing methods consolidated from Graphics
  void clear(Color color);
  void fill_rect(int x, int y, int w, int h, Color color);
  
  inline uint16_t get_pixel(int x, int y) {
    if (x < 0 || x >= get_width() || y < 0 || y >= get_height()) return 0;
    uint16_t* fb = this->getFramebuffer();
    return fb[y * SCREEN_BUF_WIDTH + x];
  }
  
  static constexpr unsigned SCREEN_BUF_WIDTH = 960;
  static constexpr unsigned SCREEN_BUF_HEIGHT = 320;
  static constexpr unsigned TFT_COL_OFFSET = 80;

private:
  uint16_t *back_buf;

  #ifdef PLATFORM_ESP32
    static constexpr unsigned TFT_WIDTH = 400;
    static constexpr unsigned TFT_HEIGHT = 960;
    static constexpr unsigned TFT_PIXEL_CLOCK = 28000000; // 30 mhz
    static constexpr unsigned BOUNCE_BUF_SIZE = (TFT_WIDTH) * 32; // 32 scanline buffer

    // Peripheral definitions (ESP32 only)
    static Arduino_XCA9554SWSPI *expander;
    static Arduino_ESP32RGBPanel *rgbpanel;
  #else // PLATFORM_DESKTOP
    SDL_Window *sdl_window;
    SDL_Surface *sdl_surface;
  #endif
};