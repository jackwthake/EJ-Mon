
#include "DisplayDriver.hpp"

#include "gfx_types.hpp"
#include "gfx.hpp"
#include "gui.hpp"

Display_HDC458002C40 *display = nullptr;
GUI gui;
EngineData engine_data = {};

void setup(void) {
  Serial.begin(115200);

  while(!Serial);

  delay(250);
  Serial.println("Beginning");
  
  // Init Display
  display = new Display_HDC458002C40();
  display->begin();

  gui.init(display);
  Serial.println("EJ-Mon ESP32 Started");
}

uint16_t hue = 0;

void loop() {
  display->clear(display->color565(hue, 255 - hue, 128));
  gui.render(display, engine_data, millis() / 1000.0f);
  display->present();
}

