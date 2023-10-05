/**
 * Header file for the main classes representing the grid.  Includes
 * both loaded images and (zoomed) textures.
 */
#ifndef IMAGEGRID_HPP
#define IMAGEGRID_HPP

#include "../common.hpp"
#include "../coordinates.hpp"
#include "gridsetup.hpp"
#include "../viewport_current_state.hpp"
// C++ headers
#include <array>
#include <atomic>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

class ImageGrid;

/**
 * An individual square on the grid at a particular zoom level.
 */
class ImageGridSquareZoomLevel {
public:
  ImageGridSquareZoomLevel()=delete;
  /**
   * TODO: update this doctring when I update terminology and notation
   *
   * @param zoom_level
   * @param max_subgrid_wpixel
   * @param max_subgrid_hpixel
   * @param subgrid_height
   * @param subgrid_width
   */
  ImageGridSquareZoomLevel(INT_T zoom_level,
                           INT_T max_subgrid_wpixel,
                           INT_T max_subgrid_hpixel,
                           INT_T subgrid_height,
                           INT_T subgrid_width);
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
   * @param square_data The data for this grid square.
   * @param use_cache Whether to use the cache for initial loading.
   * @param dest_square A vector of this class to be loaded.
   * @return If loading the square was successful.
   */
  static bool load_square(SQUARE_DATA_T square_data,
                          bool use_cache,
                          std::vector<ImageGridSquareZoomLevel*> dest_square);
  /**
   * Create the cached filename from the real filename.
   *
   * @param The filename to use to create the cached filename.
   * @return The cached filename.
   */
  static std::string create_cache_filename(std::string filename);
  /** Unload and free memory from a loaded file */
  void unload_square();
  /** @return The amount zoomed out this class represents. */
  INT_T zoom_out_value() const;
  /**
   * @param w_index
   * @param h_index
   * @return The width in pixels.
   */
  size_t rgb_wpixel(INT_T w_index, INT_T h_index) const;
  /**
   * @param w_index
   * @param h_index
   * @return The height in pixels.
   */
  size_t rgb_hpixel(INT_T w_index, INT_T h_index) const;
  /** @return The number of subgrid images in width. */
  size_t w_subgrid() const;
  /** @return The number of subgrid images in height. */
  size_t h_subgrid() const;
  /**
   * @param w_index
   * @param h_index
   * @return The x origin coordinate in pixels.
   */
  INT_T rgb_xpixel_origin(INT_T w_index, INT_T h_index) const;
  /**
   * @param w_index
   * @param h_index
   * @return The y origin coordinate in pixels.
   */
  INT_T rgb_ypixel_origin(INT_T w_index, INT_T h_index) const;
  /**
   * @param w_index
   * @param h_index
   * @return A pointer to the RGB data.
   */
  unsigned char* get_rgb_data(INT_T w_index, INT_T h_index) const;
private:
  friend class ImageGrid;
  /** The actual RGB data for this square and zoom level */
  std::unique_ptr<std::unique_ptr<unsigned char*[]>[]> rgb_data;
  // TOOD: will eventually use an object from coordinates.hpp, but for
  // now I want this freedom
  std::unique_ptr<std::unique_ptr<size_t[]>[]> _rgb_wpixel;
  std::unique_ptr<std::unique_ptr<size_t[]>[]> _rgb_hpixel;
  // TODO:  currently unused???
  size_t _w_subgrid=-1;
  size_t _h_subgrid=-1;
  // not scaled, but they need to be here for now
  INT_T _max_subgrid_wpixel=-1;
  INT_T _max_subgrid_hpixel=-1;
  // the origin foreach
  std::unique_ptr<std::unique_ptr<INT_T[]>[]> _rgb_xpixel_origin;
  std::unique_ptr<std::unique_ptr<INT_T[]>[]> _rgb_ypixel_origin;
  INT_T _zoom_out_value;
};

/**
 * An individual square on the grid.
 */
class ImageGridSquare {
public:
  ImageGridSquare()=delete;
  /**
   * @param square_data The data for this grid square.
   */
  ImageGridSquare(SQUARE_DATA_T square_data);
  ~ImageGridSquare()=default;
  ImageGridSquare(const ImageGridSquare&)=delete;
  ImageGridSquare(const ImageGridSquare&&)=delete;
  ImageGridSquare& operator=(const ImageGridSquare&)=delete;
  ImageGridSquare& operator=(const ImageGridSquare&&)=delete;
  std::unique_ptr<std::unique_ptr<ImageGridSquareZoomLevel>[]> image_array;
private:
  friend class ImageGrid;
  /**
   * How many steps each zoom takes, power of 2.
   */
  INT_T _zoom_step_number;
  INT_T _image_wpixel;
  INT_T _image_hpixel;
  INT_T _subgrid_width;
  INT_T _subgrid_height;
  INT_T _max_subgrid_wpixel;
  INT_T _max_subgrid_hpixel;

