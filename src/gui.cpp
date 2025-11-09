#include "gui.hpp"
#include "graphics/gfx_types.hpp"
#include "graphics/sprite.hpp"
#include <cmath>
#include <cstdio>


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

// Draw EJ engine sprite with coolant temperature color
void GUI::draw_ej_engine(Graphics* gfx, int cx, int cy, const EngineData& data, float /*time_s*/) {
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

  // Draw engine sprite with color replacement for magenta areas
  uint16_t* fb = gfx->get_framebuffer();
  int fb_w = gfx->get_width();
  int fb_h = gfx->get_height();

  atlas.draw_with_color(fb, fb_w, fb_h, spr_motor_block,
                        cx - spr_motor_block.w/2, cy - spr_motor_block.h/2,
                        coolant_color);
}

// Draw turbo sprite with boost-based color
void GUI::draw_turbo(Graphics* gfx, int cx, int cy, const EngineData& data, float /*time_s*/) {
  // Color interpolation: green (low boost) -> yellow (moderate boost) -> red (high boost)
  // Boost range: -15 to +20 PSI
  float boost_normalized = (data.boost_psi + 15.0f) / 35.0f;
  if (boost_normalized < 0.0f) boost_normalized = 0.0f;
  if (boost_normalized > 1.0f) boost_normalized = 1.0f;

  Color turbo_color = Theme::GREEN;
  if (boost_normalized < 0.5f) {
    // Green -> Yellow (0% to 50%)
    turbo_color = lerp_color(Theme::GREEN, Theme::YELLOW, boost_normalized * 2.0f);
  } else {
    // Yellow -> Red (50% to 100%)
    turbo_color = lerp_color(Theme::YELLOW, Theme::RED, (boost_normalized - 0.5f) * 2.0f);
  }

  // Draw turbo sprite with color replacement for magenta areas
  uint16_t* fb = gfx->get_framebuffer();
  int fb_w = gfx->get_width();
  int fb_h = gfx->get_height();

  atlas.draw_with_color(fb, fb_w, fb_h, spr_turbo_housing,
                        cx - spr_turbo_housing.w/2, cy - spr_turbo_housing.h/2,
                        turbo_color);
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

// Draw coolant lines around engine
void GUI::draw_coolant_lines(Graphics* gfx, int engine_cx, int engine_cy, const EngineData& data) {
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

  // Draw lines around top and sides of engine
  int line_thickness = 4;

  // Top line
  gfx->fill_rect(engine_cx - 100, engine_cy - 120, 200, line_thickness, coolant_color);

  // Left line
  gfx->fill_rect(engine_cx - 100, engine_cy - 120, line_thickness, 100, coolant_color);

  // Right line
  gfx->fill_rect(engine_cx + 96, engine_cy - 120, line_thickness, 100, coolant_color);
}

// Draw battery with vertical fill level
void GUI::draw_battery(Graphics* gfx, int cx, int cy, const EngineData& data) {
  // For now, use throttle as a placeholder for battery level
  // TODO: Add voltage field to EngineData and map 11V-14.5V to 0-100%
  float battery_level = data.throttle / 100.0f;  // 0.0 to 1.0

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
  constexpr int bar_y = 5;
  constexpr int bar_h = 15;
  constexpr int screen_w = 960;
  constexpr int center_x = screen_w / 2;

  // RPM range: 0-8000
  constexpr int max_rpm = 8000;
  float rpm_normalized = (float)data.rpm / (float)max_rpm;
  if (rpm_normalized > 1.0f) rpm_normalized = 1.0f;

  // Each side fills half the screen width
  int half_fill = (int)(rpm_normalized * center_x);

  // Color transitions: green -> yellow -> red as RPM increases
  Color bar_color = Theme::GREEN;
  if (rpm_normalized > 0.85f) {
    bar_color = Theme::RED;
  } else if (rpm_normalized > 0.70f) {
    bar_color = Theme::YELLOW;
  }

  // Draw background (dark gray)
  gfx->fill_rect(0, bar_y, screen_w, bar_h, Theme::GRAY_DARK);

  // Draw left side (fills from left edge toward center)
  gfx->fill_rect(0, bar_y, half_fill, bar_h, bar_color);

  // Draw right side (fills from right edge toward center)
  gfx->fill_rect(screen_w - half_fill, bar_y, half_fill, bar_h, bar_color);

  // Draw RPM value in center
  char rpm_text[16];
  snprintf(rpm_text, sizeof(rpm_text), "%u", data.rpm);
  int text_w = gfx->measure_text(rpm_text, 2);
  gfx->draw_text(rpm_text, center_x - text_w/2, bar_y + 3, 2, Theme::WHITE);
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
  spr_turbo_housing = {128, 0, 128, 128};
  spr_turbo_blades = {128, 0, 128, 128};  // Same as housing for now

  // Intercooler: (0, 128) to (319, 223) = 320x96
  spr_intercooler = {0, 128, 320, 96};

  // Engine: (0, 224) to (383, 415) = 384x192
  spr_motor_block = {0, 224, 384, 192};

  printf("GUI sprites initialized\n");
}

void GUI::render(Graphics* gfx, const EngineData& data, float time_s) {
  // Get framebuffer for sprite drawing
  uint16_t* fb = gfx->get_framebuffer();
  int fb_w = gfx->get_width();
  int fb_h = gfx->get_height();

  // Draw background first (full screen, 960x320)
  Sprite bg_sprite = {0, 0, 960, 320};
  background.draw(fb, fb_w, fb_h, bg_sprite, 0, 0, false);

  // Layout centers (for 960x320 screen)
  constexpr int engine_cx = 480;
  constexpr int engine_cy = 230;
  constexpr int turbo_cx = 150;
  constexpr int turbo_cy = 200;
  constexpr int ic_cx = 480;
  constexpr int ic_cy = 80;
  constexpr int battery_cx = 800;
  constexpr int battery_cy = 200;

  // Draw top RPM bar (Assetto Corsa style)
  draw_top_rpm_bar(gfx, data);

  // Draw all components
  draw_ej_engine(gfx, engine_cx, engine_cy, data, time_s);
  draw_turbo(gfx, turbo_cx, turbo_cy, data, time_s);
  draw_intercooler(gfx, ic_cx, ic_cy, data);
  draw_battery(gfx, battery_cx, battery_cy, data);

  // Warning indicators (moved down below RPM bar)
  if (data.knock_detected) {
    gfx->fill_rect(10, 25, 180, 50, Theme::YELLOW);
    gfx->draw_text("!KNOCK!", 20, 35, 5, Theme::BLACK);
  }

  if (data.overboost) {
    gfx->fill_rect(200, 25, 280, 50, Theme::MAGENTA);
    gfx->draw_text("!OVERBOOST!", 210, 35, 5, Theme::BLACK);
  }
}
