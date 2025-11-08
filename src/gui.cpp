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

// Draw EJ engine (top-down view) with animated pistons
void GUI::draw_ej_engine(Graphics* gfx, int cx, int cy, const EngineData& data, float time_s) {
  // EJ engine block (simplified boxer layout)
  constexpr int block_w = 180;
  constexpr int block_h = 220;
  constexpr int cylinder_spacing = 55;

  // Engine block outline
  gfx->fill_rect(cx - block_w/2, cy - block_h/2, block_w, block_h, Theme::GRAY_DARK);
  gfx->draw_rect(cx - block_w/2, cy - block_h/2, block_w, block_h, Theme::GRAY_LIGHT);

  // Piston animation based on RPM
  // RPM -> revolutions per second -> radians per second
  float rpm_rps = data.rpm / 60.0f;
  float crank_angle = time_s * rpm_rps * 2.0f * 3.14159f;

  // EJ25 firing order: 1-3-2-4
  // Cylinder positions (left bank = -1, right bank = +1)
  struct Cylinder { int x_offset; int y_offset; float phase; };
  Cylinder cylinders[4] = {
    {-70, -cylinder_spacing, 0.0f},           // Cyl 1 (left front)
    {-70, cylinder_spacing, 3.14159f},        // Cyl 2 (left rear) - 180° offset
    {70, -cylinder_spacing, 4.71239f},        // Cyl 3 (right front) - 270° offset
    {70, cylinder_spacing, 1.5708f}           // Cyl 4 (right rear) - 90° offset
  };

  // Draw cylinders and pistons
  for (int i = 0; i < 4; i++) {
    int cyl_x = cx + cylinders[i].x_offset;
    int cyl_y = cy + cylinders[i].y_offset;

    // Cylinder bore
    gfx->draw_circle(cyl_x, cyl_y, 18, Theme::GRAY_LIGHT);

    // Piston animation: moves horizontally (side-to-side) like a boxer engine
    float piston_angle = crank_angle + cylinders[i].phase;
    float piston_offset = sinf(piston_angle) * 12.0f;  // Horizontal movement distance

    // Left bank pistons move left/right from center
    // Right bank pistons move left/right from center
    int piston_x = cyl_x + (int)piston_offset;

    // Piston dimensions
    constexpr int piston_w = 14;
    constexpr int piston_h = 14;

    // Color changes based on stroke: yellow when moving outward, cyan when moving inward
    Color piston_color = (piston_offset > 0) ? Theme::YELLOW : Theme::CYAN;

    // Draw piston as a rectangle that slides horizontally
    gfx->fill_rect(piston_x - piston_w/2, cyl_y - piston_h/2,
                   piston_w, piston_h, piston_color);
    gfx->draw_rect(piston_x - piston_w/2, cyl_y - piston_h/2,
                   piston_w, piston_h, Theme::GRAY_LIGHT);
  }

  // Crankshaft center
  gfx->fill_rect(cx - 6, cy - 6, 12, 12, Theme::RED);
}

// Draw turbo with spin animation and boost-based color/scale
void GUI::draw_turbo(Graphics* gfx, int cx, int cy, const EngineData& data, float time_s) {
  // Color interpolation: blue (no boost) -> cyan -> yellow -> red (max boost)
  // Boost range: -15 to +20 PSI
  float boost_normalized = (data.boost_psi + 15.0f) / 35.0f;
  if (boost_normalized < 0.0f) boost_normalized = 0.0f;
  if (boost_normalized > 1.0f) boost_normalized = 1.0f;

  Color turbo_color = Theme::BLUE;
  if (boost_normalized < 0.5f) {
    turbo_color = lerp_color(Theme::BLUE, Theme::YELLOW, boost_normalized * 2.0f);
  } else {
    turbo_color = lerp_color(Theme::YELLOW, Theme::RED, (boost_normalized - 0.5f) * 2.0f);
  }

  uint16_t* fb = gfx->get_framebuffer();
  int fb_w = gfx->get_width();
  int fb_h = gfx->get_height();

  // Spinning compressor wheel based on boost
  float spin_speed = boost_normalized * 10.0f; // rad/s
  float wheel_angle = time_s * spin_speed;

  // Draw rotated turbo blades FIRST (so they appear behind)
  atlas.draw_rotated(fb, fb_w, fb_h, spr_turbo_blades, cx, cy - 3, wheel_angle, true);

  // Draw turbo housing LAST (on top) with color replacement for magenta areas
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

// Draw battery with voltage-based color
void GUI::draw_battery(Graphics* gfx, int cx, int cy, const EngineData& /*data*/) {
  // For now, use green (we don't have voltage in EngineData yet)
  // TODO: Map voltage 11V-14.5V to red->yellow->green
  Color bat_color = Theme::GREEN;

  // Draw battery sprite with color replacement for magenta areas
  uint16_t* fb = gfx->get_framebuffer();
  int fb_w = gfx->get_width();
  int fb_h = gfx->get_height();

  atlas.draw_with_color(fb, fb_w, fb_h, spr_battery,
                        cx - spr_battery.w/2, cy - spr_battery.h/2,
                        bat_color);
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

  // Define sprite positions in atlas (16x16 tile grid)
  // Coordinates provided by user in tiles, converted to pixels
  // Format: {x_pixels, y_pixels, width_pixels, height_pixels}

  // Turbo housing: tiles (0,5) to (5,12) = 5 tiles wide, 7 tiles tall
  spr_turbo_housing = {0, 80, 96, 128};    // 0*16=0, 5*16=80, 5*16=80, 7*16=112

  // Turbo blades: tiles (6,5) to (9,8) = 3 tiles wide, 3 tiles tall
  spr_turbo_blades = {96, 80, 64, 64};     // 6*16=96, 5*16=80, 3*16=48, 3*16=48

  // Battery: tiles (6,9) to (9,12) = 3 tiles wide, 3 tiles tall
  spr_battery = {96, 144, 64, 64};         // 6*16=96, 9*16=144, 3*16=48, 3*16=48

  // Intercooler: tiles (0,0) to (13,4) = 13 tiles wide, 4 tiles tall
  spr_intercooler = {0, 0, 224, 80};       // 0*16=0, 0*16=0, 13*16=208, 4*16=64

  // Motor block: Not defined yet
  spr_motor_block = {0, 0, 0, 0};          // TODO: Define motor block sprite

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
  constexpr int turbo_cx = 200;
  constexpr int turbo_cy = 120;
  constexpr int ic_cx = 480;
  constexpr int ic_cy = 80;
  constexpr int battery_cx = 760;
  constexpr int battery_cy = 120;

  // Draw top RPM bar (Assetto Corsa style)
  draw_top_rpm_bar(gfx, data);

  // Draw all components
  draw_ej_engine(gfx, engine_cx, engine_cy, data, time_s);
  draw_coolant_lines(gfx, engine_cx, engine_cy, data);
  draw_turbo(gfx, turbo_cx, turbo_cy, data, time_s);
  draw_intercooler(gfx, ic_cx, ic_cy, data);
  draw_battery(gfx, battery_cx, battery_cy, data);

  // Warning indicators (moved down below RPM bar)
  if (data.knock_detected) {
    gfx->fill_rect(10, 25, 100, 30, Theme::YELLOW);
    gfx->draw_text("!KNOCK!", 15, 33, 1, Theme::BLACK);
  }

  if (data.overboost) {
    gfx->fill_rect(120, 25, 120, 30, Theme::MAGENTA);
    gfx->draw_text("!OVERBOOST!", 125, 33, 1, Theme::BLACK);
  }
}