  /**
   * Read in a the file cooresponing to this square.
   *
   * @param square_data The data on this grid square.
   */
  void _read_data(SQUARE_DATA_T square_data);
};

/**
 * The grid of images.  In the future these will be lazily loaded from
 * disk/cache.
 */
class ImageGrid {
public:
  ImageGrid()=default;
  ~ImageGrid()=default;
  ImageGrid(const ImageGrid&)=delete;
  ImageGrid(const ImageGrid&&)=delete;
  ImageGrid& operator=(const ImageGrid&)=delete;
  ImageGrid& operator=(const ImageGrid&&)=delete;
  /**
   * @param grid_setup The object holding the data on the images in
   *                   the grid, including the filenames and grid
   *                   size.
   * @param viewport_current_state_imagegrid_update The current state of the viewport.
   */
  void read_grid_info(const GridSetup* grid_setup,
                      std::shared_ptr<ViewPortTransferState> viewport_current_state_imagegrid_update);
  /**
   *
   * @param grid_setup The object holding the data on the images in
   *                   the grid, including the filenames and grid
   *                   size.
   * @param keep_running Toggled when this is shutting down.
   */
  void load_grid(const GridSetup* grid_setup, std::atomic<bool> &keep_running);
  void setup_grid_cache(const GridSetup* grid_setup);
  GridPixelSize get_image_max_pixel_size() const;
  /** The individual squares in the image grid. */
  std::unique_ptr<std::unique_ptr<std::unique_ptr<ImageGridSquare>[]>[]> squares;
  /**
   * @return Whether read_grid_info was successful.
   */
  bool read_grid_info_successful() const;
  INT_T zoom_index_length() const;
private:
  /**
   * Check that particular indices are valid.
   *
   * @param i The index along the width of the grid.
   * @param j The index along the height of the grid.
   */
  bool _check_bounds(INT_T i, INT_T j);
  /**
   * Check whether it is appropriate to load a file given current
   * viewport corrdinates and zoom level.
   *
   * @param viewport_current_state The current state of the viewport.
   * @param zoom_index The zoom index to check.
   * @param i The index along the width of the grid.
   * @param j The index along the height of the grid.
   * @param zoom_index_lower_limit The lower limit of the zoom index
   *                               for things outside adjacent grid
   *                               squares.
   * @param load_all Specify if all valid files are to be loaded.
   * @return If file should be loaded for current conditions.
   */
  bool _check_load(const ViewPortCurrentState& viewport_current_state,
                   INT_T zoom_index, INT_T i, INT_T j,
                   INT_T zoom_index_lower_limit,
                   INT_T load_all);
  /**
   * Actually load the square.
   *
   * @param viewport_current_state The current state of the viewport.
   * @param i The index along the width of the grid.
   * @param j The index along the height of the grid.
   * @param zoom_index_lower_limit Do not load if only things that
   *                               need to be loaded are below this
   *                               limit
   * @param load_all specify if all valid files are to be loaded
   * @param grid_setup The object holding the data on the images in
   *                   the grid, including the filenames and grid
   *                   size.
   * @return If the file was loaded.
   */
  bool _load_square(const ViewPortCurrentState& viewport_current_state,
                  INT_T i, INT_T j,
                  INT_T zoom_index_lower_limit,
                  INT_T load_all,
                  const GridSetup* grid_setup);
  /**
   * Actually read in the files to setup things.
   *
   * @param grid_setup The object holding the data on the images in
   *                   the grid, including the filenames and grid
   *                   size.
   */
  void _read_grid_info_setup_squares(const GridSetup* const grid_setup);
  /**
   * Write out a cached copy of a file.
   *
   * @param i The index along the width of the grid.
   * @param j The index along the height of the grid.
   * @param square_data The data for this grid square.
   */
  void _write_cache(INT_T i, INT_T j, SQUARE_DATA_T square_data);
  /** Store the size of the images */
  GridImageSize _grid_image_size;
  /** Maximum size of images loaded into the grid. */
  GridPixelSize _image_max_size;
  /** Threadsafe class for getting the state of the viewport */
  std::shared_ptr<ViewPortTransferState> _viewport_current_state_imagegrid_update;
  /**
   * Indicate whether grid info was read properly.
   */
  bool _read_grid_info_successful=false;
  /**
   * How many steps each zoom takes, power of 2.
   */
  INT_T _zoom_index_length;
};

#endif
