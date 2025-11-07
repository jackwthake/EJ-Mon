#if PLATFORM_DESKTOP
//==============================================================================
// Desktop Test Build - SDL Simulator
//==============================================================================

// Display dimensions matching the actual hardware

#include "platform.h"
#include "can/can_parser.h"
#include <cstdio>
#include <cstring>

#include <SDL3/SDL.h>

#include "grapghics/bitmap_font.h"

void draw_char_3x5(SDL_Renderer* renderer, char c, int x, int y, int scale) {
  int idx = -1;

  if (c >= '0' && c <= '9') {
    idx = c - '0';
  } else if (c >= 'A' && c <= 'Z') {
    idx = 10 + (c - 'A');
  } else if (c >= 'a' && c <= 'z') {
    idx = 10 + (c - 'a');
  } else if (c == ' ') {
    idx = 36;
  } else if (c == '.') {
    idx = 37;
  } else if (c == ':') {
    idx = 38;
  } else if (c == '/') {
    idx = 39;
  } else if (c == '+') {
    idx = 40;
  } else if (c == '-') {
    idx = 41;
  } else if (c == ',') {
    idx = 42;
  } else if (c == '!') {
    idx = 43;
  } else if (c == '?') {
    idx = 44;
  } else if (c == '%') {
    idx = 45;
  }

  if (idx == -1) return;

  for (int row = 0; row < 5; row++) {
    uint8_t bits = font_3x5[idx][row];
    for (int col = 0; col < 3; col++) {
      if (bits & (1 << (2 - col))) {
        SDL_FRect pixel = {
          (float)(x + col * scale),
          (float)(y + row * scale),
          (float)scale,
          (float)scale
        };
        SDL_RenderFillRect(renderer, &pixel);
      }
    }
  }
}

void draw_simple_text(SDL_Renderer* renderer, const char* text, int x, int y, int scale = 1) {
  int cursor_x = x;
  for (int i = 0; text[i] != '\0'; i++) {
    draw_char_3x5(renderer, text[i], cursor_x, y, scale);
    cursor_x += 4 * scale; // 3 pixels width + 1 pixel spacing
  }
}

int test_main(int /*argc*/, char* /*argv*/[]) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
    return 1;
  }

  SDL_Window* window = SDL_CreateWindow(
    "EJ-Mon Simulator",
    DISPLAY_WIDTH, DISPLAY_HEIGHT,
    SDL_WINDOW_RESIZABLE
  );

  if (!window) {
    fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
    SDL_Quit();
    return 1;
  }

  SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
  if (!renderer) {
    fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  printf("EJ-Mon SDL Simulator started\n");
  printf("Display: %dx%d\n", DISPLAY_WIDTH, DISPLAY_HEIGHT);

  // Open CAN test data file
  FILE* can_file = fopen("test-data/can_data_sample.txt", "r");
  if (!can_file) {
    fprintf(stderr, "Failed to open test-data/can_data_sample.txt\n");
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  // Initialize engine data
  EngineData engine_data = {};
  CANFrame frame;
  char line[256];

  bool running = true;
  SDL_Event event;

  printf("\n--- Starting CAN data replay ---\n");

  while (running) {
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
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Draw engine data on screen
    char value_text[64];

    // Left column - Bar gauges (limited width to not overlap right side)
    int max_bar_width = 400;

    // RPM gauge (0-8000 RPM range)
    int rpm_width = (engine_data.rpm * max_bar_width) / 8000;
    if (rpm_width > max_bar_width) rpm_width = max_bar_width;

    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_FRect rpm_bg = {50, 50, (float)max_bar_width, 30};
    SDL_RenderFillRect(renderer, &rpm_bg);

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_FRect rpm_bar = {50, 50, (float)rpm_width, 30};
    SDL_RenderFillRect(renderer, &rpm_bar);

    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    snprintf(value_text, sizeof(value_text), "RPM: %u", engine_data.rpm);
    draw_simple_text(renderer, value_text, 55, 60, 2);

    // Boost gauge (-15 to +20 PSI range)
    int boost_width = (int)((engine_data.boost_psi + 15.0f) * (max_bar_width / 35.0f));
    if (boost_width < 0) boost_width = 0;
    if (boost_width > max_bar_width) boost_width = max_bar_width;

    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_FRect boost_bg = {50, 100, (float)max_bar_width, 30};
    SDL_RenderFillRect(renderer, &boost_bg);

    SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
    SDL_FRect boost_bar = {50, 100, (float)boost_width, 30};
    SDL_RenderFillRect(renderer, &boost_bar);

    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    snprintf(value_text, sizeof(value_text), "BOOST: %+.1f PSI", engine_data.boost_psi);
    draw_simple_text(renderer, value_text, 55, 110, 2);

    // Throttle gauge (0-100%)
    int throttle_width = (engine_data.throttle * max_bar_width) / 100;
    if (throttle_width > max_bar_width) throttle_width = max_bar_width;

    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_FRect throttle_bg = {50, 150, (float)max_bar_width, 30};
    SDL_RenderFillRect(renderer, &throttle_bg);

    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_FRect throttle_bar = {50, 150, (float)throttle_width, 30};
    SDL_RenderFillRect(renderer, &throttle_bar);

    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    snprintf(value_text, sizeof(value_text), "THROTTLE: %u%%", engine_data.throttle);
    draw_simple_text(renderer, value_text, 55, 160, 2);

    // Right column - Additional info
    SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);

    snprintf(value_text, sizeof(value_text), "SPEED: %u MPH", engine_data.speed);
    draw_simple_text(renderer, value_text, 480, 50, 2);

    snprintf(value_text, sizeof(value_text), "COOLANT: %dC", engine_data.coolant_temp);
    draw_simple_text(renderer, value_text, 480, 80, 2);

    snprintf(value_text, sizeof(value_text), "TIMING: %+d", engine_data.timing_adv);
    draw_simple_text(renderer, value_text, 480, 110, 2);

    // Warning indicators at bottom
    if (engine_data.knock_detected) {
      SDL_SetRenderDrawColor(renderer, 255, 200, 0, 255);
      SDL_FRect knock_indicator = {50, 270, 120, 40};
      SDL_RenderFillRect(renderer, &knock_indicator);
      SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
      draw_simple_text(renderer, "!KNOCK!", 55, 282, 2);
    }

    if (engine_data.overboost) {
      SDL_SetRenderDrawColor(renderer, 255, 0, 200, 255);
      SDL_FRect overboost_indicator = {190, 270, 180, 40};
      SDL_RenderFillRect(renderer, &overboost_indicator);
      SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
      draw_simple_text(renderer, "!OVERBOOST!", 195, 282, 2);
    }

    SDL_RenderPresent(renderer);

    SDL_Delay(16); // ~60fps
  }

  fclose(can_file);

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}

// Entry point for desktop
int main(int argc, char* argv[]) {
  return test_main(argc, argv);
}

#endif