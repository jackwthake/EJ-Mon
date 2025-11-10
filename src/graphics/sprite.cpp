#include "sprite.hpp"
#include <cstring>
#include <cstdio>
#include <cmath>

#ifdef PLATFORM_DESKTOP
#include <cstdlib>
#endif

SpriteAtlas::SpriteAtlas() : pixels(nullptr), width(0), height(0) {
}

SpriteAtlas::~SpriteAtlas() {
  if (pixels) {
    delete[] pixels;
    pixels = nullptr;
  }
}

// Simple BMP loader (supports 24-bit and 32-bit BMPs)
bool SpriteAtlas::load_from_file(const char* filename) {
#ifdef PLATFORM_DESKTOP
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
#else
  // ESP32: Load from embedded data (implement later)
  return false;
#endif
}

bool SpriteAtlas::load_from_memory(const uint8_t* data, size_t size) {
  // TODO: Implement for ESP32 embedded sprites
  (void)data;
  (void)size;
  return false;
}

// Draw sprite from atlas to framebuffer
void SpriteAtlas::draw(uint16_t* fb, int fb_w, int fb_h,
                       const Sprite& sprite, int x, int y,
                       bool transparent) {
  if (!pixels) return;

  // Clip to screen bounds
  int sx = 0, sy = 0;
  int w = sprite.w, h = sprite.h;

  if (x < 0) { sx = -x; w += x; x = 0; }
  if (y < 0) { sy = -y; h += y; y = 0; }
  if (x + w > fb_w) w = fb_w - x;
  if (y + h > fb_h) h = fb_h - y;

  if (w <= 0 || h <= 0) return;

  // Draw sprite
  for (int dy = 0; dy < h; dy++) {
    for (int dx = 0; dx < w; dx++) {
      int atlas_x = sprite.x + sx + dx;
      int atlas_y = sprite.y + sy + dy;

      if (atlas_x >= width || atlas_y >= height) continue;

      uint16_t color = pixels[atlas_y * width + atlas_x];

      // Skip transparent pixels if transparent mode
      if (transparent && (color == MAGENTA_RGB565 || color == 0x0000)) continue;

      fb[(y + dy) * fb_w + (x + dx)] = color;
    }
  }
}

// Draw sprite with color replacement (magenta -> color)
void SpriteAtlas::draw_with_color(uint16_t* fb, int fb_w, int fb_h,
                                  const Sprite& sprite, int x, int y,
                                  Color replace_color) {
  if (!pixels) return;

  // Clip to screen bounds
  int sx = 0, sy = 0;
  int w = sprite.w, h = sprite.h;

  if (x < 0) { sx = -x; w += x; x = 0; }
  if (y < 0) { sy = -y; h += y; y = 0; }
  if (x + w > fb_w) w = fb_w - x;
  if (y + h > fb_h) h = fb_h - y;

  if (w <= 0 || h <= 0) return;

  // Draw sprite
  for (int dy = 0; dy < h; dy++) {
    for (int dx = 0; dx < w; dx++) {
      int atlas_x = sprite.x + sx + dx;
      int atlas_y = sprite.y + sy + dy;

      if (atlas_x >= width || atlas_y >= height) continue;

      uint16_t color = pixels[atlas_y * width + atlas_x];

      // Skip black (transparent) pixels
      if (color == 0x0000) continue;

      // Replace magenta with the specified color
      if (color == MAGENTA_RGB565) {
        color = replace_color.value;
      }

      fb[(y + dy) * fb_w + (x + dx)] = color;
    }
  }
}

// Draw sprite with color replacement and scaling (centered at cx, cy)
void SpriteAtlas::draw_with_color_scaled(uint16_t* fb, int fb_w, int fb_h,
                                          const Sprite& sprite, int cx, int cy,
                                          Color replace_color, float scale) {
  if (!pixels) return;
  if (scale <= 0.0f) return;

  // Calculate scaled dimensions
  int scaled_w = (int)(sprite.w * scale);
  int scaled_h = (int)(sprite.h * scale);

  // Calculate top-left position (centered on cx, cy)
  int x = cx - scaled_w / 2;
  int y = cy - scaled_h / 2;

  // Draw scaled sprite using nearest-neighbor sampling
  for (int dy = 0; dy < scaled_h; dy++) {
    int screen_y = y + dy;
    if (screen_y < 0 || screen_y >= fb_h) continue;

    // Map screen y to sprite y (inverse scale)
    int src_y = (int)((float)dy / scale);
    if (src_y >= sprite.h) src_y = sprite.h - 1;

    for (int dx = 0; dx < scaled_w; dx++) {
      int screen_x = x + dx;
      if (screen_x < 0 || screen_x >= fb_w) continue;

      // Map screen x to sprite x (inverse scale)
      int src_x = (int)((float)dx / scale);
      if (src_x >= sprite.w) src_x = sprite.w - 1;

      int atlas_x = sprite.x + src_x;
      int atlas_y = sprite.y + src_y;

      if (atlas_x >= width || atlas_y >= height) continue;

      uint16_t color = pixels[atlas_y * width + atlas_x];

      // Skip black (transparent) pixels
      if (color == BLACK_RGB565) continue;

      // Replace magenta with the specified color
      if (color == MAGENTA_RGB565) {
        color = replace_color.value;
      }

      fb[screen_y * fb_w + screen_x] = color;
    }
  }
}

