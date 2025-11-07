#if PLATFORM_ESP32
//==============================================================================
// ESP32 Hardware Build
//==============================================================================

#include "platform.h"
#include <cstdio>

// ESP32 uses app_main() as entry point, not main()
extern "C" void app_main() {
  printf("Car-Mon ESP32 starting...\n");
  printf("Display: %dx%d\n", DISPLAY_WIDTH, DISPLAY_HEIGHT);

  // TODO: Initialize hardware
  // - TFT display
  // - CAN bus module (MCP2515)
  // - Create FreeRTOS tasks

  // Main loop will be handled by FreeRTOS tasks
  while (true) {
    // Your graphics/animation code here
    // This will be replaced with proper FreeRTOS tasks
  }
}

#endif
