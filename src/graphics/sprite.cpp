#include "sprite.hpp"
#include <cstring>
#include <cstdio>
#include <cmath>
#include <set>

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

  printf("Loading sprite atlas: %s (%dx%d, %dbpp)\n", filename, width, height, bpp);

  // Debug: Sample some pixels to see actual magenta value
  bool found_sample = false;

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

      // Debug: Sample magenta-like pixels
      if (!found_sample && r > 200 && b > 200 && g < 50) {
        printf("Magenta-like pixel at (%d,%d): RGB(%d,%d,%d) -> RGB565=0x%04X\n",
               x, y, r, g, b, pixels[y * width + x]);
        found_sample = true;
      }
    }
  }

  delete[] row_buffer;
  fclose(f);

  // Debug: Sample colors in turbo housing area (tiles 0,5 to 5,12 = pixels 0,80 to 80,192)
  printf("Sampling turbo housing area (0,80 to 80,192):\n");
  std::set<uint16_t> unique_colors;
  for (int y = 80; y < 192 && y < height; y++) {
    for (int x = 0; x < 80 && x < width; x++) {
      unique_colors.insert(pixels[y * width + x]);
    }
  }
  printf("Found %zu unique colors in turbo housing:\n", unique_colors.size());
  int count = 0;
  for (uint16_t c : unique_colors) {
    // Convert back to RGB for display
    uint8_t r = ((c >> 11) & 0x1F) << 3;
    uint8_t g = ((c >> 5) & 0x3F) << 2;
    uint8_t b = (c & 0x1F) << 3;
    printf("  0x%04X = RGB(%d,%d,%d)\n", c, r, g, b);
    if (++count > 10) {
      printf("  ... (%zu more)\n", unique_colors.size() - count);
      break;
    }
  }

  printf("Sprite atlas loaded successfully\n");
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

  static bool debug_printed = false;

  // Draw sprite
  for (int dy = 0; dy < h; dy++) {
    for (int dx = 0; dx < w; dx++) {
      int atlas_x = sprite.x + sx + dx;
      int atlas_y = sprite.y + sy + dy;

      if (atlas_x >= width || atlas_y >= height) continue;

      uint16_t color = pixels[atlas_y * width + atlas_x];

      // Skip black (transparent) pixels
      if (color == 0x0000) continue;

      // Debug: print first few magenta pixels found
      if (!debug_printed && color == MAGENTA_RGB565) {
        printf("Found magenta pixel at atlas (%d,%d), RGB565=0x%04X\n", atlas_x, atlas_y, color);
        debug_printed = true;
      }

      // Replace magenta with the specified color
      if (color == MAGENTA_RGB565) {
        color = replace_color.value;
      }

      fb[(y + dy) * fb_w + (x + dx)] = color;
    }
  }
}

// Draw sprite rotated around center point (for turbo animation)
void SpriteAtlas::draw_rotated(uint16_t* fb, int fb_w, int fb_h,
                               const Sprite& sprite, int cx, int cy,
                               float angle_rad, bool transparent) {
  if (!pixels) return;

  float cos_a = cosf(angle_rad);
  float sin_a = sinf(angle_rad);

  int half_w = sprite.w / 2;
  int half_h = sprite.h / 2;

  // Bounding box for rotated sprite
  int max_dim = (int)(sqrtf(sprite.w * sprite.w + sprite.h * sprite.h) / 2.0f) + 1;

  for (int dy = -max_dim; dy <= max_dim; dy++) {
    for (int dx = -max_dim; dx <= max_dim; dx++) {
      // Rotate point back to sprite space
      int src_x = (int)(dx * cos_a + dy * sin_a) + half_w;
      int src_y = (int)(-dx * sin_a + dy * cos_a) + half_h;

      // Check if source point is within sprite bounds
      if (src_x < 0 || src_x >= sprite.w || src_y < 0 || src_y >= sprite.h) continue;

      // Calculate screen position
      int screen_x = cx + dx;
      int screen_y = cy + dy;

      // Check if screen position is valid
      if (screen_x < 0 || screen_x >= fb_w || screen_y < 0 || screen_y >= fb_h) continue;

      // Get pixel from atlas
      int atlas_x = sprite.x + src_x;
      int atlas_y = sprite.y + src_y;

      if (atlas_x >= width || atlas_y >= height) continue;

      uint16_t color = pixels[atlas_y * width + atlas_x];

      // Skip magenta pixels if transparent mode
      if (transparent && color == BLACK_RGB565) continue;

      fb[screen_y * fb_w + screen_x] = color;
    }
  }
}