// Draw sprite rotated around center point
void SpriteAtlas::draw_rotated(uint16_t* fb, int fb_w, int fb_h,
                               const Sprite& sprite, int cx, int cy,
                               float angle_rad, bool transparent) {
  if (!pixels) return;

  float cos_a = cosf(angle_rad);
  float sin_a = sinf(angle_rad);

  float half_w = sprite.w / 2.0f;
  float half_h = sprite.h / 2.0f;

  // Bounding box for rotated sprite
  float max_dim = sqrtf(sprite.w * sprite.w + sprite.h * sprite.h) / 2.0f + 1;

  for (float dy = -max_dim; dy <= max_dim; dy++) {
    for (float dx = -max_dim; dx <= max_dim; dx++) {
      // Rotate point back to sprite space
      float src_x = (dx * cos_a + dy * sin_a) + half_w;
      float src_y = (-dx * sin_a + dy * cos_a) + half_h;

      // Check if source point is within sprite bounds
      if (src_x < 0 || src_x >= sprite.w || src_y < 0 || src_y >= sprite.h) continue;

      // Calculate screen position
      float screen_x = (float)cx + dx;
      float screen_y = (float)cy + dy;

      // Check if screen position is valid
      if (screen_x < 0 || screen_x >= fb_w || screen_y < 0 || screen_y >= fb_h) continue;

      // Get pixel from atlas
      int atlas_x = sprite.x + src_x;
      int atlas_y = sprite.y + src_y;

      if (atlas_x >= width || atlas_y >= height) continue;

      uint16_t color = pixels[atlas_y * width + atlas_x];

      // Skip transparent pixels
      if (transparent && color == BLACK_RGB565) continue;

      fb[(int)screen_y * fb_w + (int)screen_x] = color;
    }
  }
}

// Draw sprite with multiple color replacements (magenta + green)
void SpriteAtlas::draw_with_colors(uint16_t* fb, int fb_w, int fb_h,
                                    const Sprite& sprite, int x, int y,
                                    Color magenta_replacement, Color green_replacement) {
  if (!pixels) return;

  // Clip to screen bounds
  int sx = 0, sy = 0;
  int w = sprite.w, h = sprite.h;

  if (x < 0) { sx = -x; w += x; x = 0; }
  if (y < 0) { sy = -y; h += y; y = 0; }
  if (x + w > fb_w) w = fb_w - x;
  if (y + h > fb_h) h = fb_h - y;

  if (w <= 0 || h <= 0) return;

  // Draw sprite
  for (int dy = 0; dy < h; dy++) {
    for (int dx = 0; dx < w; dx++) {
      int atlas_x = sprite.x + sx + dx;
      int atlas_y = sprite.y + sy + dy;

      if (atlas_x >= width || atlas_y >= height) continue;

      uint16_t color = pixels[atlas_y * width + atlas_x];

      // Skip black (transparent) pixels
      if (color == BLACK_RGB565) continue;

      // Replace magenta with the specified color
      if (color == MAGENTA_RGB565) {
        color = magenta_replacement.value;
      }
      // Replace green with the specified color
      else if (color == GREEN_RGB565) {
        color = green_replacement.value;
      }

      fb[(y + dy) * fb_w + (x + dx)] = color;
    }
  }
}

