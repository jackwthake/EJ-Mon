#include "DisplayDriver.hpp"

#include "../log/log.hpp"

#ifdef PLATFORM_ESP32

// ============================================================================
// ESP32-S3 Platform Implementation
// ============================================================================

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

Display_HDC458002C40::~Display_HDC458002C40() {
  // No cleanup needed on ESP32 (static objects managed by Arduino framework)
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

#else // PLATFORM_DESKTOP

// ============================================================================
// Desktop (SDL3) Platform Implementation
// ============================================================================

Display_HDC458002C40::Display_HDC458002C40() 
  : sdl_window(nullptr), sdl_surface(nullptr), back_buf(nullptr) {
}

Display_HDC458002C40::~Display_HDC458002C40() {
  if (sdl_surface) {
    SDL_DestroySurface(sdl_surface);
    sdl_surface = nullptr;
  }
  if (sdl_window) {
    SDL_DestroyWindow(sdl_window);
    sdl_window = nullptr;
  }
  SDL_Quit();
}

void Display_HDC458002C40::begin(void) {
  LOG_PRINTLN("Display: Initializing SDL3 window...");

  // Initialize SDL with video support
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    LOG_PANIC("SDL_Init failed: %s", SDL_GetError());
  }

  // Create window (960x320)
  // Note: SDL3 shows windows by default, so no need for SDL_WINDOW_SHOWN
  sdl_window = SDL_CreateWindow(
    "EJ-Mon Desktop Display",
    SCREEN_BUF_WIDTH,
    SCREEN_BUF_HEIGHT,
    0  // flags (window is shown by default in SDL3)
  );

  if (!sdl_window) {
    LOG_PANIC("SDL_CreateWindow failed: %s", SDL_GetError());
  }

  LOG_PRINTLN("Display: SDL window created (960x320)");

  // Allocate framebuffer (RGB565 format)
  LOG_PRINT("Allocating back buffer......  ");
  back_buf = (uint16_t*)malloc(SCREEN_BUF_WIDTH * SCREEN_BUF_HEIGHT * sizeof(uint16_t));
  
  if (back_buf == nullptr) {
    LOG_PRINTLN("FAILED!");
    LOG_PANIC("Framebuffer allocation failed");
  } else {
    LOG_PRINTLN("OK");
  }

  // Create SDL surface from framebuffer
  // SDL_PIXELFORMAT_RGB565 matches our RGB565 color format
  // Signature: SDL_CreateSurfaceFrom(width, height, format, pixels, pitch)
  sdl_surface = SDL_CreateSurfaceFrom(
    SCREEN_BUF_WIDTH,
    SCREEN_BUF_HEIGHT,
    SDL_PIXELFORMAT_RGB565,
    back_buf,
    SCREEN_BUF_WIDTH * (int)sizeof(uint16_t)
  );

  if (!sdl_surface) {
    LOG_PANIC("SDL_CreateSurfaceFrom failed: %s", SDL_GetError());
  }

  LOG_PRINTLN("Display: Framebuffer initialized (RGB565)");
  
  // Clear framebuffer to black
  clear(Theme::C_BLACK);
  present();
}

void Display_HDC458002C40::present(void) {
  // Get the window surface and update it with our framebuffer
  if (sdl_window) {
    SDL_Surface *window_surface = SDL_GetWindowSurface(sdl_window);
    if (window_surface) {
      // Copy our framebuffer directly to the window surface
      SDL_Rect src_rect = { 0, 0, SCREEN_BUF_WIDTH, SCREEN_BUF_HEIGHT };
      SDL_Rect dst_rect = { 0, 0, SCREEN_BUF_WIDTH, SCREEN_BUF_HEIGHT };
      SDL_BlitSurface(sdl_surface, &src_rect, window_surface, &dst_rect);
      
      // Update the window
      if (!SDL_UpdateWindowSurface(sdl_window)) {
        LOG_PRINT("SDL_UpdateWindowSurface failed: %s\n", SDL_GetError());
      }
    }
  }
  
  // Handle window events
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_EVENT_QUIT:
      case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        LOG_PRINTLN("Window close requested, exiting...");
        exit(0);
        break;
      default:
        break;
    }
  }
}

#endif // PLATFORM_*

// Clear the framebuffer (optimized with 4-pixel unrolled loop)
void Display_HDC458002C40::clear(Color color) {
  uint16_t* fb = getFramebuffer();
  int total_pixels = get_width() * get_height();
  int aligned_count = (total_pixels / 4) * 4;  // Process in 4-pixel chunks
  
  // Unrolled loop: write 4 pixels per iteration
  // This reduces branch checks and improves instruction-level parallelism
  for (int i = 0; i < aligned_count; i += 4) {
    fb[i + 0] = color.value;
    fb[i + 1] = color.value;
    fb[i + 2] = color.value;
    fb[i + 3] = color.value;
  }
  
  // Handle remaining pixels (0-3)
  for (int i = aligned_count; i < total_pixels; i++) {
    fb[i] = color.value;
  }
}

// Fill a rectangle
void Display_HDC458002C40::fill_rect(int x, int y, int w, int h, Color color) {
  uint16_t* fb = getFramebuffer();
  int screen_w = get_width();
  int screen_h = get_height();

  for (int j = 0; j < h; j++) {
    int py = y + j;
    if (py < 0 || py >= screen_h) continue;

    for (int i = 0; i < w; i++) {
      int px = x + i;
      if (px < 0 || px >= screen_w) continue;

      fb[py * SCREEN_BUF_WIDTH + px] = color.value;
    }
  }
}