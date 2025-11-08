#pragma once

#include "gfx_types.hpp"
#include <cstdint>
#include <cstddef>

// Sprite structure for atlas-based sprites
struct Sprite {
  int16_t x;        // X position in atlas
  int16_t y;        // Y position in atlas
  int16_t w;        // Width in pixels
  int16_t h;        // Height in pixels
};

// Sprite atlas and drawing functions
class SpriteAtlas {
public:
  SpriteAtlas();
  ~SpriteAtlas();

  // Load atlas from BMP file (desktop) or embedded data (ESP32)
  bool load_from_file(const char* filename);
  bool load_from_memory(const uint8_t* data, size_t size);

  // Draw sprite from atlas to framebuffer
  void draw(uint16_t* fb, int fb_w, int fb_h,
            const Sprite& sprite, int x, int y,
            bool transparent = true);

  // Draw sprite with color replacement (magenta -> color)
  void draw_with_color(uint16_t* fb, int fb_w, int fb_h,
                       const Sprite& sprite, int x, int y,
                       Color replace_color);

  // Draw sprite rotated (for turbo animation)
  void draw_rotated(uint16_t* fb, int fb_w, int fb_h,
                    const Sprite& sprite, int cx, int cy,
                    float angle_rad, bool transparent = true);

  // Get atlas dimensions
  int get_width() const { return width; }
  int get_height() const { return height; }

private:
  uint16_t* pixels;  // RGB565 pixel data
  int width;
  int height;

  // Magenta color used for masking (Aseprite magenta: RGB 255, 0, 200)
  // RGB565 conversion: R=255>>3=31, G=0>>2=0, B=200>>3=25
  static constexpr uint16_t MAGENTA_RGB565 = (31 << 11) | (0 << 5) | 25;  // 0xF819
  static constexpr uint16_t BLACK_RGB565 = 0x0000;  // Black for transparency
};
