#pragma once
#include <cstdint>

#include "gfx_types.hpp"

class Framebuffer {
public:
  Framebuffer(int width, int height);
  ~Framebuffer();
  
  // Direct pixel access
  void set_pixel(int x, int y, Color color);
  Color get_pixel(int x, int y) const;
  
  // Bulk operations
  void clear(Color color);
  void fill_rect(int x, int y, int w, int h, Color color);
  
  // Raw buffer access
  inline uint16_t* data() { return buffer_; }
  const inline uint16_t* data() const { return buffer_; }
  
  inline int width() const { return width_; }
  inline int height() const { return height_; }

private:
  uint16_t* buffer_;
  int width_;
  int height_;
  
  inline bool in_bounds(int x, int y) const {
    return x >= 0 && x < width_ && y >= 0 && y < height_;
  }
};