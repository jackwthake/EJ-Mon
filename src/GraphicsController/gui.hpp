#pragma once

#include "gfx.hpp"
#include "sprite.hpp"
#include "../can/can_parser.h"

constexpr uint16_t SHIFT_LIGHT_RPM = 5500;  // RPM to start shift light

// GUI state and rendering for EJ-Mon
class GUI {
public:
  // Initialize GUI (can be called once at startup)
  void init(Graphics* gfx);

  // Render the complete UI given engine data and animation time
  void render(Graphics* gfx, const EngineData& data, float time_s);

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

  // Cube rotation state (constantly spinning)
  float cube1_yaw = 0.0f;
  float cube1_pitch = 0.0f;
  float cube2_yaw = 0.0f;
  float cube2_pitch = 0.0f;

  // Helper functions for drawing individual components
  void draw_ej_engine(Graphics* gfx, int cx, int cy, const EngineData& data, float time_s);
  void draw_turbo(Graphics* gfx, int cx, int cy, const EngineData& data, float time_s);
  void draw_intercooler(Graphics* gfx, int cx, int cy, const EngineData& data);
  void draw_intake_manifold(Graphics* gfx, int cx, int cy, const EngineData& data);
  void draw_battery(Graphics* gfx, int cx, int cy, const EngineData& data);
  void draw_cam_gears(Graphics* gfx, int engine_cx, int engine_cy, const EngineData& data, float time_s);
  void draw_top_rpm_bar(Graphics* gfx, const EngineData& data);
  void draw_engine_param_labels(Graphics* gfx, int cx, int cy, const EngineData& data);
};
