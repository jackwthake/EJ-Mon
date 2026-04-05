#include "sprite.hpp"
#include "atlas_data.h"
#include "DisplayDriver.hpp"

#include <cstring>
#include <cstdio>
#include <cmath>

#ifdef PLATFORM_ESP32
  #include <pgmspace.h>
  #include <esp_heap_caps.h>
#else
  #include <cstdlib>
#endif

#include "../log/log.hpp"

SpriteAtlas::SpriteAtlas() : pixels(nullptr), width(0), height(0) {
}

SpriteAtlas::~SpriteAtlas() {
  if (pixels) {
    #ifdef PLATFORM_ESP32
      heap_caps_free(pixels);
    #else
      free(pixels);
    #endif
    pixels = nullptr;
  }
}

#ifndef PLATFORM_ESP32
// Simple BMP loader (supports 24-bit and 32-bit BMPs) - Desktop only
bool SpriteAtlas::load_from_file(const char* filename) {
  FILE* f = fopen(filename, "rb");
  if (!f) {
    fprintf(stderr, "Failed to open sprite atlas: %s\n", filename);
    return false;
  }

  // Read BMP header
  uint8_t header[54];
  if (fread(header, 1, 54, f) != 54) {
    fprintf(stderr, "Invalid BMP file: %s\n", filename);
    fclose(f);
    return false;
  }

  // Check BMP signature
  if (header[0] != 'B' || header[1] != 'M') {
    fprintf(stderr, "Not a BMP file: %s\n", filename);
    fclose(f);
    return false;
  }

  // Extract image info
  uint32_t data_offset = *(uint32_t*)&header[10];
  width = *(int32_t*)&header[18];
  height = *(int32_t*)&header[22];
  uint16_t bpp = *(uint16_t*)&header[28];

  if (bpp != 24 && bpp != 32) {
    fprintf(stderr, "Unsupported BMP format (must be 24 or 32 bpp): %s\n", filename);
    fclose(f);
    return false;
  }

  // Allocate pixel buffer
  pixels = new uint16_t[width * height];

  // Seek to pixel data
  fseek(f, data_offset, SEEK_SET);

  // BMP is stored bottom-up, so we read it backwards
  int bytes_per_pixel = bpp / 8;
  int row_size = ((width * bytes_per_pixel + 3) / 4) * 4; // Row size padded to 4 bytes

  uint8_t* row_buffer = new uint8_t[row_size];

  for (int y = height - 1; y >= 0; y--) {
    if (fread(row_buffer, 1, row_size, f) != (size_t)row_size) {
      fprintf(stderr, "Failed to read BMP data: %s\n", filename);
      delete[] row_buffer;
      delete[] pixels;
      pixels = nullptr;
      fclose(f);
      return false;
    }

    // Convert BGR(A) to RGB565
    for (int x = 0; x < width; x++) {
      uint8_t b = row_buffer[x * bytes_per_pixel + 0];
      uint8_t g = row_buffer[x * bytes_per_pixel + 1];
      uint8_t r = row_buffer[x * bytes_per_pixel + 2];

      // Convert to RGB565
      uint16_t r5 = (r >> 3) & 0x1F;
      uint16_t g6 = (g >> 2) & 0x3F;
      uint16_t b5 = (b >> 3) & 0x1F;

      pixels[y * width + x] = (r5 << 11) | (g6 << 5) | b5;
    }
  }

  delete[] row_buffer;
  fclose(f);
  return true;
}

#endif

bool SpriteAtlas::load_from_rgb565(const uint16_t* data, int w, int h) {
  if (!data || w <= 0 || h <= 0) return false;

  // Clean up existing data
  if (pixels) {
    #ifdef PLATFORM_ESP32
      heap_caps_free(pixels);
    #else
      free(pixels);
    #endif
    pixels = nullptr;
  }

  width = w;
  height = h;
  size_t num_pixels = (size_t)w * h;
  size_t data_size = num_pixels * sizeof(uint16_t);

  #ifdef PLATFORM_ESP32
    // Allocate in PSRAM on ESP32-S3 (much larger than internal DRAM)
    pixels = (uint16_t*)heap_caps_malloc(data_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!pixels) {
      // Fall back to internal RAM if PSRAM not available
      pixels = (uint16_t*)heap_caps_malloc(data_size, MALLOC_CAP_8BIT);
    }
  #else
    // Standard malloc on desktop
    pixels = (uint16_t*)malloc(data_size);
  #endif
  
  if (!pixels) return false;

  // Copy data
  #ifdef PLATFORM_ESP32
    memcpy_P(pixels, data, data_size);
  #else
    memcpy(pixels, data, data_size);
  #endif

  return true;
}

