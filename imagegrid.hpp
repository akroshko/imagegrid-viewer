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
  ImageGridSquareZoomLevel(INT_T zoom_level);
  ~ImageGridSquareZoomLevel();
  ImageGridSquareZoomLevel(const ImageGridSquareZoomLevel&)=delete;
  ImageGridSquareZoomLevel(const ImageGridSquareZoomLevel&&)=delete;
  ImageGridSquareZoomLevel& operator=(const ImageGridSquareZoomLevel&)=delete;
  ImageGridSquareZoomLevel& operator=(const ImageGridSquareZoomLevel&&)=delete;
  std::mutex load_mutex;
  std::atomic<bool> is_loaded{false};
  /**
   * Load a file and fill out squares.
   *
   * @param filename The filename to load.
   *
   * @param dest_square A vector of this class to be loaded.
   */
  static bool load_file(std::string filename, std::vector<ImageGridSquareZoomLevel*> dest_square);
  /** Unload and free memory from a loaded file */
  void unload_file();
  /** Accessor for the zoom level this class represents. */
  INT_T zoom_index() const;
  /** Accessor for the width in pixels. */
  size_t rgb_wpixel() const;
  /** Accessor for the height in pixels. */
  size_t rgb_hpixel() const;
  /** the actual RGB data for this square and zoom level */
  unsigned char* rgb_data=nullptr;
private:
  // these don't use an object from coordinates.hpp since they are
  // managing raw manually allocated memory
  size_t _rgb_wpixel;
  size_t _rgb_hpixel;
  INT_T _zoom_index;
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
  /** Accessor for the width in pixels. */
  INT_T image_wpixel() const;
  /** Accessor for the height in pixels. */
  INT_T image_hpixel() const;
private:
  friend class ImageGrid;
  /**
   * How many steps each zoom takes, power of 2.
   */
  INT_T _zoom_step_number;
  INT_T _image_wpixel;
  INT_T _image_hpixel;
  /**
   * Read in a the file cooresponing to this square.
   *
   * @param filename
   */
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
  void read_grid_info(const GridSetup* grid_setup,
                      std::shared_ptr<ViewPortCurrentState> viewport_current_state_imagegrid_update);
  void load_grid(const GridSetup* grid_setup, std::atomic<bool> &keep_running);
  GridPixelSize get_image_max_pixel_size() const;
  /** The individual squares in the image grid. */
  std::unique_ptr<ImageGridSquare**[]> squares;
  /**
   * Return whether read_grid_info was successful.
   */
  bool read_grid_info_successful() const;
  INT_T zoom_step_number() const;
private:
  /**
   * Check that particular indices are valid.
   *
   * @param i the index along the width of the grid.
   *
   * @param j the index along the height of the grid.
   */
  bool _check_bounds(INT_T i, INT_T j);
  /**
   * Check whether it is appropriate to load a file given current
   * viewport corrdinates and zoom level.
   *
   * @param k the zoom index to check
   *
   * @param i the index along the width of the grid.
   *
   * @param j the index along the height of the grid.
   *
   * @param current_load_zoom Specifies the current zoom level of the viewport
   *
   * @param current_grid_x the current viewport x coordinate
   *
   * @param current_grid_y the current viewport y coordinate
   *
   * @param load_all specify if all valid fils are to be loaded
   */
  bool _check_load(INT_T k, INT_T i, INT_T j,
                   INT_T current_load_zoom,
                   INT_T current_grid_x, INT_T current_grid_y,
                   INT_T load_all);
  /**
   * Actually load the file.
   *
   * @param i the index along the width of the grid.
   *
   * @param j the index along the height of the grid.
   *
   * @param zoom_index_lower_limit don't load if only things that need
   * to be loaded are below this limit
   *
   * @param current_grid_x the current viewport x coordinate
   *
   * @param current_grid_y the current viewport y coordinate
   */
  bool _load_file(INT_T i, INT_T j,
                  INT_T current_grid_x, INT_T current_grid_y,
                  INT_T zoom_index_lower_limit,
                  INT_T load_all,
                  const GridSetup* grid_setup);
  /** Store the size of the images */
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
