#include "gfx.hpp"
#include "gfx_backend.hpp"
#include "bitmap_font.hpp"

#include "../platform.hpp"
#include <cstring>

Graphics* create_graphics() {
#if PLATFORM_DESKTOP
  return new GraphicsSDL(DISPLAY_WIDTH, DISPLAY_HEIGHT);
#elif PLATFORM_ESP32
  return new GraphicsESP32(DISPLAY_WIDTH, DISPLAY_HEIGHT);
#else
  #error "No graphics backend for this platform"
#endif
}

// Clear the framebuffer
void Graphics::clear(Color color) {
  uint16_t* fb = get_framebuffer();
  int size = get_width() * get_height();
  for (int i = 0; i < size; i++) {
    fb[i] = color.value;
  }
}

// Fill a rectangle
void Graphics::fill_rect(int x, int y, int w, int h, Color color) {
  uint16_t* fb = get_framebuffer();
  int screen_w = get_width();
  int screen_h = get_height();

  for (int j = 0; j < h; j++) {
    int py = y + j;
    if (py < 0 || py >= screen_h) continue;

    for (int i = 0; i < w; i++) {
      int px = x + i;
      if (px < 0 || px >= screen_w) continue;

      fb[py * screen_w + px] = color.value;
    }
  }
}

// Draw a single pixel
void Graphics::draw_pixel(int x, int y, Color color) {
  if (x < 0 || x >= get_width() || y < 0 || y >= get_height()) return;

  uint16_t* fb = get_framebuffer();
  fb[y * get_width() + x] = color.value;
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

// Draw a circle (Midpoint circle algorithm)
void Graphics::draw_circle(int cx, int cy, int r, Color color) {
  int x = r;
  int y = 0;
  int err = 0;

  while (x >= y) {
    draw_pixel(cx + x, cy + y, color);
    draw_pixel(cx + y, cy + x, color);
    draw_pixel(cx - y, cy + x, color);
    draw_pixel(cx - x, cy + y, color);
    draw_pixel(cx - x, cy - y, color);
    draw_pixel(cx - y, cy - x, color);
    draw_pixel(cx + y, cy - x, color);
    draw_pixel(cx + x, cy - y, color);

    if (err <= 0) {
      y += 1;
      err += 2 * y + 1;
    }

    if (err > 0) {
      x -= 1;
      err -= 2 * x + 1;
    }
  }
}

// Helper function to get character index in font
static int get_char_index(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'Z') return 10 + (c - 'A');
  if (c >= 'a' && c <= 'z') return 10 + (c - 'a'); // Lowercase maps to uppercase

  // Special characters
  switch (c) {
    case ' ': return 36;
    case '.': return 37;
    case ':': return 38;
    case '/': return 39;
    case '+': return 40;
    case '-': return 41;
    case ',': return 42;
    case '!': return 43;
    case '?': return 44;
    case '%': return 45;
    default: return 36; // Default to space for unknown chars
  }
}

// Draw a single character
void Graphics::draw_char(char c, int x, int y, int scale, Color color) {
  int idx = get_char_index(c);

  for (int row = 0; row < 5; row++) {
    uint8_t bits = font_3x5[idx][row];
    for (int col = 0; col < 3; col++) {
      if (bits & (1 << (2 - col))) {
        // Draw scaled pixel
        fill_rect(x + col * scale, y + row * scale, scale, scale, color);
      }
    }
  }
}

// Draw text string
void Graphics::draw_text(const char* text, int x, int y, int scale, Color color) {
  int cursor_x = x;

  while (*text) {
    draw_char(*text, cursor_x, y, scale, color);
    cursor_x += 4 * scale; // 3 pixels + 1 spacing
    text++;
  }
}

// Measure text width
int Graphics::measure_text(const char* text, int scale) {
  int len = 0;
  while (text[len]) len++;
  return len * 4 * scale; // 3 pixels + 1 spacing per char
}