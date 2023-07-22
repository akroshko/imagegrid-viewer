/**
 * Header file for the main classes representing the grid.  Includes
 * both loaded images and (zoomed) textures.
 */
#ifndef IMAGEGRID_HPP
#define IMAGEGRID_HPP

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

class ImageGrid;

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
private:
  friend class ImageGrid;
  /**
   * How many steps each zoom takes, power of 2.
   */
  INT_T _zoom_step_number;
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
  INT_T zoom_step_number();
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
  GridImageSize _grid_image_size;
  /** Maximum size of images loaded into the grid. */
  GridPixelSize _image_max_size;
  /** Threadsafe class for getting the state of the viewport */
  std::shared_ptr<ViewPortCurrentState> _viewport_current_state_imagegrid_update;
  /** The locaton of the grid coordinate. */
  GridCoordinate _viewport_grid;
  /**
   * Indicate whether grid info was read properly.
   */
  bool _read_grid_info_successful=false;
  /**
   * How many steps each zoom takes, power of 2.
   */
  INT_T _zoom_step_number;
};

#endif
