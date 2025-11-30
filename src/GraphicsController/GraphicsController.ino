
#include "DisplayDriver.hpp"

Display_HDC458002C40 *display = nullptr;

void setup(void) {
  Serial.begin(115200);

  while(!Serial);

  delay(250);
  Serial.println("Beginning");
  
  // Init Display
  display = new Display_HDC458002C40();
  display->begin();
  Serial.println("EJ-Mon ESP32 Started");
}

uint16_t hue = 0;

void loop() {
  Serial.println("Before clear");
  display->clear(display->color565(hue, 255 - hue, 128));
  hue += 2;

  Serial.println("Before rect");
  display->fill_rect(0, 0, 80, 80, Theme::C_RED);

  Serial.println("Before flush");
  display->present();
  delay(16);
}

