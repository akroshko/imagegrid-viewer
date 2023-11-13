/**
 * Manage any textures that are in the overlay.
 */
#ifndef TEXTURE_OVERLAY_HPP
#define TEXTURE_OVERLAY_HPP

#include "common.hpp"
#include "c_sdl2/sdl2.hpp"
// C++ headers
#include <memory>
#include <mutex>
#include <string>

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
  // not currently used and not sure I need these but here because
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
  /**
   * Draw the overlay to a surface.
   *
   * @param drawable_surface The screen surface to draw on.
   */
  void draw_overlay(SDLDrawableSurface* drawable_surface);
private:
  std::unique_ptr<SDLFontTextureWrapper> _overlay_message_wrapper;
};

#endif
