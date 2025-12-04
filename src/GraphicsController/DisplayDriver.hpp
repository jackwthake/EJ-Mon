#pragma once

#include <Arduino_GFX_Library.h>

#include "gfx.hpp"

// Wrapper class for HDC458002C40 display with RGB interface and I2C expander
class Display_HDC458002C40 : public Arduino_RGB_Display, public Graphics {
public:
  Display_HDC458002C40();
  
  void begin(void);
  void present(void);
  
  // Override of Arduino_RGB_Display functions for Graphics interface
  inline uint16_t *getFramebuffer() override { return back_buf; };
  inline int get_width() const override { return SCREEN_BUF_WIDTH; };
  inline int get_height() const override { return SCREEN_BUF_HEIGHT; };
  
  inline int get_x_offset() const override { return 0; };
  inline int get_stride() const override { return SCREEN_BUF_WIDTH; };
  
  static constexpr unsigned SCREEN_BUF_WIDTH = 960;
  static constexpr unsigned SCREEN_BUF_HEIGHT = 320;
private:
  static constexpr unsigned TFT_WIDTH = 400;
  static constexpr unsigned TFT_HEIGHT = 960;

  static constexpr unsigned TFT_PIXEL_CLOCK = 24000000; // 24 mhz
  static constexpr unsigned BOUNCE_BUF_SIZE = (TFT_WIDTH) * 2 * 16; // 48 scanline buffer, 80 collumn blanking

  uint16_t *back_buf;

  // Peripheral definitions
  // I2C expander for backlight and reset
  static Arduino_XCA9554SWSPI *expander;

  // RGB panel definition
  static Arduino_ESP32RGBPanel *rgbpanel;
};