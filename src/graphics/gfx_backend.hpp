#pragma once

#include "gfx.hpp"
#include "gfx_types.hpp"
#include "framebuffer.hpp"

#if PLATFORM_DESKTOP

#include <SDL3/SDL.h>

class GraphicsSDL : public Graphics {
private:
  SDL_Window* window_;
  SDL_Renderer* renderer_;
  Framebuffer framebuffer_;
  
public:
  GraphicsSDL(int w, int h);
  ~GraphicsSDL();

  void begin() override;
  void present() override;

  int get_width() const override;
  int get_height() const override;
  uint16_t* get_framebuffer() override;
};

#elif PLATFORM_ESP32

// UNIMPLEMENTED - Placeholder for ESP32 graphics backend
class GraphicsESP32 : public Graphics {
private:
  Framebuffer framebuffer_;

public:
  GraphicsESP32(int w, int h);

  void present() override;

  uint16_t* get_framebuffer() override {
    return framebuffer_.data();
  }
};

#endif