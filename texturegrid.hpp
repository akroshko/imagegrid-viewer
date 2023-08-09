/**
 * Header file for the main classes representing the grid of textures.
 */
#ifndef TEXTUREGRID_HPP
#define TEXTUREGRID_HPP

#include "debug.hpp"
#include "error.hpp"
#include "types.hpp"
#include "defaults.hpp"
#include "coordinates.hpp"
#include "gridsetup.hpp"
#include "viewport_current_state.hpp"

// C++ headers
#include <array>
#include <atomic>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

// library headers
#include <SDL2/SDL.h>

/**
 * An individual square at a particular zoom level in the texture
 * grid.
 */
class TextureGridSquareZoomLevel {
public:
  TextureGridSquareZoomLevel()=default;
  ~TextureGridSquareZoomLevel();
  TextureGridSquareZoomLevel(const TextureGridSquareZoomLevel&)=delete;
  TextureGridSquareZoomLevel(const TextureGridSquareZoomLevel&&)=delete;
  TextureGridSquareZoomLevel& operator=(const TextureGridSquareZoomLevel&)=delete;
  TextureGridSquareZoomLevel& operator=(const TextureGridSquareZoomLevel&&)=delete;
  /**
   * Unload the texture for this square.  Also resets any state.
   */
  void unload_texture();
  /**
   * Set this texture as a loaded image/data.
   *
   * @param load_index The last loaded index representing the zoom
   *                   level of what was loaded.
   */
  void set_image_loaded (INT_T load_index);
  /**
   * Set this texture as a filler.
   */
  void set_image_filler ();
  // lock when the display_area is being worked on
  std::mutex display_mutex;
  // is a real image/data loaded here
  std::atomic<bool> is_loaded{false};
  // is something loaded that makes this displayable, can be a filler texture
  std::atomic<bool> is_displayable{false};
  SDL_Surface* display_texture=nullptr;
  // TODO: this one needs help being private and investigation whether
  // there's a better way
  INT_T last_load_index=INT_MAX;
};

/**
 * An individual square in the texture grid.
 */
class TextureGridSquare {
public:
  TextureGridSquare(INT_T zoom_index_length);
  ~TextureGridSquare();
  TextureGridSquare(const TextureGridSquare&)=delete;
  TextureGridSquare(const TextureGridSquare&&)=delete;
  TextureGridSquare& operator=(const TextureGridSquare&)=delete;
  TextureGridSquare& operator=(const TextureGridSquare&&)=delete;
  /**
   * An array of textures. The first element of the array is the
   * full-size texture the subsequent elements are zoomed textures
   * each reduced by a factor of 2.
   */
  std::unique_ptr<TextureGridSquareZoomLevel*[]> texture_array;
  // TODO: this one needs help being private
  /** the size of the texture with no zoom */
  GridPixelSize texture_pixel_size;
private:
  INT_T _zoom_index_length;
};

/**
 * The grid as textures.  These are generally loaded lazily.
 */
class TextureGrid {
public:
  TextureGrid()=delete;
  /**
   *
   * @param grid_setup The object holding the data on the images in
   *                   the grid, including the filenames and grid
   *                   size.
   *
   * @param zoom_index_length The length of the array holding
   *                          progressively zoomed out images.
   *
   * @param image_max_pixel_size The maximum size in pixels of the
   *                             images on the grid.
   */
  TextureGrid(const GridSetup* grid_setup,
              INT_T zoom_index_length);
  ~TextureGrid();
  TextureGrid(const TextureGrid&)=delete;
  TextureGrid(const TextureGrid&&)=delete;
  TextureGrid& operator=(const TextureGrid&)=delete;
  TextureGrid& operator=(const TextureGrid&&)=delete;
  /**
   * Initialize the maximum size of each texture and the maximum zoom,
   * generally has to be done after all imagesquare loaded.
   */
  /** the individual squares */
  std::unique_ptr<TextureGridSquare**[]> squares;
  /** @return The size of the grid in images. */
  GridImageSize grid_image_size() const;
  /** @return The length of texture zoom array. */
  INT_T textures_zoom_index_length() const;
private:
  /** this size of this grid in number of textures */
  GridImageSize _grid_image_size;
  /** the maximum zoom (maximum number of reductions by a factor of 2) */
  INT_T _zoom_index_length;
};

#endif
