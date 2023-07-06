/**
 * Header file for the main classes representing the grid.  Includes
 * both loaded images and (zoomed) textures.
 */
#ifndef GRIDCLASSES_HPP
#define GRIDCLASSES_HPP

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
 * An individual square on the grid at a particular zoom level.
 *
 * TODO: Currently only stores one zoom level.  However code is in the
 * works that improves this.
 */
class ImageGridSquareZoomLevel {
public:
  ImageGridSquareZoomLevel()=default;
  ~ImageGridSquareZoomLevel();
  ImageGridSquareZoomLevel(const ImageGridSquareZoomLevel&)=delete;
  ImageGridSquareZoomLevel(const ImageGridSquareZoomLevel&&)=delete;
  ImageGridSquareZoomLevel& operator=(const ImageGridSquareZoomLevel&)=delete;
  ImageGridSquareZoomLevel& operator=(const ImageGridSquareZoomLevel&&)=delete;
  std::mutex load_mutex;
  std::atomic<bool> is_loaded{false};
  /**
   * Load a file into this square.
   *
   * @param The filename to load.
   */
  bool load_file(std::string filename);
  // TODO: these don't use an object from coordinates.hpp since they
  // are raw memory
  /** */
  void unload_file();
  size_t rgb_wpixel;
  /** */
  size_t rgb_hpixel;
  // the actual RGB data
  unsigned char* rgb_data=nullptr;
  INT_T zoom_level;
  INT_T max_zoom_level;
};


/**
 * An individual square on the grid.
 */
class ImageGridSquare {
public:
  ImageGridSquare();
  ~ImageGridSquare();
  ImageGridSquare(const ImageGridSquare&)=delete;
  ImageGridSquare(const ImageGridSquare&&)=delete;
  ImageGridSquare& operator=(const ImageGridSquare&)=delete;
  ImageGridSquare& operator=(const ImageGridSquare&&)=delete;
  void read_file(std::string filename);
  std::unique_ptr<ImageGridSquareZoomLevel*[]> image_array=nullptr;
  // std::unique_ptr<std::array<ImageGridSquareZoomLevel,<unsigned>>> image_array;
  // std::unique_ptr<std::array<std::unique_ptr<ImageGridSquareZoomLevel>>> image_array=nullptr;
  INT_T image_wpixel;
  INT_T image_hpixel;
};

/**
 * Iterate over an ImageGrid.  Generally in an optimal pattern
 */
class ImageGridIterator {
public:
  ImageGridIterator()=delete;
  ImageGridIterator(INT_T w, INT_T h, INT_T current_grid_x, INT_T current_grid_y);
  ~ImageGridIterator()=default;
  bool get_next(INT_T &k, INT_T &i, INT_T &j);
private:
  INT_T _w;
  INT_T _h;
  INT_T _x_current;
  INT_T _y_current;
  std::queue<std::array<INT_T,3>> _index_values;
};

/**
 * The grid of images.  In the future these will be lazily loaded from
 * disk/cache.
 */
class ImageGrid {
public:
  ImageGrid()=delete;
  ImageGrid(GridSetup *grid_setup, std::shared_ptr<ViewPortCurrentState> viewport_current_state_imagegrid_update);
  ~ImageGrid();
  ImageGrid(const ImageGrid&)=delete;
  ImageGrid(const ImageGrid&&)=delete;
  ImageGrid& operator=(const ImageGrid&)=delete;
  ImageGrid& operator=(const ImageGrid&&)=delete;
  bool read_grid_info(GridSetup *grid_setup);
  void load_grid(GridSetup *grid_setup, std::atomic<bool> &keep_running);
  GridPixelSize get_image_max_pixel_size();
  /** The individual squares in the image grid. */
  std::unique_ptr<ImageGridSquare*[]> squares=nullptr;
private:
  /**
   * Check bounds on the grid.
   */
  bool _check_bounds(INT_T k, INT_T i, INT_T j);
  /**
   * Figure out whether to load.
   */
  bool _check_load(INT_T k, INT_T i, INT_T j, INT_T current_load_zoom, INT_T current_grid_x, INT_T current_grid_y, INT_T load_all);
  /**
   * Actually load the file.
   */
  bool _load_file(INT_T k, INT_T i, INT_T j, INT_T current_grid_x, INT_T current_grid_y, INT_T load_all, GridSetup *grid_setup);
  /** Store the coordinates */
  GridImageSize grid_image_size;
  /** Maximum size of images loaded into the grid. */
  GridPixelSize image_max_size;
  // /** Maximum size of secondary images loaded into the grid. */
  // GridPixelSize image_second_max_size;
  // /** Maximum size of thumbnail images loaded into the grid. */
  // GridPixelSize image_thumbnail_max_size;
  INT_T zoom_step;
  /** Threadsafe class for getting the state of the viewport */
  std::shared_ptr<ViewPortCurrentState> _viewport_current_state_imagegrid_update=nullptr;
  /** The locaton of the grid coordinate. */
  GridCoordinate _viewport_grid;
};

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
  SDL_Surface* display_texture=nullptr;
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
  std::unique_ptr<TextureGridSquareZoomLevel*[]> texture_array=nullptr;
  // TODO: does not work for now because elements of texture_array contain a mutex
  //       this can be improved
  // std::array<TextureGridSquareZoomLevel*, 10> texture_array;
  /** the size of the texture with no zoom */
  GridPixelSize texture_pixel_size;
};

/**
 * The grid as textures.  These are generally loaded lazily.
 */
class TextureGrid {
public:
  TextureGrid()=delete;
  TextureGrid(GridSetup *grid_setup);
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
  std::unique_ptr<TextureGridSquare*[]> squares=nullptr;
  /** this size of this grid in number of textures */
  GridImageSize grid_image_size;
  /** maximum size of the individual textures in pixels */
  GridPixelSize max_pixel_size;
  /** the maximum zoom (maximum number of reductions by a factor of 2) */
  INT_T textures_max_zoom_index;
};

#endif
