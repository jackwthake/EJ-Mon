#pragma once

#include <cstdint>

// RGB565 color format (native for most TFT displays)
struct Color {
  uint16_t value;
  
  constexpr Color(uint16_t v) : value(v) {}
  constexpr Color(uint8_t r, uint8_t g, uint8_t b) 
    : value(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)) {}
  
  static constexpr Color rgb(uint8_t r, uint8_t g, uint8_t b) {
    return Color(r, g, b);
  }
};

// OP-1 inspired color palette
// Names prefixed with C_ to avoid macro collisions with Arduino libs
namespace Theme {
  constexpr Color C_BLACK       = Color(0, 0, 0);
  constexpr Color C_WHITE       = Color(255, 255, 255);
  constexpr Color C_GRAY_DARK   = Color(50, 50, 50);
  constexpr Color C_GRAY_LIGHT  = Color(200, 200, 200);

  // Accent colors
  constexpr Color C_RED         = Color(255, 0, 0);
  constexpr Color C_CYAN        = Color(0, 255, 255);
  constexpr Color C_GREEN       = Color(0, 255, 0);
  constexpr Color C_YELLOW      = Color(255, 200, 0);
  constexpr Color C_MAGENTA     = Color(255, 0, 200);
  constexpr Color C_BLUE        = Color(100, 150, 255);
}

struct Rect {
  int x, y, w, h;
};

struct Point {
  int x, y;
};