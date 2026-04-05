// Desktop entry point for ej-mon
// This file is only compiled on desktop builds (not on ESP32)

#include "DisplayDriver.hpp"
#include "gui.hpp"
#include "../log/log.hpp"

int main(int argc, char *argv[]) {
  LOG_BEGIN();
  LOG_PRINTLN("EJ-Mon Desktop - Starting");

  // Init Display
  display = new Display_HDC458002C40();
  display->begin();

  gui.init(display);
  LOG_PRINTLN("EJ-Mon Desktop Started");

  // Call the rendering loop directly (no FreeRTOS tasks on desktop)
  gui_task(nullptr);

  return 0;
}
