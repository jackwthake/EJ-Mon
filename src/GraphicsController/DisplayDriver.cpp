#include "DisplayDriver.hpp"

#include "../log/log.hpp"

Arduino_XCA9554SWSPI *Display_HDC458002C40::expander = new Arduino_XCA9554SWSPI(
  PCA_TFT_RESET, PCA_TFT_CS, PCA_TFT_SCK, PCA_TFT_MOSI,
  &Wire, 0x3F
);

Arduino_ESP32RGBPanel *Display_HDC458002C40::rgbpanel = new Arduino_ESP32RGBPanel(
  TFT_DE, TFT_VSYNC, TFT_HSYNC, TFT_PCLK,
  TFT_R1, TFT_R2, TFT_R3, TFT_R4, TFT_R5,
  TFT_G0, TFT_G1, TFT_G2, TFT_G3, TFT_G4, TFT_G5,
  TFT_B1, TFT_B2, TFT_B3, TFT_B4, TFT_B5,

  /* hsync_polarity    */ 1,
  /* hsync_front_porch */ 10,
  /* hsync_pulse_width */ 10,
  /* hsync_back_porch  */ 30,

  /* vsync_polarity    */ 1,
  /* vsync_front_porch */ 17,
  /* vsync_pulse_width */ 2,
  /* vsync_back_porch  */ 15,

  /* pclk_active_neg */ 1,
  /* prefer_speed    */ TFT_PIXEL_CLOCK,
  /* bigEndian       */ false,

  /* de_idle_high    */ 0,
  /* pclk_idle_high  */ 0,
  /* bounce buffer   */ BOUNCE_BUF_SIZE
);

Display_HDC458002C40::Display_HDC458002C40() : Arduino_RGB_Display(
                                                  TFT_WIDTH, TFT_HEIGHT, rgbpanel, 1 /* rotation */, false /* auto_flush */,
                                                  expander, GFX_NOT_DEFINED /* RST */, 
                                                  HD458002C40_init_operations, sizeof(HD458002C40_init_operations),
                                                  TFT_COL_OFFSET /* col_offset1 */, 0 /* row_offset1 */, 0 /* col_offsetx2 */, 0 /* row_offset2 */ 
                                                ) {
  
  }

void Display_HDC458002C40::begin(void) {
  Wire.setClock(1000000); // 1MHz - fast mode plus for PCA9554
  if (!Arduino_RGB_Display::begin()) {
    LOG_PRINTLN("Display: Initialization failed!");
  }

  LOG_PRINT("Display: initialized: %dx%d\n\r", this->width(), this->height());

  this->fillScreen(RGB565_BLACK);
  expander->pinMode(PCA_TFT_BACKLIGHT, OUTPUT);
  expander->digitalWrite(PCA_TFT_BACKLIGHT, HIGH);
  LOG_PRINTLN("Display: Backlight on");

  LOG_PRINT("Allocating back buffer......  ");
  this->back_buf = (uint16_t*)heap_caps_malloc(SCREEN_BUF_WIDTH * SCREEN_BUF_HEIGHT * sizeof(uint16_t), 
                                               MALLOC_CAP_SPIRAM | MALLOC_CAP_DMA | MALLOC_CAP_8BIT);
  if (this->back_buf == nullptr) {
    LOG_PRINTLN("FAILED!");
    while (1);
  } else {
    LOG_PRINTLN("OK");
  }

  LOG_PRINT("=============- Post Display Init Memory Diag -=============\n\r");
  LOG_PRINT("\tFree heap: %u\n\r", heap_caps_get_free_size(MALLOC_CAP_DEFAULT));
  LOG_PRINT("\tFree PSRAM: %u\n\r", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
  LOG_PRINT("\tPSRAM size reported: %u\n\r", ESP.getPsramSize());
  LOG_PRINT("\tFramebuffer ptr: %p\n\r", (void*)this->getFramebuffer());
  LOG_PRINT("\tBounce buffer bytes: %u\n\r", (unsigned)BOUNCE_BUF_SIZE);
  LOG_PRINT("===========================================================\n\r\n\r");
}

void Display_HDC458002C40::present(void) {
  this->draw16bitRGBBitmap(-TFT_COL_OFFSET - 1, TFT_COL_OFFSET - 1, this->back_buf, SCREEN_BUF_WIDTH, SCREEN_BUF_HEIGHT);
}