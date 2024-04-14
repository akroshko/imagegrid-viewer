#include "common.hpp"
#include "texture_overlay.hpp"
// C++ headers
#include <string>

TextureOverlay::TextureOverlay () {
  this->_overlay_message_wrapper=std::make_unique<SDLFontTextureWrapper>();
}

TextureOverlay::~TextureOverlay () {
}

void TextureOverlay::update_overlay(const std::string& overlay_message) {
  this->_overlay_message_wrapper->update_text(overlay_message);
}

void TextureOverlay::draw_overlay(SDLDrawableSurface* drawable_surface) {
  this->_overlay_message_wrapper->draw_text(drawable_surface,OVERLAY_X,OVERLAY_Y);
}