bool SpriteAtlas::load_embedded() {
  return load_from_rgb565(atlas_data, ATLAS_DATA_WIDTH, ATLAS_DATA_HEIGHT);
}

// Draw sprite from atlas to framebuffer
void SpriteAtlas::draw(Display_HDC458002C40 *gfx, const Sprite& sprite, int x, int y, bool transparent) {
  if (!pixels) return;

  int fb_w = gfx->get_width();
  int fb_h = gfx->get_height();
  uint16_t *fb = gfx->getFramebuffer();

  // Clip to screen bounds
  int sx = 0, sy = 0;
  int w = sprite.w, h = sprite.h;

  if (x < 0) { sx = -x; w += x; x = 0; }
  if (y < 0) { sy = -y; h += y; y = 0; }
  if (x + w > fb_w) w = fb_w - x;
  if (y + h > fb_h) h = fb_h - y;

  if (w <= 0 || h <= 0) return;

  // Draw sprite directly to framebuffer (cached row offsets for performance)
  int stride = Display_HDC458002C40::SCREEN_BUF_WIDTH;
  for (int dy = 0; dy < h; dy++) {
    uint16_t* row = &fb[(y + dy) * stride + x];  // Cache row pointer
    int atlas_y = sprite.y + sy + dy;

    for (int dx = 0; dx < w; dx++) {
      int atlas_x = sprite.x + sx + dx;

      if (atlas_x >= width || atlas_y >= height) continue;

      uint16_t color = this->get_pixel(atlas_x, atlas_y);

      // Skip transparent pixels if transparent mode
      if (transparent && (color == MAGENTA_RGB565 || color == 0x0000)) continue;

      row[dx] = color;
    }
  }
}

// Draw sprite with color replacement (magenta -> color)
void SpriteAtlas::draw_with_color(Display_HDC458002C40 *gfx, const Sprite& sprite, int x, int y, Color replace_color) {
  if (!pixels) return;

  int fb_w = gfx->get_width();
  int fb_h = gfx->get_height();
  uint16_t *fb = gfx->getFramebuffer();

  int sx = 0, sy = 0;
  int w = sprite.w, h = sprite.h;

  if (x < 0) { sx = -x; w += x; x = 0; }
  if (y < 0) { sy = -y; h += y; y = 0; }
  if (x + w > fb_w) w = fb_w - x;
  if (y + h > fb_h) h = fb_h - y;

  if (w <= 0 || h <= 0) return;

  uint16_t replace_val = replace_color.value;
  int stride = Display_HDC458002C40::SCREEN_BUF_WIDTH;

  for (int dy = 0; dy < h; dy++) {
    uint16_t* row = &fb[(y + dy) * stride + x];  // Cache row pointer
    int atlas_y = sprite.y + sy + dy;

    for (int dx = 0; dx < w; dx++) {
      int atlas_x = sprite.x + sx + dx;

      if (atlas_x >= width || atlas_y >= height) continue;

      uint16_t color = this->get_pixel(atlas_x, atlas_y);

      if (color == 0x0000) continue; // transparent

      if (color == MAGENTA_RGB565)
        color = replace_val;

      row[dx] = color;
    }
  }
}

