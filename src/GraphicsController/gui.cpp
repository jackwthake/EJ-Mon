#include "gui.hpp"
#include "gfx_types.hpp"
#include "sprite.hpp"
#include <cmath>
#include <cfloat>
#include <cstdio>
#include <iostream>

#include <esp_heap_caps.h>

//==============================================================================
// Helper Functions
//==============================================================================

// Helper to interpolate colors
static Color lerp_color(Color c1, Color c2, float t) {
  if (t <= 0.0f) return c1;
  if (t >= 1.0f) return c2;

  // Extract RGB565 components
  uint8_t r1 = (c1.value >> 11) & 0x1F;
  uint8_t g1 = (c1.value >> 5) & 0x3F;
  uint8_t b1 = c1.value & 0x1F;

  uint8_t r2 = (c2.value >> 11) & 0x1F;
  uint8_t g2 = (c2.value >> 5) & 0x3F;
  uint8_t b2 = c2.value & 0x1F;

  uint8_t r = (uint8_t)(r1 + (r2 - r1) * t);
  uint8_t g = (uint8_t)(g1 + (g2 - g1) * t);
  uint8_t b = (uint8_t)(b1 + (b2 - b1) * t);

  return Color((r << 11) | (g << 5) | b);
}

//==============================================================================
// GUI Component Drawing Functions
//==============================================================================


//==============================================================================
// Public GUI Methods
//==============================================================================

void GUI::init(Display_HDC458002C40 *gfx) {
  // Load sprite atlas from embedded data
  if (!atlas.load_embedded()) {
    fprintf(stderr, "Failed to load sprite atlas!\n");
  }

  // Define sprite positions in atlas (from SVG export)
  // Format: {x_pixels, y_pixels, width_pixels, height_pixels}

  // Battery: (0, 0) to (127, 95) = 128x96
  printf("GUI sprites initialized\n");
}

static constexpr Color SHIFT_LIGHT_COLORS[] = {
  Theme::C_YELLOW, Theme::C_WHITE, Theme::C_RED, Theme::C_WHITE
};

void GUI::render(Display_HDC458002C40 *gfx, const EngineData& data, float time_s) {
  // Get framebuffer for sprite drawing
  uint16_t* fb = gfx->getFramebuffer();
  int fb_w = gfx->get_width();
  int fb_h = gfx->get_height();

  Color bg_color(0, 0, 20);

  if (data.rpm >= 4000 && data.rpm < SHIFT_LIGHT_RPM) {
    float t = (float)(data.rpm - 4000) / 3000.0f;

    // Light blue to dark red
    bg_color = lerp_color(Color(0, 0, 20), Color(200, 0, 0), t);

    last_frame_above_shift_rpm = false;
  } else if (data.rpm >= SHIFT_LIGHT_RPM) {
    // Flashing shift light effect
    if (!last_frame_above_shift_rpm) {
      last_frame_above_shift_rpm = true;
    }

    float t = fmodf(time_s * 1.5f, 1.0f);  // 5 Hz flash
    int color_index = (int)(t * (sizeof(SHIFT_LIGHT_COLORS)/sizeof(SHIFT_LIGHT_COLORS[0])));
    bg_color = SHIFT_LIGHT_COLORS[color_index];
  } else {
    last_frame_above_shift_rpm = false;
  }

  gfx->clear(bg_color);

  // Layout centers (for 960x320 screen)
  // constexpr int engine_cx = 500;
  // constexpr int engine_cy = 230;
}
