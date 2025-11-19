#include "gui.hpp"
#include "graphics/gfx_types.hpp"
#include "graphics/sprite.hpp"
#include <cmath>
#include <cfloat>
#include <cstdio>
#include <iostream>

extern "C" {
#include <shader-works/renderer.h>
#include <shader-works/primitives.h>
#include <shader-works/maths.h>
}

//==============================================================================
// shader-works Color Conversion Functions
//==============================================================================
// Required by shader-works for pixel format conversion
// Note: shader-works uses u32 internally, we convert to/from RGB565

// Convert RGB888 to RGB565 (packed as u32)
u32 rgb_to_u32(u8 r, u8 g, u8 b) {
  // Convert 8-bit RGB to 5-6-5 format
  u32 r5 = (r >> 3) & 0x1F;  // 5 bits for red
  u32 g6 = (g >> 2) & 0x3F;  // 6 bits for green
  u32 b5 = (b >> 3) & 0x1F;  // 5 bits for blue
  return (r5 << 11) | (g6 << 5) | b5;
}

// Convert RGB565 (packed as u32) to RGB888
void u32_to_rgb(u32 color, u8 *r, u8 *g, u8 *b) {
  // Extract RGB565 components
  u32 r5 = (color >> 11) & 0x1F;
  u32 g6 = (color >> 5) & 0x3F;
  u32 b5 = color & 0x1F;

  // Convert back to 8-bit (with proper scaling to preserve full range)
  *r = (r5 << 3) | (r5 >> 2);  // 5-bit to 8-bit
  *g = (g6 << 2) | (g6 >> 4);  // 6-bit to 8-bit
  *b = (b5 << 3) | (b5 >> 2);  // 5-bit to 8-bit
}

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
                                  coolant_color, Theme::GREEN, active_timing_mark, NUM_TIMING_MARKS);
}

void GUI::draw_cubes(Graphics* gfx, int cx, int cy, const EngineData& data) {
  // Clear buffers
  for (int i = 0; i < CUBES_DISPLAY_WIDTH * CUBES_DISPLAY_HEIGHT; ++i) {
    color_buffer[i] = 0x000000;  // Black background
    depth_buffer[i] = FLT_MAX;
  }

  // Create camera transform
  transform_t camera = {0};
  camera.position = make_float3(0.0f, 0.0f, 0.0f);
  camera.yaw = 0.0f;
  camera.pitch = 0.0f;
  update_camera(&renderer_state, &camera);

  // Enable wireframe mode
  renderer_state.wireframe_mode = true;

  // Base rotation speed (at 60 FPS, this gives nice continuous motion)
  const float BASE_SPEED = 0.1f;
  const float BASE_CUBE_SIZE = 4.0f;
  const float CUBE_DEPTH = -15.0f;  // Further back to prevent clipping

  // Cube 1: Throttle affects rotation speed and size
  float throttle_normalized = data.throttle / 100.0f;
  float throttle_speed = throttle_normalized * 2.0f + 0.5f;  // 0.5x to 2.5x speed
  float cube1_size = BASE_CUBE_SIZE + throttle_normalized * 2.0f;  // 4.0 to 6.0 units
  cube1_yaw += BASE_SPEED * throttle_speed * 1.3f;
  cube1_pitch += BASE_SPEED * throttle_speed * 0.7f;

  model_t cube1 = {0};
  generate_cube(&cube1, make_float3(0.0f, 0.0f, CUBE_DEPTH), make_float3(cube1_size, cube1_size, cube1_size));
  cube1.transform.yaw = cube1_yaw;
  cube1.transform.pitch = cube1_pitch;
  render_model(&renderer_state, &camera, &cube1, NULL, 0);
  delete_model(&cube1);

  // Clear only the depth buffer to prevent clipping between cubes
  // Keep color buffer so wireframes accumulate
  for (int i = 0; i < CUBES_DISPLAY_WIDTH * CUBES_DISPLAY_HEIGHT; ++i) {
    depth_buffer[i] = FLT_MAX;
  }

  // Cube 2: Load affects rotation speed and size - rotates on both axes differently
  // Engine load in CAN data ranges from ~20% (idle) to ~130% (full throttle)
  // Map this to 0.0-1.0 for better visual variation
  float load_normalized = (data.engine_load - 20.0f) / 110.0f;  // 20-130% -> 0.0-1.0
  if (load_normalized < 0.0f) load_normalized = 0.0f;
  if (load_normalized > 1.0f) load_normalized = 1.0f;
  float load_speed = load_normalized * 2.0f + 0.4f;  // 0.4x to 2.4x speed
  float cube2_size = BASE_CUBE_SIZE + load_normalized * 2.2f;  // 4.0 to 6.2 units
  cube2_yaw += BASE_SPEED * load_speed * 0.8f;
  cube2_pitch += BASE_SPEED * load_speed * 1.5f;

  model_t cube2 = {0};
  generate_cube(&cube2, make_float3(0.0f, 0.0f, CUBE_DEPTH), make_float3(cube2_size * 0.75f, cube2_size * 0.75f, cube2_size * 0.75f));
  cube2.transform.yaw = cube2_yaw;
  cube2.transform.pitch = cube2_pitch;
  render_model(&renderer_state, &camera, &cube2, NULL, 0);
  delete_model(&cube2);

  // Blit the color buffer to the framebuffer at (cx, cy)
  uint16_t* fb = gfx->get_framebuffer();
  int fb_w = gfx->get_width();
  int fb_h = gfx->get_height();

  for (int y = 0; y < CUBES_DISPLAY_HEIGHT; ++y) {
    for (int x = 0; x < CUBES_DISPLAY_WIDTH; ++x) {
      int screen_x = cx + x - CUBES_DISPLAY_WIDTH / 2;
      int screen_y = cy + y - CUBES_DISPLAY_HEIGHT / 2;

      if (screen_x >= 0 && screen_x < fb_w && screen_y >= 0 && screen_y < fb_h) {
        u32 pixel = color_buffer[y * CUBES_DISPLAY_WIDTH + x];
        // Convert from u32 to RGB565 (already handled by our conversion functions)
        if (pixel == 0x000000) continue;  // Skip black pixels (background)
        fb[screen_y * fb_w + screen_x] = (uint16_t)pixel;
      }
    }
  }
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
  float scale = 1.0f + (scale_t * 0.15f);  // 1.0 to 1.5

  // Draw turbo sprite with color replacement and scaling
  uint16_t* fb = gfx->get_framebuffer();
  int fb_w = gfx->get_width();
  int fb_h = gfx->get_height();

  atlas.draw_with_color_scaled(fb, fb_w, fb_h, spr_turbo_housing,
                                cx, cy, turbo_color, scale);
}

