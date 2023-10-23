#include "common.hpp"
#include "texture_overlay.hpp"
// C++ headers
#include <string>
// C headers
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

// TODO: move sdl into it's own wrapper
TextureOverlay::TextureOverlay () {
  TTF_Init();
  this->_sdl_current_font = TTF_OpenFont(OVERLAY_FONT_PATH, 24);
  this->_sdl_font_color = {255, 255, 255, 0};
}

TextureOverlay::~TextureOverlay () {
  if (this->_overlay_message_surface != nullptr) {
    SDL_FreeSurface(this->_overlay_message_surface);
  }
  if (this->_sdl_current_font != nullptr) {
    TTF_CloseFont(this->_sdl_current_font);
  }
  TTF_Quit();
}

void TextureOverlay::update_overlay(std::string overlay_message) {
  if (this->_overlay_message_surface != nullptr) {
    SDL_FreeSurface(this->_overlay_message_surface);
  }
  this->_overlay_message_surface=TTF_RenderText_Solid(this->_sdl_current_font,
                                              overlay_message.c_str(),
                                              this->_sdl_font_color);
}

SDL_Surface* TextureOverlay::overlay_message_surface() {
  return this->_overlay_message_surface;
}
