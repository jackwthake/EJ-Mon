#include <Arduino_GFX_Library.h>
#include <Adafruit_FT6206.h>
#include <Adafruit_CST8XX.h>

#include "platform.hpp"

Arduino_XCA9554SWSPI *expander = new Arduino_XCA9554SWSPI(
  PCA_TFT_RESET, PCA_TFT_CS, PCA_TFT_SCK, PCA_TFT_MOSI,
  &Wire, 0x3F
);

Arduino_ESP32RGBPanel *rgbpanel = new Arduino_ESP32RGBPanel(
  TFT_DE, TFT_VSYNC, TFT_HSYNC, TFT_PCLK,
  TFT_R1, TFT_R2, TFT_R3, TFT_R4, TFT_R5,
  TFT_G0, TFT_G1, TFT_G2, TFT_G3, TFT_G4, TFT_G5,
  TFT_B1, TFT_B2, TFT_B3, TFT_B4, TFT_B5,
  1 /* hsync_polarity */, 10 /* hsync_front_porch */, 10 /* hsync_pulse_width */, 50 /* hsync_back_porch */,
  1 /* vsync_polarity */, 15 /* vsync_front_porch */, 2 /* vsync_pulse_width */, 17 /* vsync_back_porch */,
  1 /* pclk_active_neg */, GFX_NOT_DEFINED /* auto speed */, false /* use big endian */,
  0 /* de_idle_high */, 0 /* pclk_idle_high */
);

Arduino_RGB_Display *gfx = new Arduino_RGB_Display(
  DISPLAY_WIDTH, DISPLAY_HEIGHT, rgbpanel, 0 /* rotation */, true /* auto_flush */,
  expander, GFX_NOT_DEFINED /* RST */, HD458002C40_init_operations, sizeof(HD458002C40_init_operations),
  80 /* col_offset1 */, 0 /* row_offset1 */, 8 /* col_offset2 */, 0 /* row_offset2 */
);

void setup() {
  // Enable backlight
  expander->pinMode(PCA_TFT_BACKLIGHT, OUTPUT);
  expander->digitalWrite(PCA_TFT_BACKLIGHT, HIGH);

  Wire.setClock(1000000); // speed up I2C
  gfx->begin();
  gfx->fillScreen(BLACK);
  gfx->setCursor(20, 20);
  gfx->setTextColor(WHITE);
  gfx->setTextSize(3);
  gfx->println("RGB-666 DISPLAY OK!");
}

uint16_t hue = 0;

void loop() {
  // Smooth color cycle instead of random jumps
  gfx->fillScreen(gfx->color565(hue, 255 - hue, 128));
  hue += 2;
  delay(16); // ~60fps
}