// Draw sprite with sequential green shades for timing marks
void SpriteAtlas::draw_with_green_sequence(uint16_t* fb, int fb_w, int fb_h,
                                            const Sprite& sprite, int x, int y,
                                            Color magenta_replacement,
                                            int active_index, int num_shades) {
  if (!pixels) return;

  // Clip to screen bounds
  int sx = 0, sy = 0;
  int w = sprite.w, h = sprite.h;

  if (x < 0) { sx = -x; w += x; x = 0; }
  if (y < 0) { sy = -y; h += y; y = 0; }
  if (x + w > fb_w) w = fb_w - x;
  if (y + h > fb_h) h = fb_h - y;

  if (w <= 0 || h <= 0) return;

  // Calculate RGB565 values for green shades
  // Green ranges from RGB(0, 100, 0) to RGB(0, 216, 0) in steps of 4
  // For 30 shades: 100, 104, 108, ..., 216 (step of 4)
  // Step of 4 ensures each shade maps to unique RGB565 value (6-bit green = 64 levels)
  const int GREEN_START = 100;
  const int GREEN_STEP = 4;

  // Pre-calculate RGB565 values for each shade
  uint16_t green_rgb565[32];  // Support up to 32 shades
  for (int i = 0; i < num_shades && i < 32; i++) {
    int g = GREEN_START + i * GREEN_STEP;  // 100, 104, 108, ..., 216
    uint16_t g6 = (g >> 2) & 0x3F;
    green_rgb565[i] = (0 << 11) | (g6 << 5) | 0;
  }

  // Light gray for inactive marks
  const uint16_t LIGHT_GRAY_RGB565 = ((200 >> 3) << 11) | ((200 >> 2) << 5) | (200 >> 3);
  // Dimmer green for trailing mark
  const uint16_t DIM_GREEN_RGB565 = (0 << 11) | (40 << 5) | 0;  // RGB(0, 160, 0) approx

  // Calculate previous index (the one behind the active mark)
  int prev_index = (active_index - 1 + num_shades) % num_shades;

  // Draw sprite
  for (int dy = 0; dy < h; dy++) {
    for (int dx = 0; dx < w; dx++) {
      int atlas_x = sprite.x + sx + dx;
      int atlas_y = sprite.y + sy + dy;

      if (atlas_x >= width || atlas_y >= height) continue;

      uint16_t color = pixels[atlas_y * width + atlas_x];

      // Skip black (transparent) pixels
      if (color == BLACK_RGB565) continue;

      // Replace magenta with the specified color
      if (color == MAGENTA_RGB565) {
        color = magenta_replacement.value;
      }
      // Check if this is a green shade and replace accordingly
      else {
        bool is_green_shade = false;
        bool is_active_mark = false;

        for (int i = 0; i < num_shades && i < 32; i++) {
          if (color == green_rgb565[i]) {
            is_green_shade = true;
            if (i == active_index) {
              color = GREEN_RGB565;  // Bright green for active
              is_active_mark = true;
            } else if (i == prev_index) {
              color = DIM_GREEN_RGB565;  // Dim green for previous mark
              is_active_mark = true;
            }
            
            break;  // Found the shade, exit loop
          }
        }

        // Skip drawing non-active timing marks
        if (is_green_shade && !is_active_mark) {
          continue;
        }
      }

      fb[(y + dy) * fb_w + (x + dx)] = color;
    }
  }
}

// Draw sprite with vertical fill level (fills from bottom to top)
void SpriteAtlas::draw_with_fill(uint16_t* fb, int fb_w, int fb_h,
                                  const Sprite& sprite, int x, int y,
                                  float fill_level, Color fill_color) {
  if (!pixels) return;

  // Clamp fill level
  if (fill_level < 0.0f) fill_level = 0.0f;
  if (fill_level > 1.0f) fill_level = 1.0f;

  // Calculate fill height (from bottom)
  int fill_height = (int)(sprite.h * fill_level);
  int fill_start_y = sprite.h - fill_height;  // Y offset where fill begins

  for (int dy = 0; dy < sprite.h; dy++) {
    int screen_y = y + dy;
    if (screen_y < 0 || screen_y >= fb_h) continue;

    for (int dx = 0; dx < sprite.w; dx++) {
      int screen_x = x + dx;
      if (screen_x < 0 || screen_x >= fb_w) continue;

      int atlas_x = sprite.x + dx;
      int atlas_y = sprite.y + dy;

      if (atlas_x >= width || atlas_y >= height) continue;

      uint16_t color = pixels[atlas_y * width + atlas_x];

      // Skip black (transparent) pixels
      if (color == BLACK_RGB565) continue;

      // If this is in the filled region and the pixel is magenta, use fill color
      if (dy >= fill_start_y && color == MAGENTA_RGB565) {
        color = fill_color.value;
      }
      // If this is above the fill line and magenta, skip it (empty part)
      else if (dy < fill_start_y && color == MAGENTA_RGB565) {
        continue;
      }

      fb[screen_y * fb_w + screen_x] = color;
    }
  }
}
