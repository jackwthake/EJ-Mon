
#include "DisplayDriver.hpp"

#include "gfx_types.hpp"
#include "gui.hpp"

#include "../log/log.hpp"

// Platform-specific timing helpers
#ifdef PLATFORM_ESP32
  #include <sys/time.h>
  #define get_millis() millis()
#else // PLATFORM_DESKTOP
  #include <chrono>
  #include <thread>
  
  static auto start_time = std::chrono::high_resolution_clock::now();
  
  inline uint32_t millis() {
    auto elapsed = std::chrono::high_resolution_clock::now() - start_time;
    return std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
  }
  
  inline void delay(uint32_t ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
  }
  
  #define get_millis() millis()
#endif

Display_HDC458002C40 *display = nullptr;
GUI gui;
EngineData engine_data = {
  .rpm = 3500,
  .throttle = 45,
  .coolant_temp = 85,
  .intake_temp = 40,
  .boost_psi = 12.5f,
  .knock_count = 0,
  .timing_adv = 10,
};

void gui_task(void *param) {
  LOG_PRINTLN("Success!");
  
  constexpr uint32_t TARGET_FPS = 60;
  constexpr uint32_t FRAME_TIME_MS = 1000 / TARGET_FPS;  // 16ms per frame
  
  for (;;) {
    uint32_t frame_start = get_millis();
    
    gui.render(display, engine_data, frame_start / 1000.0f);

    static float t = 0.0f;
    t += 0.02f;
    engine_data.rpm = 3000 + (int)(1500 * (sinf(t) + 1.0f) / 2.0f);
    engine_data.boost_psi = 10.0f + 10.0f * (cosf(t * 1.5f) + 1.0f) / 2.0f;
    engine_data.intake_temp = 30 + (int)(40 * (sinf(t * 0.8f) + 1.0f) / 2.0f);
    engine_data.coolant_temp = 70 + (int)(30 * (sinf(t * 0.5f + 1.0f) / 2.0f));
    
    // Frame rate limiting: cap at TARGET_FPS
    uint32_t frame_elapsed = get_millis() - frame_start;
    if (frame_elapsed < FRAME_TIME_MS) {
      delay(FRAME_TIME_MS - frame_elapsed);
    }
  }
}

void can_ingest_task(void *param) {
  LOG_PRINTLN("Success!");

  for (;;) {
    // unimplemented
    delay(500);
    // LOG_PRINTLN("CAN Ingest Task Heartbeat"); 
  }
}

#ifdef PLATFORM_ESP32

void setup(void) {
  LOG_BEGIN();
  LOG_PRINTLN("Beginning");

  // Init Display
  display = new Display_HDC458002C40();
  display->begin();

  gui.init(display);
  LOG_PRINTLN("EJ-Mon ESP32 Started");

  LOG_PRINT("Starting CAN Ingest Task on APP CPU...\t");
  BaseType_t res = xTaskCreatePinnedToCore(
    can_ingest_task,
    "can_ingest_task", 
    8192,         // stack size
    nullptr,      // parameter
    1,            // normal priority
    nullptr,      // handle
    0             // APP_CPU
  );

  if (res != pdPASS) {
    LOG_PANIC("Failed to create CAN Ingest Task!");
  }

  delay(100);

  LOG_PRINT("Starting GUI Task on PRO CPU...\t");
  res = xTaskCreatePinnedToCore(
    gui_task,
    "gui_task", 
    8192,         // stack size
    nullptr,      // parameter
    2,            // high priority
    nullptr,      // handle
    1             // PRO_CPU
  );

  if (res != pdPASS) {
    LOG_PANIC("Failed to create GUI Task!");
  }
}

void loop() { }

