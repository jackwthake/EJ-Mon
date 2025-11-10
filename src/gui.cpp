#include "gui.hpp"
#include "graphics/gfx_types.hpp"
#include "graphics/sprite.hpp"
#include <cmath>
#include <cstdio>
#include <iostream>


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

// Draw EJ engine sprite with coolant temperature color and firing order animation
void GUI::draw_ej_engine(Graphics* gfx, int cx, int cy, const EngineData& data, float time_s) {
  // Color based on coolant temp: blue (cold) -> green -> yellow -> red (hot)
  // Typical range: 0°C to 120°C
  float temp_normalized = data.coolant_temp / 120.0f;
  if (temp_normalized < 0.0f) temp_normalized = 0.0f;
  if (temp_normalized > 1.0f) temp_normalized = 1.0f;

  Color coolant_color = Theme::BLUE;
  if (temp_normalized < 0.33f) {
    coolant_color = lerp_color(Theme::BLUE, Theme::GREEN, temp_normalized * 3.0f);
  } else if (temp_normalized < 0.66f) {
    coolant_color = lerp_color(Theme::GREEN, Theme::YELLOW, (temp_normalized - 0.33f) * 3.0f);
  } else {
    coolant_color = lerp_color(Theme::YELLOW, Theme::RED, (temp_normalized - 0.66f) * 3.0f);
  }

  // Calculate which timing mark should be active based on RPM
  // 30 timing marks going clockwise: RGB(0,100,0) to RGB(0,216,0) in steps of 4
  const int NUM_TIMING_MARKS = 30;

  // Calculate increment speed based on RPM
  // At 6000 RPM, we want smooth but visible motion
  // Speed factor determines how many marks to advance per frame at max RPM
  float rpm_normalized = data.rpm / 6000.0f;  // Normalize to 0-1 range
  float speed_factor = rpm_normalized * 1.1f;  // Adjust this to tune speed (1.1 = ~6 marks/sec at 6000 RPM @60fps)

  // Accumulate fractional increments
  timing_mark_accumulator += speed_factor;

  // When accumulator >= 1.0, increment the active mark
  if (timing_mark_accumulator >= 1.0f) {
    int steps = (int)timing_mark_accumulator;
    active_timing_mark = (active_timing_mark + steps) % NUM_TIMING_MARKS;
    timing_mark_accumulator -= steps;
  }

  // Draw engine sprite with sequential green shades
  uint16_t* fb = gfx->get_framebuffer();
  int fb_w = gfx->get_width();
  int fb_h = gfx->get_height();

  atlas.draw_with_green_sequence(fb, fb_w, fb_h, spr_motor_block,
                                  cx - spr_motor_block.w/2, cy - spr_motor_block.h/2,
                                  coolant_color, active_timing_mark, NUM_TIMING_MARKS);
}

// Draw turbo sprite with boost-based color and scaling
void GUI::draw_turbo(Graphics* gfx, int cx, int cy, const EngineData& data, float /*time_s*/) {
  // Color interpolation: green (low boost) -> yellow (moderate boost) -> red (high boost)
  // Boost range: -15 to +20 PSI
  float boost_normalized = (data.boost_psi + 15.0f) / 35.0f;
  if (boost_normalized < 0.0f) boost_normalized = 0.0f;
  if (boost_normalized > 1.0f) boost_normalized = 1.0f;

  Color turbo_color = Theme::GREEN;
  if (boost_normalized < 0.5f) {
    // Green -> Yellow (0% to 50%)
    turbo_color = lerp_color(Theme::BLUE, Theme::WHITE, boost_normalized * 2.0f);
  } else {
    // Yellow -> Red (50% to 100%)
    turbo_color = lerp_color(Theme::WHITE, Theme::RED, (boost_normalized - 0.5f) * 2.0f);
  }

  // Scale based on boost with ease-in (quadratic)
  // Scale from 1.0 (no boost) to 1.5 (max boost)
  float scale_t = boost_normalized * boost_normalized;  // Ease-in quadratic
  float scale = 1.0f + (scale_t * 0.5f);  // 1.0 to 1.5

  // Draw turbo sprite with color replacement and scaling
  uint16_t* fb = gfx->get_framebuffer();
  int fb_w = gfx->get_width();
  int fb_h = gfx->get_height();

  atlas.draw_with_color_scaled(fb, fb_w, fb_h, spr_turbo_housing,
                                cx, cy, turbo_color, scale);
}

