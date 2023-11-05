/**
 * Manage any textures that are in the overlay.
 */
#ifndef TEXTURE_OVERLAY_HPP
#define TEXTURE_OVERLAY_HPP

#include "common.hpp"
#include "c_sdl/sdl.hpp"
// C++ headers
#include <string>
#include <mutex>
// C headers
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

/**
 * This class holds the textures in the overlay.
 */
class TextureOverlay {
public:
  TextureOverlay();
  ~TextureOverlay();
  TextureOverlay(const TextureOverlay&)=delete;
  TextureOverlay(const TextureOverlay&&)=delete;
  TextureOverlay& operator=(const TextureOverlay&)=delete;
  TextureOverlay& operator=(const TextureOverlay&&)=delete;
  // lock when the display_area is being worked on
  std::mutex display_mutex;
  // not currently used and nt sure I need these but here because
  // TextureGridSquareZoomLevel uses them
  //
  // is a real image/data loaded here
  // std::atomic<bool> is_loaded{false};
  // is something loaded that makes this displayable, can be a filler texture
  // std::atomic<bool> is_displayable{false};
  /**
   * Update the overlay.
   *
   * @param overlay_message The message to include in the overlay.
   */
  void update_overlay(std::string overlay_message);
  /** @return The SDL surface. */
  SDL_Surface* overlay_message_surface();
private:
  SDL_Surface* _overlay_message_surface=nullptr;
  TTF_Font* _sdl_current_font=nullptr;
  SDL_Color _sdl_font_color;
};

#endif
