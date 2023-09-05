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
#include "c_compatible/sdl.hpp"
// C++ headers
#include <array>
#include <atomic>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

class TextureGrid;

/**
 * An individual square at a particular zoom level in the texture
 * grid.
 */
class TextureGridSquareZoomLevel {
public:
  TextureGridSquareZoomLevel();
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
  /**
   * Get whether this texture is a filler.
   *
   * @return Whether this texture is a filler.
   */
  bool get_image_filler () const;
  // lock when the display_area is being worked on
  std::mutex display_mutex;
  // is a real image/data loaded here
  std::atomic<bool> is_loaded{false};
  // is something loaded that makes this displayable, can be a filler texture
  std::atomic<bool> is_displayable{false};
  // TODO: this one needs help being private and investigation whether
  // there's a better way
  INT_T last_load_index=INT_MAX;
  SDLDisplayTextureWrapper* const display_texture_wrapper();
  SDLDisplayTextureWrapper* const filler_texture_wrapper();
private:
  friend class TextureGrid;
  // the actual display texture
  std::unique_ptr<SDLDisplayTextureWrapper> _display_texture_wrapper;
  // the filler texture
  SDLDisplayTextureWrapper* _filler_texture_wrapper;
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
   * @param zoom_index_length The length of the array holding
   *                          progressively zoomed out images.
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
  // TODO: may need this comment for the future
  // /**
  //  * Initialize the maximum size of each texture and the maximum zoom,
  //  * generally has to be done after all imagesquare loaded.
  //  */
  /** the individual squares */
  std::unique_ptr<TextureGridSquare**[]> squares;
  /** fillers for the individual squares. */
  std::unique_ptr<SDLDisplayTextureWrapper*[]> filler_squares;
  /** @return The size of the grid in images. */
  GridImageSize grid_image_size() const;
  /** @return The length of texture zoom array. */
  INT_T textures_zoom_index_length() const;
  /**
   * @param grid_setup The object holding the data on the images in
   *                   the grid, including the filenames and grid
   *                   size.
   * @param zoom_index_length The length of the array holding
   *                          progressively zoomed out images.
   * @param grid_pixel_size The maximum size of images in the grid.
   */
  void init_filler_squares(const GridSetup* const grid_setup,
                           INT_T zoom_index_length,
                           GridPixelSize grid_pixel_size);
private:
  /** this size of this grid in number of textures */
  GridImageSize _grid_image_size;
  /** the maximum zoom (maximum number of reductions by a factor of 2) */
  INT_T _zoom_index_length;
};

#endif
