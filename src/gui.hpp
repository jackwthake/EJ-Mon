#pragma once

#include "graphics/gfx.hpp"
#include "can/can_parser.h"

// GUI state and rendering for EJ-Mon
class GUI {
public:
  // Initialize GUI (can be called once at startup)
  void init();

  // Render the complete UI given engine data and animation time
  void render(Graphics* gfx, const EngineData& data, float time_s);

private:
  // Helper functions for drawing individual components
  static void draw_ej_engine(Graphics* gfx, int cx, int cy, const EngineData& data, float time_s);
  static void draw_turbo(Graphics* gfx, int cx, int cy, const EngineData& data, float time_s);
  static void draw_intercooler(Graphics* gfx, int cx, int cy, const EngineData& data);
  static void draw_coolant_lines(Graphics* gfx, int engine_cx, int engine_cy, const EngineData& data);
  static void draw_battery(Graphics* gfx, int cx, int cy, const EngineData& data);
  static void draw_top_rpm_bar(Graphics* gfx, const EngineData& data);
};