// Draw intake manifold with throttle-based tick animation
void GUI::draw_intake_manifold(Graphics* gfx, int cx, int cy, const EngineData& data) {
  // 5 ticks that continuously cycle through, speed controlled by throttle
  const int NUM_TICKS = 5;

  // Calculate animation speed based on throttle (0-100%)
  float throttle_normalized = data.throttle / 100.0f;  // Normalize to 0-1
  float speed_factor = 0.1f + (throttle_normalized * 0.8f);  // ~2.5 cycles/sec at 100% throttle @60fps

  // Accumulate fractional increments
  intake_tick_accumulator += speed_factor;

  // When accumulator >= 1.0, increment the active tick
  if (intake_tick_accumulator >= 1.0f) {
    int steps = (int)intake_tick_accumulator;
    active_intake_tick = (active_intake_tick + steps) % NUM_TICKS;
    intake_tick_accumulator -= steps;
  }

  // Use white for the sprite body (replaces magenta)
  Color tick_color = Theme::CYAN;

  // Draw intake manifold sprite with green tick sequence
  uint16_t* fb = gfx->get_framebuffer();
  int fb_w = gfx->get_width();
  int fb_h = gfx->get_height();

  atlas.draw_with_green_sequence(fb, fb_w, fb_h, spr_intake_manifold,
                                  cx - spr_intake_manifold.w/2, cy - spr_intake_manifold.h/2,
                                  Theme::BLACK, tick_color, active_intake_tick, NUM_TICKS);
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
  // Camshafts rotate at half crankshaft speed (2:1 ratio), then scale down for visual effect
  // Calculate rotation angle based on RPM and time
  float scale = 0.15f;
  float cam_rpm = (data.rpm / 2.0f) * scale;
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
  int gear1_y = engine_top + 41;

  int gear2_x = engine_left + 345;
  int gear2_y = engine_top + 41;

  int gear3_x = engine_left + 40;
  int gear3_y = engine_top + 137;

  int gear4_x = engine_left + 345;
  int gear4_y = engine_top + 137;

  // Draw all 4 cam gears rotating at the same speed
  atlas.draw_rotated(fb, fb_w, fb_h, spr_cam_gear, gear1_x, gear1_y, angle, true);
  atlas.draw_rotated(fb, fb_w, fb_h, spr_cam_gear, gear2_x, gear2_y, angle, true);
  atlas.draw_rotated(fb, fb_w, fb_h, spr_cam_gear, gear3_x, gear3_y, angle, true);
  atlas.draw_rotated(fb, fb_w, fb_h, spr_cam_gear, gear4_x, gear4_y, angle, true);
}

