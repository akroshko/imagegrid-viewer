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
  ImageGridSquareZoomLevel()=delete;
  ImageGridSquareZoomLevel(INT_T zoom_level, INT_T max_zoom_level);
  ~ImageGridSquareZoomLevel();
  ImageGridSquareZoomLevel(const ImageGridSquareZoomLevel&)=delete;
  ImageGridSquareZoomLevel(const ImageGridSquareZoomLevel&&)=delete;
  ImageGridSquareZoomLevel& operator=(const ImageGridSquareZoomLevel&)=delete;
  ImageGridSquareZoomLevel& operator=(const ImageGridSquareZoomLevel&&)=delete;
  std::mutex load_mutex;
  std::atomic<bool> is_loaded{false};
  /**
   * Load a file into a square.
   *
   * @param filename The filename to load.
   *
   * @param dest_square The square to be loaded.
   */
  static bool load_file(std::string filename, std::vector<ImageGridSquareZoomLevel*> dest_square);
  /** */
  void unload_file();
  INT_T zoom_level();
  INT_T max_zoom_level();
  /** */
  size_t rgb_wpixel();
  /** */
  size_t rgb_hpixel();
  /** the actual RGB data for this square and zoom level */
  unsigned char* rgb_data=nullptr;
private:
  // TODO: these don't use an object from coordinates.hpp since they
  // are raw memory
  /** */
  size_t _rgb_wpixel;
  /** */
  size_t _rgb_hpixel;
  INT_T _zoom_level;
  INT_T _max_zoom_level;
};


/**
 * An individual square on the grid.
 */
class ImageGridSquare {
public:
  ImageGridSquare()=delete;
  ImageGridSquare(std::string filename);
  ~ImageGridSquare();
  ImageGridSquare(const ImageGridSquare&)=delete;
  ImageGridSquare(const ImageGridSquare&&)=delete;
  ImageGridSquare& operator=(const ImageGridSquare&)=delete;
  ImageGridSquare& operator=(const ImageGridSquare&&)=delete;
  std::unique_ptr<ImageGridSquareZoomLevel*[]> image_array;
  INT_T image_wpixel();
  INT_T image_hpixel();
  INT_T zoom_step_number;
private:
  INT_T _image_wpixel;
  INT_T _image_hpixel;
  void _read_file(std::string filename);
};

/**
 * The grid of images.  In the future these will be lazily loaded from
 * disk/cache.
 */
class ImageGrid {
public:
  ImageGrid()=default;
  ~ImageGrid();
  ImageGrid(const ImageGrid&)=delete;
  ImageGrid(const ImageGrid&&)=delete;
  ImageGrid& operator=(const ImageGrid&)=delete;
  ImageGrid& operator=(const ImageGrid&&)=delete;
  void read_grid_info(GridSetup *grid_setup, std::shared_ptr<ViewPortCurrentState> viewport_current_state_imagegrid_update);
  void load_grid(GridSetup *grid_setup, std::atomic<bool> &keep_running);
  GridPixelSize get_image_max_pixel_size();
  /** The individual squares in the image grid. */
  std::unique_ptr<ImageGridSquare**[]> squares;
  /**
   * Return whether read_grid_info was successful.
   */
  bool read_grid_info_successful();
  // TODO: make private once testing done
  INT_T zoom_step_number;
private:
  /**
   * Check bounds on the grid.
   */
  bool _check_bounds(INT_T i, INT_T j);
  /**
   * Figure out whether to load.
   */
  bool _check_load(INT_T k, INT_T i, INT_T j, INT_T current_load_zoom, INT_T current_grid_x, INT_T current_grid_y, INT_T load_all);
  /**
   * Actually load the file.
   */
  bool _load_file(INT_T i, INT_T j, INT_T current_grid_x, INT_T current_grid_y, INT_T load_all, GridSetup *grid_setup);
  /** Store the coordinates */
  GridImageSize grid_image_size;
  /** Maximum size of images loaded into the grid. */
  GridPixelSize image_max_size;
  INT_T zoom_step;
  /** Threadsafe class for getting the state of the viewport */
  std::shared_ptr<ViewPortCurrentState> _viewport_current_state_imagegrid_update;
  /** The locaton of the grid coordinate. */
  GridCoordinate _viewport_grid;
  /**
   * Indicate whether grid info was read properly.
   */
  bool _read_grid_info_successful=false;
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
  std::unique_ptr<TextureGridSquareZoomLevel*[]> texture_array;
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
  std::unique_ptr<TextureGridSquare*[]> squares;
  /** this size of this grid in number of textures */
  GridImageSize grid_image_size;
  /** maximum size of the individual textures in pixels */
  GridPixelSize max_pixel_size;
  /** the maximum zoom (maximum number of reductions by a factor of 2) */
  INT_T textures_max_zoom_index;
};

#endif
