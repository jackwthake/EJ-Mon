#pragma once
#include <cstdint>

#include "gfx_types.hpp"

class Graphics {
public:
  // Lifecycle
  virtual void begin() = 0;
  virtual void present() = 0;  // Swap/flush framebuffer to display
  
  // Framebuffer access
  virtual uint16_t* getFramebuffer() = 0;
  virtual int get_width() const = 0;
  virtual int get_height() const = 0;

  // Framebuffer stride (may differ from width due to hardware offsets)
  virtual int get_stride() const { return get_width(); }
  // X offset into framebuffer (for displays with column offsets)
  virtual int get_x_offset() const { return 0; }
  
  // Clear & fill
  void clear(Color color);
  void fill_rect(int x, int y, int w, int h, Color color);
  
  // Primitives
  void draw_pixel(int x, int y, Color color);
  void draw_line(int x0, int y0, int x1, int y1, Color color);
  void draw_rect(int x, int y, int w, int h, Color color);

  inline uint16_t get_pixel(int x, int y) {
    if (x < 0 || x >= get_width() || y < 0 || y >= get_height()) return 0;
    uint16_t* fb = this->getFramebuffer();
    return fb[y * get_stride() + get_x_offset() + x]; // Note: Adjusted for rotated framebuffer
  }
  
  // Virtual destructor
  virtual ~Graphics() = default;
};

// Factory function - creates appropriate backend
Graphics* create_graphics();