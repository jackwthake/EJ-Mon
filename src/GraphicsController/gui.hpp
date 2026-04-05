#pragma once

#include "DisplayDriver.hpp"
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

  bool last_frame_above_shift_rpm = false;

  // Timing mark animation state
  float timing_mark_accumulator = 0.0f;  // Accumulates fractional increments
  int active_timing_mark = 0;             // Current active mark (0-29)

  // Intake manifold tick animation state
  float intake_tick_accumulator = 0.0f;  // Accumulates fractional increments
  int active_intake_tick = 0;             // Current active tick (0-4)

  // Helper functions for drawing individual components
};

// Global variables (defined in gui.cpp)
extern Display_HDC458002C40 *display;
extern GUI gui;
extern EngineData engine_data;

// Main rendering task
void gui_task(void *param);
