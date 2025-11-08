#pragma once
#include <cstdint>

#include "gfx_types.hpp"

class Graphics {
public:
  // Lifecycle
  virtual void begin() = 0;
  virtual void present() = 0;  // Swap/flush framebuffer to display
  
  // Framebuffer access
  virtual uint16_t* get_framebuffer() = 0;
  virtual int get_width() const = 0;
  virtual int get_height() const = 0;
  
  // Clear & fill
  void clear(Color color);
  void fill_rect(int x, int y, int w, int h, Color color);
  
  // Primitives
  void draw_pixel(int x, int y, Color color);
  void draw_line(int x0, int y0, int x1, int y1, Color color);
  void draw_rect(int x, int y, int w, int h, Color color);
  void draw_circle(int cx, int cy, int r, Color color);
  
  // Text rendering
  void draw_char(char c, int x, int y, int scale, Color color);
  void draw_text(const char* text, int x, int y, int scale, Color color);
  int measure_text(const char* text, int scale);  // Returns width

  // Virtual destructor
  virtual ~Graphics() = default;
};

// Factory function - creates appropriate backend
Graphics* create_graphics();