// Draw intercooler with temperature-based color
void GUI::draw_intercooler(Graphics* gfx, int cx, int cy, const EngineData& data) {
  // Color based on intake temp: blue (cold) -> green -> yellow -> red (hot)
  // Typical range: -10°C to 80°C
  float temp_normalized = (data.intake_temp + 10.0f) / 90.0f;
  if (temp_normalized < 0.0f) temp_normalized = 0.0f;
  if (temp_normalized > 1.0f) temp_normalized = 1.0f;

  Color ic_color = Theme::CYAN;
  if (temp_normalized < 0.33f) {
    ic_color = lerp_color(Theme::CYAN, Theme::GREEN, temp_normalized * 3.0f);
  } else if (temp_normalized < 0.66f) {
    ic_color = lerp_color(Theme::GREEN, Theme::YELLOW, (temp_normalized - 0.33f) * 3.0f);
  } else {
    ic_color = lerp_color(Theme::YELLOW, Theme::RED, (temp_normalized - 0.66f) * 3.0f);
  }

  // Draw intercooler sprite with color replacement for magenta areas
  uint16_t* fb = gfx->get_framebuffer();
  int fb_w = gfx->get_width();
  int fb_h = gfx->get_height();

  atlas.draw_with_color(fb, fb_w, fb_h, spr_intercooler,
                        cx - spr_intercooler.w/2, cy - spr_intercooler.h/2,
                        ic_color);
}

// Draw 4 cam gears rotating around the engine
void GUI::draw_cam_gears(Graphics* gfx, int engine_cx, int engine_cy, const EngineData& data, float time_s) {
  // Camshafts rotate at half crankshaft speed (2:1 ratio)
  // Calculate rotation angle based on RPM and time
  float cam_rpm = data.rpm / 2.0f;
  float revolutions_per_second = cam_rpm / 60.0f;
  const float TWO_PI = 2.0f * 3.14159f;
  float angle = revolutions_per_second * time_s * TWO_PI;

  // Normalize angle to prevent floating point precision issues
  // Keep angle within 0 to 2π range
  angle = fmodf(angle, TWO_PI);
  if (angle < 0) angle += TWO_PI;

  // Get framebuffer
  uint16_t* fb = gfx->get_framebuffer();
  int fb_w = gfx->get_width();
  int fb_h = gfx->get_height();

  // Calculate engine sprite's top-left corner
  // engine_cx and engine_cy are the center of the engine (384x192 sprite)
  int engine_left = engine_cx - spr_motor_block.w / 2;
  int engine_top = engine_cy - spr_motor_block.h / 2;

  // Cam gear positions relative to engine sprite (centers)
  // Positions given: (43, 39), (348, 49), (42, 134), (348, 134)
  int gear1_x = engine_left + 40;
  int gear1_y = engine_top + 39;

  int gear2_x = engine_left + 345;
  int gear2_y = engine_top + 39;

  int gear3_x = engine_left + 40;
  int gear3_y = engine_top + 135;

  int gear4_x = engine_left + 345;
  int gear4_y = engine_top + 134;

  // Draw all 4 cam gears rotating at the same speed
  atlas.draw_rotated(fb, fb_w, fb_h, spr_cam_gear, gear1_x, gear1_y, angle, true);
  atlas.draw_rotated(fb, fb_w, fb_h, spr_cam_gear, gear2_x, gear2_y, angle, true);
  atlas.draw_rotated(fb, fb_w, fb_h, spr_cam_gear, gear3_x, gear3_y, angle, true);
  atlas.draw_rotated(fb, fb_w, fb_h, spr_cam_gear, gear4_x, gear4_y, angle, true);
}

// Draw battery with vertical fill level
void GUI::draw_battery(Graphics* gfx, int cx, int cy, const EngineData& data) {
  // Map battery voltage (12V-14V) to fill level (0.0-1.0)
  float battery_level = (data.battery_voltage - 12.0f) / 2.0f;  // 12V=0.0, 14V=1.0
  if (battery_level < 0.0f) battery_level = 0.0f;
  if (battery_level > 1.0f) battery_level = 1.0f;

  // Color based on level: red (low) -> yellow (medium) -> green (full)
  Color bat_color = Theme::GREEN;
  if (battery_level < 0.33f) {
    bat_color = lerp_color(Theme::RED, Theme::YELLOW, battery_level * 3.0f);
  } else if (battery_level < 0.66f) {
    bat_color = lerp_color(Theme::YELLOW, Theme::GREEN, (battery_level - 0.33f) * 3.0f);
  } else {
    bat_color = Theme::GREEN;
  }

  // Draw battery sprite with vertical fill level
  uint16_t* fb = gfx->get_framebuffer();
  int fb_w = gfx->get_width();
  int fb_h = gfx->get_height();

  atlas.draw_with_fill(fb, fb_w, fb_h, spr_battery,
                       cx - spr_battery.w/2, cy - spr_battery.h/2,
                       battery_level, bat_color);
}

