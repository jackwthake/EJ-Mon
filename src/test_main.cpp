#if PLATFORM_DESKTOP
//==============================================================================
// Desktop Test Build - SDL Simulator
//==============================================================================

// Display dimensions matching the actual hardware

#include "platform.h"
#include <cstdio>

#include <SDL3/SDL.h>

int test_main(int argc, char* argv[]) {
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

  bool running = true;
  SDL_Event event;

  while (running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        running = false;
      }
      if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
        running = false;
      }
    }

    // Clear screen to black
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Draw a test rectangle (cyan, like that psychedelic vibe)
    SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
    SDL_FRect rect = {100, 100, 200, 120};
    SDL_RenderFillRect(renderer, &rect);

    SDL_RenderPresent(renderer);

    SDL_Delay(16); // ~60fps
  }

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