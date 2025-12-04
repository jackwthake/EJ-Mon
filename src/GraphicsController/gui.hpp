#pragma once

#include "gfx.hpp"
#include "sprite.hpp"
#include "../can/can_parser.h"

constexpr uint16_t SHIFT_LIGHT_RPM = 5500;  // RPM to start shift light

// GUI state and rendering for EJ-Mon
class GUI {
public:
  // Initialize GUI (can be called once at startup)
  void init(Display_HDC458002C40 *gfx);

  // Render the complete UI given engine data and animation time
  void render(Display_HDC458002C40 *gfx, const EngineData& data, float time_s);

private:
  // Sprite atlas
  SpriteAtlas atlas;

  // Sprite definitions (positions in atlas)
  Sprite spr_turbo_housing;
  Sprite spr_intercooler;
  Sprite spr_motor_block;
  Sprite spr_battery;
  Sprite spr_cam_gear;
  Sprite spr_intake_manifold;
  Sprite spr_knock_warning;
  Sprite spr_overboost_warning;

  // label sprites
  Sprite spr_rpm_label;
  Sprite spr_boost_label;
  Sprite spr_temp_label;
  Sprite spr_battery_label;

  bool last_frame_above_shift_rpm = false;

  // Timing mark animation state
  float timing_mark_accumulator = 0.0f;  // Accumulates fractional increments
  int active_timing_mark = 0;             // Current active mark (0-29)

  // Intake manifold tick animation state
  float intake_tick_accumulator = 0.0f;  // Accumulates fractional increments
  int active_intake_tick = 0;             // Current active tick (0-4)

  // Helper functions for drawing individual components
  void draw_ej_engine(Display_HDC458002C40 *gfx, int cx, int cy, const EngineData& data, float time_s);
  void draw_turbo(Display_HDC458002C40 *gfx, int cx, int cy, const EngineData& data, float time_s);
  void draw_intercooler(Display_HDC458002C40 *gfx, int cx, int cy, const EngineData& data);
  void draw_intake_manifold(Display_HDC458002C40 *gfx, int cx, int cy, const EngineData& data);
  void draw_battery(Display_HDC458002C40 *gfx, int cx, int cy, const EngineData& data);
  void draw_cam_gears(Display_HDC458002C40 *gfx, int engine_cx, int engine_cy, const EngineData& data, float time_s);
  void draw_top_rpm_bar(Display_HDC458002C40 *gfx, const EngineData& data);
  void draw_engine_param_labels(Display_HDC458002C40 *gfx, int cx, int cy, const EngineData& data);
};