// Draw Assetto Corsa style top RPM bar (fills from both sides)
void GUI::draw_top_rpm_bar(Graphics* gfx, const EngineData& data) {
  constexpr int bar_y = 0;
  constexpr int bar_h = 15;
  constexpr int screen_w = 960;
  constexpr int center_x = screen_w / 2;

  // RPM range: 0-6500 RPM
  constexpr int max_rpm = 6500;
  float rpm_normalized = (float)data.rpm / (float)max_rpm;
  if (rpm_normalized > 1.0f) rpm_normalized = 1.0f;

  // Each side fills half the screen width
  int half_fill = (int)(rpm_normalized * center_x);

  // Color transitions: green -> yellow -> red as RPM increases
  Color bar_color = Theme::WHITE;

  if (rpm_normalized <= 0.7f) {
    // From 0.0 → 0.7, fade from white → yellow
    float t = rpm_normalized / 0.7f;
    bar_color = lerp_color(Theme::WHITE, Theme::YELLOW, t);
  } else {
    // From 0.7 → 1.0, fade from yellow → red
    float t = (rpm_normalized - 0.7f) / (1.0f - 0.7f);
    bar_color = lerp_color(Theme::YELLOW, Theme::RED, t);
  }

  // Draw left side (fills from left edge toward center)
  gfx->fill_rect(0, bar_y, half_fill, bar_h, bar_color);

  // Draw right side (fills from right edge toward center)
  gfx->fill_rect(screen_w - half_fill, bar_y, half_fill, bar_h, bar_color);
}

//==============================================================================
// Public GUI Methods
//==============================================================================

void GUI::init() {
  // Load sprite atlas
  if (!atlas.load_from_file("res/atlas.bmp")) {
    fprintf(stderr, "Failed to load sprite atlas!\n");
  }

  // Load background
  if (!background.load_from_file("res/background.bmp")) {
    fprintf(stderr, "Failed to load background!\n");
  }

  // Define sprite positions in atlas (from SVG export)
  // Format: {x_pixels, y_pixels, width_pixels, height_pixels}

  // Battery: (0, 0) to (127, 95) = 128x96
  spr_battery = {0, 0, 128, 96};

  // Turbo: (128, 0) to (255, 127) = 128x128
  spr_turbo_housing = {128, 0, 127, 128};

  // Intercooler: (0, 128) to (319, 223) = 320x96
  spr_intercooler = {0, 128, 320, 96};

  // Engine: (0, 224) to (383, 415) = 384x192
  spr_motor_block = {0, 224, 384, 192};

  // Cam gear: (256, 0) to (319, 63) = 64x64
  spr_cam_gear = {256, 0, 64, 64};

  printf("GUI sprites initialized\n");
}

static constexpr Color SHIFT_LIGHT_COLORS[] = { 
  Theme::YELLOW, Theme::WHITE, Theme::RED, Theme::WHITE
};

void GUI::render(Graphics* gfx, const EngineData& data, float time_s) {
  // Get framebuffer for sprite drawing
  uint16_t* fb = gfx->get_framebuffer();
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
  constexpr int engine_cx = 480;
  constexpr int engine_cy = 230;
  constexpr int turbo_cx = 140;
  constexpr int turbo_cy = 220;
  constexpr int ic_cx = 480;
  constexpr int ic_cy = 75;
  constexpr int battery_cx = 810;
  constexpr int battery_cy = 250;

  // Draw top RPM bar (Assetto Corsa style)
  draw_top_rpm_bar(gfx, data);

  // Draw all components
  draw_ej_engine(gfx, engine_cx, engine_cy, data, time_s);
  draw_cam_gears(gfx, engine_cx, engine_cy, data, time_s);
  draw_turbo(gfx, turbo_cx, turbo_cy, data, time_s);
  draw_intercooler(gfx, ic_cx, ic_cy, data);
  draw_battery(gfx, battery_cx, battery_cy, data);

  // Warning indicators (moved down below RPM bar)
  if (data.knock_detected) {
    gfx->fill_rect(45, 30, 180, 50, Theme::YELLOW);
    gfx->draw_text("!KNOCK!", 70, 40, 5, Theme::BLACK);
  }

  if (data.overboost) {
    gfx->fill_rect(200, 30, 280, 50, Theme::MAGENTA);
    gfx->draw_text("!OVERBOOST!", 210, 35, 5, Theme::BLACK);
  }
}
