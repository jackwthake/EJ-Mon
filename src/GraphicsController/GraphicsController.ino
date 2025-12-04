
#include "DisplayDriver.hpp"

#include "gfx_types.hpp"
#include "gfx.hpp"
#include "gui.hpp"

Display_HDC458002C40 *display = nullptr;
GUI gui;
EngineData engine_data = {
  .rpm = 3500,
  .speed = 65,
  .throttle = 45,
  .coolant_temp = 85,
  .intake_temp = 40,
  .boost_psi = 12.5f,
  .knock_count = 0,
  .timing_adv = 10,
  .engine_load = 55,
  .maf_gs = 18,
  .battery_voltage = 13.8f,
  .knock_detected = false,
  .overboost = false
};

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


void loop() {
  display->clear(display->color565(20, 0, 90));
  gui.render(display, engine_data, millis());
  display->present();
}

