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
namespace Theme {
  constexpr Color BLACK       = Color(0, 0, 0);
  constexpr Color WHITE       = Color(255, 255, 255);
  constexpr Color GRAY_DARK   = Color(50, 50, 50);
  constexpr Color GRAY_LIGHT  = Color(200, 200, 200);
  
  // Accent colors
  constexpr Color RED         = Color(255, 0, 0);
  constexpr Color CYAN        = Color(0, 255, 255);
  constexpr Color GREEN       = Color(0, 255, 0);
  constexpr Color YELLOW      = Color(255, 200, 0);
  constexpr Color MAGENTA     = Color(255, 0, 200);
  constexpr Color BLUE        = Color(100, 150, 255);
}

struct Rect {
  int x, y, w, h;
};

struct Point {
  int x, y;
};