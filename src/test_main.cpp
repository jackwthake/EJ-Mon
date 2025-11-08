#if PLATFORM_DESKTOP
//==============================================================================
// Desktop Test Build - SDL Simulator
//==============================================================================

#include "platform.hpp"
#include "can/can_parser.h"
#include "graphics/gfx.hpp"
#include "graphics/gfx_backend.hpp"
#include "gui.hpp"

#include <cstdio>
#include <cstring>
#include <chrono>
#include <thread>
#include <SDL3/SDL.h>

//==============================================================================
// Helper Functions
//==============================================================================

// Linear interpolation helper
template<typename T>
static T lerp(T current, T target, float t) {
  return current + (target - current) * t;
}

// Interpolate engine data for smooth animations
static void interpolate_engine_data(EngineData& interpolated, const EngineData& target, float speed) {
  // Interpolated values (smooth transitions)
  interpolated.rpm = (uint16_t)lerp((float)interpolated.rpm, (float)target.rpm, speed);
  interpolated.throttle = (uint8_t)lerp((float)interpolated.throttle, (float)target.throttle, speed);
  interpolated.boost_psi = lerp(interpolated.boost_psi, target.boost_psi, speed);

  // Instant updates (no interpolation needed)
  interpolated.speed = target.speed;
  interpolated.coolant_temp = target.coolant_temp;
  interpolated.timing_adv = target.timing_adv;
  interpolated.knock_count = target.knock_count;
  interpolated.knock_detected = target.knock_detected;
  interpolated.overboost = target.overboost;
}

//==============================================================================
// Main Entry Point
//==============================================================================

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

  // Initialize GUI
  GUI gui;
  gui.init();

  // Open CAN test data file
  FILE* can_file = fopen("test-data/can_data_sample.txt", "r");
  if (!can_file) {
    fprintf(stderr, "Failed to open test-data/can_data_sample.txt\n");
    delete gfx;
    return 1;
  }

  // Initialize engine data
  EngineData engine_data = {};
  EngineData engine_data_prev = {};  // Previous frame for interpolation
  EngineData engine_data_interpolated = {};  // Interpolated values for smooth display
  CANFrame frame;
  char line[256];

  bool running = true;
  SDL_Event event;

  // Performance monitoring using C++ chrono
  using Clock = std::chrono::steady_clock;
  using Ms = std::chrono::milliseconds;
  using Micros = std::chrono::microseconds;

  // Interpolation timing
  auto last_can_update = Clock::now();
  constexpr float interpolation_speed = 0.15f;  // Smoothing factor (0-1, lower = smoother)

  uint64_t frame_count = 0;
  auto last_perf_report = Clock::now();
  std::chrono::microseconds total_render_time{0};
  std::chrono::microseconds max_render_time{0};

  // CAN data playback timing
  auto playback_start = Clock::now();

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

    // Calculate elapsed time since playback started
    auto elapsed = std::chrono::duration_cast<Ms>(frame_start - playback_start);
    uint32_t elapsed_ms = elapsed.count();

    // Read and process CAN frames up to current elapsed time
    // Limit to prevent blocking the render loop
    constexpr int max_frames_per_render = 50;
    int frames_processed = 0;

    while (frames_processed < max_frames_per_render) {
      long pos = ftell(can_file);
      if (!fgets(line, sizeof(line), can_file)) {
        // End of file, rewind to loop
        rewind(can_file);
        playback_start = frame_start; // Reset playback timer
        printf("\n--- Looping CAN data ---\n\n");
        continue;
      }

      // Try to parse the line
      CANFrame temp_frame;
      if (CANParser::parse_line(line, temp_frame)) {
        // Check if this frame's timestamp is in the future
        if (temp_frame.timestamp_ms > elapsed_ms) {
          // This frame isn't due yet, rewind and break
          fseek(can_file, pos, SEEK_SET);
          break;
        }

        // Process this frame
        frame = temp_frame;
        engine_data_prev = engine_data;  // Save previous values
        CANParser::decode_frame(frame, engine_data);
        frames_processed++;
        last_can_update = frame_start;  // Mark when we got new CAN data
      }
    }

    // Smooth interpolation for display values
    interpolate_engine_data(engine_data_interpolated, engine_data, interpolation_speed);

    // Calculate time in seconds for animations
    auto time_elapsed = std::chrono::duration_cast<Micros>(frame_start - playback_start);
    float time_s = time_elapsed.count() / 1000000.0f;

    // Render frame
    gfx->clear(Theme::BLACK);
    gui.render(gfx, engine_data_interpolated, time_s);
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
    auto perf_elapsed = std::chrono::duration_cast<Ms>(frame_end - last_perf_report);
    if (perf_elapsed.count() >= 5000) {
      float avg_render_ms = total_render_time.count() / 1000.0f / frame_count;
      float max_render_ms = max_render_time.count() / 1000.0f;
      float fps = frame_count / (perf_elapsed.count() / 1000.0f);

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
