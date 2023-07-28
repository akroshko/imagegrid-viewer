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
  void unload_texture();
  // lock when the display_area is being worked on
  std::mutex display_mutex;
  std::atomic<bool> is_loaded{false};
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
  TextureGridSquare();
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
};

/**
 * The grid as textures.  These are generally loaded lazily.
 */
class TextureGrid {
public:
  TextureGrid()=delete;
  TextureGrid(const GridSetup* grid_setup);
  ~TextureGrid();
  TextureGrid(const TextureGrid&)=delete;
  TextureGrid(const TextureGrid&&)=delete;
  TextureGrid& operator=(const TextureGrid&)=delete;
  TextureGrid& operator=(const TextureGrid&&)=delete;
  /**
   * Initialize the maximum size of each texture and the maximum zoom,
   * generally has to be done after all imagesare loaded.
   */
  void init_max_zoom_index(const GridPixelSize &image_max_pixel_size);
  /** the indidivual squares */
  std::unique_ptr<TextureGridSquare*[]> squares;
  GridImageSize grid_image_size() const;
  GridPixelSize max_pixel_size() const;
  INT_T textures_max_zoom_index() const;
private:
  /** this size of this grid in number of textures */
  GridImageSize _grid_image_size;
  /** maximum size of the individual textures in pixels */
  GridPixelSize _max_pixel_size;
  /** the maximum zoom (maximum number of reductions by a factor of 2) */
  INT_T _textures_max_zoom_index;
};

#endif
