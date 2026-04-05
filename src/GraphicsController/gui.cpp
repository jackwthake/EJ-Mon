#include "gui.hpp"
#include "gfx_types.hpp"
#include "sprite.hpp"
#include <cmath>
#include <cfloat>
#include <cstdio>
#include <iostream>

#ifdef PLATFORM_ESP32
  #include <esp_heap_caps.h>
#endif

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

//==============================================================================
// Global Variables and Rendering Task
//==============================================================================

// Platform-specific timing helpers
#ifdef PLATFORM_ESP32
  #include <sys/time.h>
  #define get_millis() millis()
#else // PLATFORM_DESKTOP
  #include <chrono>
  #include <thread>
  
  static auto start_time = std::chrono::high_resolution_clock::now();
  
  inline uint32_t millis() {
    auto elapsed = std::chrono::high_resolution_clock::now() - start_time;
    return std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
  }
  
  inline void delay(uint32_t ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
  }
  
  #define get_millis() millis()
#endif

#include "../log/log.hpp"

// Global objects
Display_HDC458002C40 *display = nullptr;
GUI gui;
EngineData engine_data = {
  .rpm = 3500,
  .throttle = 45,
  .coolant_temp = 85,
  .intake_temp = 40,
  .boost_psi = 12.5f,
  .knock_count = 0,
  .timing_adv = 10,
};

void gui_task(void *param) {
  LOG_PRINTLN("Success!");
  
  constexpr uint32_t TARGET_FPS = 60;
  constexpr uint32_t FRAME_TIME_MS = 1000 / TARGET_FPS;  // 16ms per frame
  
  for (;;) {
    uint32_t frame_start = get_millis();
    
    gui.render(display, engine_data, frame_start / 1000.0f);

    static float t = 0.0f;
    t += 0.02f;
    engine_data.rpm = 3000 + (int)(1500 * (sinf(t) + 1.0f) / 2.0f);
    engine_data.boost_psi = 10.0f + 10.0f * (cosf(t * 1.5f) + 1.0f) / 2.0f;
    engine_data.intake_temp = 30 + (int)(40 * (sinf(t * 0.8f) + 1.0f) / 2.0f);
    engine_data.coolant_temp = 70 + (int)(30 * (sinf(t * 0.5f + 1.0f) / 2.0f));

    display->present();
    
    // Frame rate limiting: cap at TARGET_FPS
    uint32_t frame_elapsed = get_millis() - frame_start;
    if (frame_elapsed < FRAME_TIME_MS) {
      delay(FRAME_TIME_MS - frame_elapsed);
    }
  }
}
