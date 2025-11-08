#if PLATFORM_DESKTOP
//==============================================================================
// Desktop Test Build - SDL Simulator
//==============================================================================

#include "platform.hpp"
#include "can/can_parser.h"
#include "graphics/gfx.hpp"
#include "graphics/gfx_backend.hpp"
#include "graphics/gfx_types.hpp"

#include <cstdio>
#include <cstring>
#include <chrono>
#include <thread>
#include <SDL3/SDL.h>

int test_main(int /*argc*/, char* /*argv*/[]) {
  printf("EJ-Mon SDL Simulator started\n");
  printf("Display: %dx%d\n", DISPLAY_WIDTH, DISPLAY_HEIGHT);

  // Create graphics backend
  Graphics* gfx = create_graphics();
  if (!gfx) {
    fprintf(stderr, "Failed to create graphics backend\n");
    return 1;
  }

  gfx->begin();

  // Open CAN test data file
  FILE* can_file = fopen("test-data/can_data_sample.txt", "r");
  if (!can_file) {
    fprintf(stderr, "Failed to open test-data/can_data_sample.txt\n");
    delete gfx;
    return 1;
  }

  // Initialize engine data
  EngineData engine_data = {};
  CANFrame frame;
  char line[256];

  bool running = true;
  SDL_Event event;

  // Performance monitoring using C++ chrono
  using Clock = std::chrono::steady_clock;
  using Ms = std::chrono::milliseconds;
  using Micros = std::chrono::microseconds;

  uint64_t frame_count = 0;
  auto last_perf_report = Clock::now();
  std::chrono::microseconds total_render_time{0};
  std::chrono::microseconds max_render_time{0};

  printf("\n--- Starting CAN data replay ---\n");
  printf("Simulating ESP32-S3 @ 240MHz performance constraints\n\n");

  while (running) {
    auto frame_start = Clock::now();

    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        running = false;
      }
      if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
        running = false;
      }
    }

    // Read next line from CAN data file
    if (fgets(line, sizeof(line), can_file)) {
      if (CANParser::parse_line(line, frame)) {
        // Decode the CAN frame
        CANParser::decode_frame(frame, engine_data);

        // Print decoded data to console
        printf("t=%5u | RPM=%4u | Speed=%3u MPH | Throttle=%3u%% | Coolant=%3dC | Boost=%+5.1f PSI | Knock=%u | Timing=%+3d\n",
               frame.timestamp_ms,
               engine_data.rpm,
               engine_data.speed,
               engine_data.throttle,
               engine_data.coolant_temp,
               engine_data.boost_psi,
               engine_data.knock_count,
               engine_data.timing_adv);
      }
    } else {
      // End of file, rewind to loop
      rewind(can_file);
      printf("\n--- Rewinding CAN data ---\n\n");
    }

    // Clear screen to black
    gfx->clear(Theme::BLACK);

    // Draw engine data on screen
    char value_text[64];

    // Left column - Bar gauges (limited width to not overlap right side)
    constexpr int max_bar_width = 400;
    constexpr int bar_height = 30;

    // RPM gauge (0-8000 RPM range)
    int rpm_width = (engine_data.rpm * max_bar_width) / 8000;
    if (rpm_width > max_bar_width) rpm_width = max_bar_width;

    gfx->fill_rect(50, 50, max_bar_width, bar_height, Theme::GRAY_DARK);
    gfx->fill_rect(50, 50, rpm_width, bar_height, Theme::RED);

    snprintf(value_text, sizeof(value_text), "RPM: %u", engine_data.rpm);
    gfx->draw_text(value_text, 55, 60, 2, Theme::GRAY_LIGHT);

    // Boost gauge (-15 to +20 PSI range)
    int boost_width = (int)((engine_data.boost_psi + 15.0f) * (max_bar_width / 35.0f));
    if (boost_width < 0) boost_width = 0;
    if (boost_width > max_bar_width) boost_width = max_bar_width;

    gfx->fill_rect(50, 100, max_bar_width, bar_height, Theme::GRAY_DARK);
    gfx->fill_rect(50, 100, boost_width, bar_height, Theme::CYAN);

    snprintf(value_text, sizeof(value_text), "BOOST: %+.1f PSI", engine_data.boost_psi);
    gfx->draw_text(value_text, 55, 110, 2, Theme::GRAY_LIGHT);

    // Throttle gauge (0-100%)
    int throttle_width = (engine_data.throttle * max_bar_width) / 100;
    if (throttle_width > max_bar_width) throttle_width = max_bar_width;

    gfx->fill_rect(50, 150, max_bar_width, bar_height, Theme::GRAY_DARK);
    gfx->fill_rect(50, 150, throttle_width, bar_height, Theme::GREEN);

    snprintf(value_text, sizeof(value_text), "THROTTLE: %u%%", engine_data.throttle);
    gfx->draw_text(value_text, 55, 160, 2, Theme::GRAY_LIGHT);

    // Right column - Additional info
    snprintf(value_text, sizeof(value_text), "SPEED: %u MPH", engine_data.speed);
    gfx->draw_text(value_text, 480, 50, 2, Theme::GRAY_LIGHT);

    snprintf(value_text, sizeof(value_text), "COOLANT: %dC", engine_data.coolant_temp);
    gfx->draw_text(value_text, 480, 80, 2, Theme::GRAY_LIGHT);

    snprintf(value_text, sizeof(value_text), "TIMING: %+d", engine_data.timing_adv);
    gfx->draw_text(value_text, 480, 110, 2, Theme::GRAY_LIGHT);

    // Warning indicators at bottom
    if (engine_data.knock_detected) {
      gfx->fill_rect(50, 270, 120, 40, Theme::YELLOW);
      gfx->draw_text("!KNOCK!", 55, 282, 2, Theme::BLACK);
    }

    if (engine_data.overboost) {
      gfx->fill_rect(190, 270, 180, 40, Theme::MAGENTA);
      gfx->draw_text("!OVERBOOST!", 195, 282, 2, Theme::BLACK);
    }

    gfx->present();

    // Calculate render time for this frame
    auto frame_end = Clock::now();
    auto render_time = std::chrono::duration_cast<Micros>(frame_end - frame_start);

    total_render_time += render_time;
    if (render_time > max_render_time) {
      max_render_time = render_time;
    }
    frame_count++;

    // Performance report every 5 seconds
    auto elapsed = std::chrono::duration_cast<Ms>(frame_end - last_perf_report);
    if (elapsed.count() >= 5000) {
      float avg_render_ms = total_render_time.count() / 1000.0f / frame_count;
      float max_render_ms = max_render_time.count() / 1000.0f;
      float fps = frame_count / (elapsed.count() / 1000.0f);

      printf("\n[PERF] Avg render: %.2fms | Max: %.2fms | FPS: %.1f\n",
             avg_render_ms, max_render_ms, fps);

      // Check if we're meeting performance targets
      if (avg_render_ms > 16.67f) {
        printf("       WARNING: Avg render exceeds 60 FPS budget (16.67ms)\n");
      }
      if (max_render_ms > 33.0f) {
        printf("       WARNING: Max render exceeds 30 FPS budget (33ms)\n");
      }

      // Reset counters
      total_render_time = Micros{0};
      max_render_time = Micros{0};
      frame_count = 0;
      last_perf_report = frame_end;
    }

    // Target 60 FPS (16.67ms per frame)
    std::this_thread::sleep_for(Ms(16));
  }

  fclose(can_file);
  delete gfx;

  return 0;
}

// Entry point for desktop
int main(int argc, char* argv[]) {
  return test_main(argc, argv);
}

#endif
