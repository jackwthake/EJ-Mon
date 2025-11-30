#pragma once

#include <Arduino_GFX_Library.h>

#include "gfx.hpp"

// Wrapper class for HDC458002C40 display with RGB interface and I2C expander
class Display_HDC458002C40 : public Arduino_RGB_Display, public Graphics {
public:
  Display_HDC458002C40();
  
  void begin(void);
  void present(void);
  
  uint16_t *get_framebuffer() override { return back_buf; };
  int get_width() const override { return TFT_WIDTH; };
  int get_height() const override { return TFT_HEIGHT; };
  
  int get_x_offset() const override { return 0; };
  
  static constexpr unsigned SCREEN_BUF_WIDTH = 320;
  static constexpr unsigned SCREEN_BUF_HEIGHT = 960;
private:
  static constexpr unsigned TFT_WIDTH = 320;
  static constexpr unsigned TFT_HEIGHT = 960;

  static constexpr unsigned TFT_PIXEL_CLOCK = 24800000; // 24.8 mhz
  static constexpr unsigned BOUNCE_BUF_SIZE = (TFT_WIDTH + 80) * 2 * 16; // 48 scanline buffer, 80 collumn blanking

  uint16_t *back_buf;

  // Peripheral definitions
  // I2C expander for backlight and reset
  static Arduino_XCA9554SWSPI *expander;

  // RGB panel definition
  static Arduino_ESP32RGBPanel *rgbpanel;
};