#pragma once

#include "graphics/gfx.hpp"
#include "graphics/sprite.hpp"
#include "can/can_parser.h"

// GUI state and rendering for EJ-Mon
class GUI {
public:
  // Initialize GUI (can be called once at startup)
  void init();

  // Render the complete UI given engine data and animation time
  void render(Graphics* gfx, const EngineData& data, float time_s);

private:
  // Sprite atlas and background
  SpriteAtlas atlas;
  SpriteAtlas background;

  // Sprite definitions (positions in atlas)
  Sprite spr_turbo_housing;
  Sprite spr_intercooler;
  Sprite spr_motor_block;
  Sprite spr_battery;

  // Helper functions for drawing individual components
  void draw_ej_engine(Graphics* gfx, int cx, int cy, const EngineData& data, float time_s);
  void draw_turbo(Graphics* gfx, int cx, int cy, const EngineData& data, float time_s);
  void draw_intercooler(Graphics* gfx, int cx, int cy, const EngineData& data);
  void draw_battery(Graphics* gfx, int cx, int cy, const EngineData& data);
  void draw_top_rpm_bar(Graphics* gfx, const EngineData& data);
};
