#include "gfx.hpp"
#include <cstring>

// Clear the framebuffer
void Graphics::clear(Color color) {
  uint16_t* fb = get_framebuffer();
  int stride = get_stride();
  int x_off = get_x_offset();
  int screen_w = get_width();
  int screen_h = get_height();

  for (int y = 0; y < screen_h; y++) {
    for (int x = 0; x < screen_w; x++) {
      fb[y * stride + x_off + x] = color.value;
    }
  }
}

// Fill a rectangle
void Graphics::fill_rect(int x, int y, int w, int h, Color color) {
  uint16_t* fb = get_framebuffer();
  int stride = get_stride();
  int x_off = get_x_offset();
  int screen_w = get_width();
  int screen_h = get_height();

  for (int j = 0; j < h; j++) {
    int py = y + j;
    if (py < 0 || py >= screen_h) continue;

    for (int i = 0; i < w; i++) {
      int px = x + i;
      if (px < 0 || px >= screen_w) continue;

      this->draw_pixel(px, py, color.value);
    }
  }
}

// Draw a single pixel
void Graphics::draw_pixel(int x, int y, Color color) {
  if (x < 0 || x >= get_width() || y < 0 || y >= get_height()) return;

  uint16_t* fb = get_framebuffer();
  fb[x * get_stride() + get_x_offset() + y] = color.value; // Note: Adjusted for rotated framebuffer
}

// Draw a line (Bresenham's algorithm)
void Graphics::draw_line(int x0, int y0, int x1, int y1, Color color) {
  int dx = x1 - x0;
  int dy = y1 - y0;

  if (dx < 0) dx = -dx;
  if (dy < 0) dy = -dy;

  int sx = x0 < x1 ? 1 : -1;
  int sy = y0 < y1 ? 1 : -1;
  int err = dx - dy;

  while (true) {
    draw_pixel(x0, y0, color);

    if (x0 == x1 && y0 == y1) break;

    int e2 = 2 * err;
    if (e2 > -dy) {
      err -= dy;
      x0 += sx;
    }
    if (e2 < dx) {
      err += dx;
      y0 += sy;
    }
  }
}

// Draw a rectangle outline
void Graphics::draw_rect(int x, int y, int w, int h, Color color) {
  draw_line(x, y, x + w - 1, y, color);           // Top
  draw_line(x, y + h - 1, x + w - 1, y + h - 1, color); // Bottom
  draw_line(x, y, x, y + h - 1, color);           // Left
  draw_line(x + w - 1, y, x + w - 1, y + h - 1, color); // Right
}
