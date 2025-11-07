#include "framebuffer.hpp"

Framebuffer::Framebuffer(int width, int height) : width_(width), height_(height) {
  buffer_ = new uint16_t[width * height];
}

Framebuffer::~Framebuffer() {
  delete[] buffer_;
}

void Framebuffer::set_pixel(int x, int y, Color color) {
  if (in_bounds(x, y)) {
    buffer_[y * width_ + x] = color.value;
  }
}

Color Framebuffer::get_pixel(int x, int y) const {
  if (in_bounds(x, y)) {
    return Color(buffer_[y * width_ + x]);
  }
  return Color(0); // Return black for out-of-bounds
}

void Framebuffer::clear(Color color) {
  for (int i = 0; i < width_ * height_; i++) {
    buffer_[i] = color.value;
  }
}

void Framebuffer::fill_rect(int x, int y, int w, int h, Color color) {
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      set_pixel(x + i, y + j, color);
    }
  }
}