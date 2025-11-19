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

  // Draw sprite with color replacement and scaling (magenta -> color)
  // scale: 1.0 = normal size, >1.0 = larger, <1.0 = smaller
  void draw_with_color_scaled(uint16_t* fb, int fb_w, int fb_h,
                               const Sprite& sprite, int cx, int cy,
                               Color replace_color, float scale);

  // Draw sprite rotated around center point
  void draw_rotated(uint16_t* fb, int fb_w, int fb_h,
                    const Sprite& sprite, int cx, int cy,
                    float angle_rad, bool transparent = true);

  // Draw sprite with vertical fill level (0.0 = empty, 1.0 = full)
  // Fills from bottom to top
  void draw_with_fill(uint16_t* fb, int fb_w, int fb_h,
                      const Sprite& sprite, int x, int y,
                      float fill_level, Color fill_color);

  // Draw sprite with multiple color replacements (magenta + green)
  void draw_with_colors(uint16_t* fb, int fb_w, int fb_h,
                        const Sprite& sprite, int x, int y,
                        Color magenta_replacement, Color green_replacement);

  // Draw sprite with sequential green shades (for timing marks)
  // green_shades[i] = color to use for green shade i (RGB 0, 200+i*step, 0)
  // active_index = which shade should be bright green, others become dark gray
  void draw_with_green_sequence(uint16_t* fb, int fb_w, int fb_h,
                                 const Sprite& sprite, int x, int y,
                                 Color magenta_replacement, Color active_tick,
                                 int active_index, int num_shades);

  
  void draw_symbol_from_atlas(uint16_t* fb, int fb_w, int fb_h,
                          int x, int y, unsigned char symbol, Color color);
  // Draw a single digit from the atlas
  void draw_number_from_atlas(uint16_t* fb, int fb_w, int fb_h,
                          int x, int y, int value, int expected_length, Color color);

  // Get atlas dimensions
  int get_width() const { return width; }
  int get_height() const { return height; }

private:
  uint16_t* pixels;  // RGB565 pixel data
  int width;
  int height;

  // Digit sprites for numbers 0-9 and symbols
  static constexpr int DIGIT_WIDTH = 48;
  static constexpr int DIGIT_HEIGHT = 80;
  static constexpr int NUM_DIGITS = 14;  // 0-9, minus, minus decimal point and percentage
  Sprite digit_sprites[NUM_DIGITS] = {
    {0, 432, DIGIT_WIDTH, DIGIT_HEIGHT},     // '0'
    {DIGIT_WIDTH * 1, 432, DIGIT_WIDTH, DIGIT_HEIGHT},    // '1'
    {DIGIT_WIDTH * 2, 432, DIGIT_WIDTH, DIGIT_HEIGHT},   // '2'
    {DIGIT_WIDTH * 3, 432, DIGIT_WIDTH, DIGIT_HEIGHT},   // '3'
    {DIGIT_WIDTH * 4, 432, DIGIT_WIDTH, DIGIT_HEIGHT},   // '4'
    {DIGIT_WIDTH * 5, 432, DIGIT_WIDTH, DIGIT_HEIGHT},   // '5'
    {DIGIT_WIDTH * 6, 432, DIGIT_WIDTH, DIGIT_HEIGHT},   // '6'
    {DIGIT_WIDTH * 7, 432, DIGIT_WIDTH, DIGIT_HEIGHT},   // '7'
    {DIGIT_WIDTH * 8, 432, DIGIT_WIDTH, DIGIT_HEIGHT},   // '8'
    {DIGIT_WIDTH * 9, 432, DIGIT_WIDTH, DIGIT_HEIGHT},   // '9'
    {DIGIT_WIDTH * 10, 432, DIGIT_WIDTH, DIGIT_HEIGHT},   // '-' minus sign
    {DIGIT_WIDTH * 11, 432, DIGIT_WIDTH, DIGIT_HEIGHT},  // '.' decimal point
    {DIGIT_WIDTH * 12, 432, DIGIT_WIDTH, DIGIT_HEIGHT},  // '%' percentage sign
    {DIGIT_WIDTH * 13, 432, DIGIT_WIDTH, DIGIT_HEIGHT}   // ° degree sign
  };

  // Magenta color used for masking (RGB 255, 0, 255)
  // RGB565 conversion: R=255>>3=31, G=0>>2=0, B=255>>3=31
  static constexpr uint16_t MAGENTA_RGB565 = (31 << 11) | (0 << 5) | 31;  // 0xF81F
  static constexpr uint16_t BLACK_RGB565 = 0x0000;  // Black for transparency
  // Green color for timing marks (RGB 0, 255, 0)
  static constexpr uint16_t GREEN_RGB565 = (0 << 11) | (63 << 5) | 0;  // 0x07E0
};