// Draw sprite with color replacement and scaling (centered at cx, cy)
void SpriteAtlas::draw_with_color_scaled(Display_HDC458002C40 *gfx, const Sprite& sprite, int cx, int cy, Color replace_color, float scale) {
  if (!pixels) return;
  if (scale <= 0.0f) return;

  int fb_w = gfx->get_width();
  int fb_h = gfx->get_height();
  uint16_t *fb = gfx->getFramebuffer();

  // Calculate scaled dimensions
  int scaled_w = (int)(sprite.w * scale);
  int scaled_h = (int)(sprite.h * scale);

  // Calculate top-left position (centered on cx, cy)
  int x = cx - scaled_w / 2;
  int y = cy - scaled_h / 2;

  // Use fixed-point arithmetic for scaling: avoid float division per pixel
  // scale_inv = 65536 / scale (fixed-point Q16.16 reciprocal)
  uint32_t scale_inv = (uint32_t)(65536.0f / scale);
  int stride = Display_HDC458002C40::SCREEN_BUF_WIDTH;
  uint16_t replace_val = replace_color.value;

  // Draw scaled sprite using nearest-neighbor sampling directly to framebuffer
  for (int dy = 0; dy < scaled_h; dy++) {
    int screen_y = y + dy;
    if (screen_y < 0 || screen_y >= fb_h) continue;

    // Map screen y to sprite y using fixed-point: src_y = (dy * scale_inv) >> 16
    int src_y = ((dy * scale_inv) >> 16);
    if (src_y >= sprite.h) src_y = sprite.h - 1;

    uint16_t* row = &fb[screen_y * stride];

    for (int dx = 0; dx < scaled_w; dx++) {
      int screen_x = x + dx;
      if (screen_x < 0 || screen_x >= fb_w) continue;

      // Map screen x to sprite x using fixed-point: src_x = (dx * scale_inv) >> 16
      int src_x = ((dx * scale_inv) >> 16);
      if (src_x >= sprite.w) src_x = sprite.w - 1;

      int atlas_x = sprite.x + src_x;
      int atlas_y = sprite.y + src_y;

      if (atlas_x >= width || atlas_y >= height) continue;

      uint16_t color = this->get_pixel(atlas_x, atlas_y);

      // Skip black (transparent) pixels
      if (color == BLACK_RGB565) continue;

      // Replace magenta with the specified color
      if (color == MAGENTA_RGB565) {
        color = replace_val;
      }

      row[screen_x] = color;
    }
  }
}

// Draw a single symbol from the atlas, characters supported: '-', '.', '%', '°'
void SpriteAtlas::draw_symbol_from_atlas(Display_HDC458002C40 *gfx, int x, int y, unsigned char symbol, Color color) {
  if (!pixels) return;

  int symbol_index = -1;
  if (symbol >= '0' && symbol <= '9') {
    symbol_index = symbol - '0';
  } else if (symbol == '-') {
    symbol_index = 10;  // Minus sign
  } else if (symbol == '.') {
    symbol_index = 11;  // Decimal point
  } else if (symbol == '%') {
    symbol_index = 12;  // Percentage sign
  } else if (symbol == 248) {  // ASCII code for degree symbol '°'
    symbol_index = 13;  // Degree symbol
  }

  if (symbol_index < 0 || symbol_index >= NUM_DIGITS) return;

  const Sprite& symbol_sprite = digit_sprites[symbol_index];

  // Draw the symbol sprite with color replacement
  draw_with_color(gfx, symbol_sprite, x, y, color);
}

void SpriteAtlas::draw_number_from_atlas(Display_HDC458002C40 *gfx, int x, int y, int value, int expected_length, Color color) {
  if (!pixels) return;

  constexpr int padding_between_digits = 2;  // Pixels between digits

  // Convert value to string to extract digits
  char buffer[16];
  snprintf(buffer, sizeof(buffer), "%d", value);
  int len = strlen(buffer);

  if (len <= 0) return;

  if (len < expected_length) {
    // Pad with leading zeros
    int padding = expected_length - len;
    for (int i = expected_length - 1; i >= padding; i--) {
      buffer[i] = buffer[i - padding];
    }
    for (int i = 0; i < padding; i++) {
      buffer[i] = '0';
    }
    buffer[expected_length] = '\0';
    len = expected_length;
  }

  // Draw each digit
  for (int i = 0; i < len && i < NUM_DIGITS; i++) {
    char ch = buffer[i];
    if (ch < '0' || ch > '9') continue;  // Skip non-digit characters

    int digit_index = ch - '0';

    const Sprite& digit_sprite = digit_sprites[digit_index];

    // Draw the digit sprite with color replacement
    draw_with_color(gfx, digit_sprite, x, y, color);

    // Advance x position for next digit
    x += digit_sprite.w + padding_between_digits;
  }
}