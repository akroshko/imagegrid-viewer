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
#include <atomic>
#include <memory>
#include <mutex>
#include <vector>
// C headers
#include <cstddef>

class ImageGrid;
class ImageGridSquare;
class ImageGridSquareZoomLevel;

/**
 * An individual square on the grid at a particular zoom level.
 */
class ImageGridSquareZoomLevel {
public:
  ImageGridSquareZoomLevel()=delete;
  /**
   * TODO: update this doctring when I update terminology and notation
   *
   * @param parent_square The grid square this one represents a zoom level of.
   * @param zoom_level The zoom level of this grid square.
   */
  ImageGridSquareZoomLevel(ImageGridSquare* parent_square,
                           INT64 zoom_level);
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
   * @param grid_square
   * @param use_cache Whether to use the cache for initial loading.
   * @param dest_square A vector of this class to be loaded.
   * @return If loading the square was successful.
   */
  static bool load_square(ImageGridSquare* grid_square,
                          bool use_cache,
                          std::vector<ImageGridSquareZoomLevel*> dest_square);
  /** Unload and free memory from a loaded file */
  void unload_square();
  /** @return The amount zoomed out this class represents. */
  INT64 zoom_out_value() const;
  /**
   * @param subgrid_index The index of the subgrid.
   * @return The width in pixels.
   */
  size_t rgb_wpixel(SubGridIndex& subgrid_index) const;
  /**
   * @param subgrid_index The index of the subgrid.
   * @return The height in pixels.
   */
  size_t rgb_hpixel(SubGridIndex& subgrid_index) const;
  /**
   * @param subgrid_index The index of the subgrid.
   * @return The x origin coordinate in pixels.
   */
  INT64 rgb_xpixel_origin(SubGridIndex& subgrid_index) const;
  /**
   * @param subgrid_index The index of the subgrid.
   * @return The y origin coordinate in pixels.
   */
  INT64 rgb_ypixel_origin(SubGridIndex& subgrid_index) const;
  /**
   * @param subgrid_index The index of the subgrid.
   * @return A pointer to the RGB data.
   */
  unsigned char* get_rgb_data(SubGridIndex& subgrid_index) const;
  /** @return The subgrid width of this square. */
  INT64 subgrid_width() const;
  /** @return The subgrid height of this square. */
  INT64 subgrid_height() const;
  /** @return The max subgrid pixel width for each image at the zoom level of this square. */
  INT64 max_subgrid_wpixel() const;
  /** @return The max subgrid pixel height for each image at the zoom level of this square. */
  INT64 max_subgrid_hpixel() const;
private:
  friend class ImageGrid;
  friend class ImageGridSquare;
  ImageGridSquare* _parent_square;
  /** The actual RGB data for this square and zoom level */
  std::unique_ptr<std::unique_ptr<unsigned char*[]>[]> _rgb_data;
  // TOOD: will eventually use an object from coordinates.hpp, but for
  // now I want this freedom
  std::unique_ptr<std::unique_ptr<size_t[]>[]> _rgb_wpixel;
  std::unique_ptr<std::unique_ptr<size_t[]>[]> _rgb_hpixel;
  // not scaled, but they need to be here for now
  INT64 _max_subgrid_wpixel=-1;
  INT64 _max_subgrid_hpixel=-1;
  // the origin foreach
  std::unique_ptr<std::unique_ptr<INT64[]>[]> _rgb_xpixel_origin;
  std::unique_ptr<std::unique_ptr<INT64[]>[]> _rgb_ypixel_origin;
  INT64 _zoom_out_value;
};

/**
 * An individual square on the grid.
 */
class ImageGridSquare {
public:
  ImageGridSquare()=delete;
  /**
   */
  ImageGridSquare(GridSetup* grid_setup,
                  ImageGrid* parent_grid,
                  const GridIndex& grid_index);
  ~ImageGridSquare()=default;
  ImageGridSquare(const ImageGridSquare&)=delete;
  ImageGridSquare(const ImageGridSquare&&)=delete;
  ImageGridSquare& operator=(const ImageGridSquare&)=delete;
  ImageGridSquare& operator=(const ImageGridSquare&&)=delete;
  std::unique_ptr<std::unique_ptr<ImageGridSquareZoomLevel>[]> image_array;
  /** @return The subgrid width. */
  INT64 subgrid_width();
  /** @return The subgrid height. */
  INT64 subgrid_height();
  /** @return The parent image grid. */
  ImageGrid* parent_grid() const;
  /** @return The grid setup class. */
  GridSetup* grid_setup() const;
private:
  friend class ImageGrid;
  friend class  ImageGridSquareZoomLevel;
  ImageGrid* _parent_grid;
  GridSetup* _grid_setup;
  GridIndex _grid_index;
  INT64 _image_wpixel;
  INT64 _image_hpixel;
  INT64 _max_subgrid_wpixel;
  INT64 _max_subgrid_hpixel;
  /**
   * Read in a the file cooresponing to this square.
   */
  void _read_data();
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
  void read_grid_info(GridSetup* grid_setup,
                      std::shared_ptr<ViewPortTransferState> viewport_current_state_imagegrid_update);
  /**
   *
   * @param grid_setup The object holding the data on the images in
   *                   the grid, including the filenames and grid
   *                   size.
   * @param keep_running Toggled when this is shutting down.
   */
  void load_grid(const GridSetup* grid_setup, std::atomic<bool>& keep_running);
  void setup_grid_cache(GridSetup* const grid_setup);
  GridPixelSize get_image_max_pixel_size() const;
  ImageGridSquare* squares(const GridIndex& grid_index) const;
  /**
   * @return Whether read_grid_info was successful.
   */
  bool read_grid_info_successful() const;
  INT64 zoom_index_length() const;
  GridSetup* grid_setup() const;
  const GridImageSize grid_image_size();
private:
  GridSetup* _grid_setup;
  /**
   * Check that particular indices are valid.
   *
   * @param grid_index The index of grid square to check.
   */
  bool _check_bounds(const GridIndex& grid_index);
  /**
   * Check whether it is appropriate to load a file given current
   * viewport corrdinates and zoom level.
   *
   * @param viewport_current_state The current state of the viewport.
   * @param zoom_index The zoom index to check.
   * @param grid_index The index of grid square to check.
   * @param zoom_index_lower_limit The lower limit of the zoom index
   *                               for things outside adjacent grid
   *                               squares.
   * @param load_all Specify if all valid files are to be loaded.
   * @return If file should be loaded for current conditions.
   */
  bool _check_load(const ViewPortCurrentState& viewport_current_state,
                   INT64 zoom_index,
                   const GridIndex& grid_index,
                   INT64 zoom_index_lower_limit,
                   INT64 load_all);
  /**
   * Actually load the square.
   *
   * @param viewport_current_state The current state of the viewport.
   * @param grid_index The index of grid square to load.
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
                    const GridIndex& grid_index,
                    INT64 zoom_index_lower_limit,
                    INT64 load_all,
                    const GridSetup* grid_setup);
  /**
   * Actually read in the files to setup things.
   *
   * @param grid_setup The object holding the data on the images in
   *                   the grid, including the filenames and grid
   *                   size.
   */
  void _read_grid_info_setup_squares(GridSetup* const grid_setup);
  /**
   * Write out a cached copy of a file.
   *
   * @param grid_index The index of grid square to write cache for.
   */
  void _write_cache(const GridIndex& grid_index);
  /** The individual squares in the image grid. */
  std::unique_ptr<std::unique_ptr<std::unique_ptr<ImageGridSquare>[]>[]> _squares;
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
  INT64 _zoom_index_length;
};

#endif