// Draw battery with vertical fill level
void GUI::draw_battery(Graphics* gfx, int cx, int cy, const EngineData& data) {
  // Map battery voltage (10V-15V) to fill level (0.0-1.0)
  float battery_level = (data.battery_voltage - 10.0f) / 5.0f;  // 10V=0.0, 15V=1.0
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

void GUI::draw_engine_param_labels(Graphics* gfx, int cx, int cy, const EngineData& data) {
  const int label_x = cx - 140;

  // Draw RPM label
  atlas.draw(gfx->get_framebuffer(), gfx->get_width(), gfx->get_height(),
              spr_rpm_label, label_x, cy - 40, true);
  atlas.draw_number_from_atlas(gfx->get_framebuffer(), gfx->get_width(), gfx->get_height(),
                              label_x, cy - 128, data.rpm, 2, Theme::WHITE);
  // Draw Temp label
  atlas.draw(gfx->get_framebuffer(), gfx->get_width(), gfx->get_height(),
              spr_temp_label, label_x, cy + 86, true);

  // convert to fahrenheit for display
  int temp_display = (int)(data.coolant_temp * 9.0f / 5.0f + 32.0f);
  atlas.draw_number_from_atlas(gfx->get_framebuffer(), gfx->get_width(), gfx->get_height(),
                              label_x, cy, temp_display, 3, Theme::CYAN);
  // draw °
  atlas.draw_symbol_from_atlas(gfx->get_framebuffer(), gfx->get_width(), gfx->get_height(),
                              label_x + 144, cy, (unsigned char)248, Theme::CYAN);
}

//==============================================================================
// Public GUI Methods
//==============================================================================

void GUI::init(Graphics* gfx) {
  // Load sprite atlas
  if (!atlas.load_from_file("res/atlas.bmp")) {
    fprintf(stderr, "Failed to load sprite atlas!\n");
  }

  // Define sprite positions in atlas (from SVG export)
  // Format: {x_pixels, y_pixels, width_pixels, height_pixels}

  // Battery: (0, 0) to (127, 95) = 128x96
  spr_battery = {0, 0, 128, 96};

  // Turbo: (128, 0) to (255, 127) = 128x128
  spr_turbo_housing = {128, 0, 124, 92};

  // Intercooler: (0, 128) to (319, 223) = 320x96
  spr_intercooler = {256, 64, 151, 64};

  // Engine: (0, 224) to (383, 415) = 384x192
  spr_motor_block = {0, 176, 384, 192};

  // Cam gear: (256, 0) to (319, 63) = 64x64
  spr_cam_gear = {256, 0, 64, 64};

  // Intake manifold: (0, 128) to (248, 201) = 248x73
  spr_intake_manifold = {0, 96, 248, 73};

  // Knock warning: (0, 368) to (179, 428) = 179x60
  spr_knock_warning = {416, 176, 64, 64};

  // Overboost warning: (192, 368) to (477, 428) = 285x60
  spr_overboost_warning = {480, 176, 64, 64};

  spr_rpm_label = {416, 0, 128, 32};

  spr_boost_label = {416, 64, 224, 32};

  spr_temp_label = {416, 128, 252, 32};

  spr_battery_label = {416, 192, 248, 32};

  printf("GUI sprites initialized\n");

  // Initialize shader-works
  init_renderer(&renderer_state, CUBES_DISPLAY_WIDTH, CUBES_DISPLAY_HEIGHT, 0, 0, 
                color_buffer, depth_buffer, 15.f);
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
  // Wabi Sabi ui design lol 
  constexpr int engine_cx = 500;
  constexpr int engine_cy = 230;
  constexpr int turbo_cx = 336;
  constexpr int turbo_cy = 80;
  constexpr int ic_cx = 500;
  constexpr int ic_cy = 70;
  constexpr int intake_cx = 497;
  constexpr int intake_cy = 130;  // Between intercooler and engine

  constexpr int battery_cx = 820;
  constexpr int battery_cy = 260;
  constexpr int cubes_cx = 820;
  constexpr int cubes_cy = 150;

  constexpr int labels_cx = 160;
  constexpr int labels_cy = 180;
  constexpr int warning_cx = 820;
  constexpr int warning_cy = 20;

  draw_top_rpm_bar(gfx, data);

  // Draw all components
  draw_intake_manifold(gfx, intake_cx, intake_cy, data);
  draw_ej_engine(gfx, engine_cx, engine_cy, data, time_s);
  draw_cam_gears(gfx, engine_cx, engine_cy, data, time_s);
  draw_cubes(gfx, cubes_cx, cubes_cy, data);
  draw_turbo(gfx, turbo_cx, turbo_cy, data, time_s);
  draw_intercooler(gfx, ic_cx, ic_cy, data);
  draw_battery(gfx, battery_cx, battery_cy, data);
  draw_engine_param_labels(gfx, labels_cx, labels_cy, data);

  // Draw warning messages on top of everything
  if (data.knock_detected) {
    atlas.draw(fb, fb_w, fb_h, spr_knock_warning, warning_cx - 72, warning_cy, true);
  }

  if (data.overboost) {
    atlas.draw(fb, fb_w, fb_h, spr_overboost_warning, warning_cx + 12, warning_cy, true);
  }
}
