#include "DisplayDriver.hpp"

Arduino_XCA9554SWSPI *Display_HDC458002C40::expander = new Arduino_XCA9554SWSPI(
  PCA_TFT_RESET, PCA_TFT_CS, PCA_TFT_SCK, PCA_TFT_MOSI,
  &Wire, 0x3F
);

Arduino_ESP32RGBPanel *Display_HDC458002C40::rgbpanel = new Arduino_ESP32RGBPanel(
  TFT_DE, TFT_VSYNC, TFT_HSYNC, TFT_PCLK,
  TFT_R1, TFT_R2, TFT_R3, TFT_R4, TFT_R5,
  TFT_G0, TFT_G1, TFT_G2, TFT_G3, TFT_G4, TFT_G5,
  TFT_B1, TFT_B2, TFT_B3, TFT_B4, TFT_B5,

  /* hsync_polarity */ 1,
  /* hsync_front_porch */ 22,
  /* hsync_pulse_width */ 8,
  /* hsync_back_porch */ 30,

  /* vsync_polarity */ 1,
  /* vsync_front_porch */ 8,
  /* vsync_pulse_width */ 4,
  /* vsync_back_porch */ 10,

  /* pclk_idle */ 0,
  /* pixel clock */ TFT_PIXEL_CLOCK,
  /* bigEndian */ false,
  /* pclk_active_neg */ 0, // latch on rising edge
  /* pref speed */ 0,
  /* bounce bytes */ BOUNCE_BUF_SIZE
);

static void rotate_copy(const uint16_t* src_buffer, uint16_t* dst_buffer) {

    // Constants 
    const int SRC_W = Display_HDC458002C40::SCREEN_BUF_WIDTH;  // 960 (Use the correct constant name)
    const int SRC_H = Display_HDC458002C40::SCREEN_BUF_HEIGHT; // 320
    const int DST_W = Display_HDC458002C40::SCREEN_BUF_HEIGHT; // 320 (Width and height are swapped for 90-degree rotation)

    // Iterate through Source (x_src, y_src)
    for (int y_src = 0; y_src < SRC_H; ++y_src) {
        for (int x_src = 0; x_src < SRC_W; ++x_src) {

            // --- 90-Degree Counter-Clockwise (Left) Rotation ---
            // x_dst = (SRC_H - 1) - y_src  (0 to 319)
            // y_dst = x_src                (0 to 959)

            // Calculate 1D Indices
            size_t src_index = (size_t)y_src * SRC_W + x_src;

            // I_dst = y_dst * DST_STRIDE + x_dst
            // Replace DST_W with dst_stride
            size_t dst_index = (size_t)x_src * DST_W + ((SRC_H - 1) - y_src); 

            dst_buffer[dst_index] = src_buffer[src_index];
        }
    }
}

Display_HDC458002C40::Display_HDC458002C40() : Arduino_RGB_Display(
                                                  TFT_WIDTH, TFT_HEIGHT, rgbpanel, 0 /* rotation */, false /* auto_flush */,
                                                  expander, GFX_NOT_DEFINED /* RST */, 
                                                  HD458002C40_init_operations, sizeof(HD458002C40_init_operations),
                                                  80 /* col_offset1 */, 0 /* row_offset1 */, 0 /* col_offsetx2 */, 0 /* row_offset2 */ 
                                                ) {
  
  }

void Display_HDC458002C40::begin(void) {
  Wire.setClock(1000000); // 1MHz - fast mode plus for PCA9554
  if (!Arduino_RGB_Display::begin()) {
    Serial.println("Display: Initialization failed!");
  }

  Serial.printf("Display: initialized: %dx%d\n", this->width(), this->height());

  this->fillScreen(RGB565_BLACK);
  expander->pinMode(PCA_TFT_BACKLIGHT, OUTPUT);
  expander->digitalWrite(PCA_TFT_BACKLIGHT, HIGH);
  Serial.println("Display: Backlight on");

  Serial.printf("Allocating back buffer......  ");
  this->back_buf = (uint16_t*)ps_malloc(SCREEN_BUF_WIDTH * SCREEN_BUF_HEIGHT * sizeof(uint16_t));
  if (this->back_buf == nullptr) {
    Serial.println("FAILED!");
    while (1);
  } else {
    Serial.println("OK");
  }

  Serial.printf("=============- Post Display Init Memory Diag -=============\n");
  Serial.printf("Free heap: %u\n", heap_caps_get_free_size(MALLOC_CAP_DEFAULT));
  Serial.printf("Free PSRAM: %u\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
  Serial.printf("PSRAM size reported: %u\n", ESP.getPsramSize());
  Serial.printf("Framebuffer ptr: %p\n", (void*)this->getFramebuffer());
  Serial.printf("Bounce buffer bytes: %u\n", (unsigned)BOUNCE_BUF_SIZE);
}

void Display_HDC458002C40::present(void) {
  // Iterate through half the height to swap the top line with the bottom line
  // for (int i = 0; i < SCREEN_BUF_HEIGHT / 2; ++i) {
  //   uint16_t* top_line_ptr = back_buf + (i * SCREEN_BUF_WIDTH);
    
  //   uint16_t* bottom_line_ptr = back_buf + ((SCREEN_BUF_HEIGHT - 1 - i) * SCREEN_BUF_WIDTH);
    
  //   // Swap the entire range (line) of 'SCREEN_BUF_WIDTH' elements
  //   std::swap_ranges(
  //     top_line_ptr,                       // Start of the top line
  //     top_line_ptr + SCREEN_BUF_WIDTH,    // End of the top line (exclusive)
  //     bottom_line_ptr                     // Start of the bottom line
  //   );
  // }
  
  this->draw16bitRGBBitmap(0, 0, this->back_buf, TFT_WIDTH, TFT_HEIGHT);
  Arduino_RGB_Display::flush();
}