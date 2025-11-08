#include "../platform.hpp"

#if PLATFORM_DESKTOP

#include "gfx_backend.hpp"

GraphicsSDL::GraphicsSDL(int w, int h) : framebuffer_(w, h) {
  SDL_Init(SDL_INIT_VIDEO);
  window_ = SDL_CreateWindow("EJ-Mon", w, h, 0);
  renderer_ = SDL_CreateRenderer(window_, nullptr);
}

GraphicsSDL::~GraphicsSDL() {
  SDL_DestroyRenderer(renderer_);
  SDL_DestroyWindow(window_);
  SDL_Quit();
}

void GraphicsSDL::present() {
  SDL_Texture* texture = SDL_CreateTexture(
    renderer_,
    SDL_PIXELFORMAT_RGB565,
    SDL_TEXTUREACCESS_STREAMING,
    framebuffer_.width(),
    framebuffer_.height()
  );
  
  SDL_UpdateTexture(texture, nullptr, framebuffer_.data(), framebuffer_.width() * sizeof(uint16_t));
  SDL_RenderClear(renderer_);
  SDL_RenderTexture(renderer_, texture, nullptr, nullptr);
  SDL_RenderPresent(renderer_);
  SDL_DestroyTexture(texture);
}

void GraphicsSDL::begin() {
  // No-op for SDL backend
}

int GraphicsSDL::get_width() const {
  return framebuffer_.width();
}

int GraphicsSDL::get_height() const {
  return framebuffer_.height();
}

uint16_t* GraphicsSDL::get_framebuffer() {
  return framebuffer_.data();
}

#endif