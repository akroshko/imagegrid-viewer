/**
 * Headers for updating loaded and zoomed textures.
 */
#ifndef TEXTURE_UPDATE_HPP
#define TEXTURE_UPDATE_HPP

#include "debug.hpp"
#include "error.hpp"
#include "types.hpp"
#include "viewport.hpp"
#include "coordinates.hpp"
#include "imagegrid.hpp"
#include "texturegrid.hpp"
// C++ headers
#include <atomic>

/**
 * This class updates the currently loaded textures
 */
class TextureUpdate {
public:
  TextureUpdate()=delete;
  TextureUpdate(std::shared_ptr<ViewPortCurrentState> viewport_current_state_texturegrid_update);
  ~TextureUpdate()=default;
  TextureUpdate(const TextureUpdate&)=delete;
  TextureUpdate(const TextureUpdate&&)=delete;
  TextureUpdate& operator=(const TextureUpdate&)=delete;
  TextureUpdate& operator=(const TextureUpdate&&)=delete;
  /** Find zoom index (for indexing textures) based on actual zoom. */
  int find_zoom_index(FLOAT_T zoom);
  /**
   * Find the textures needed to render the current viewport
   *
   * @param keeping_running flag to stop what's happening, generally to indicate program exit
   */
  void find_current_textures(ImageGrid *grid, TextureGrid* texture_grid, std::atomic<bool> &keep_running);
  /**
   * Update the textures based on the current coordinates and zoom
   * level.
   *
   * @param grid the image grid
   *
   * @param xgrid the x coordinate on the grid
   *
   * @param ygrid the y coordinate on the grid
   *
   * @param zoom_index The index of the current zoom.
   *
   * @param loadall load all textures at this zoom index, otherwise a 3x3 is loaded
   *
   * @param texture_copy_count keep track of the number of textures copied
   *
   * @param keeping_running flag to stop what's happening, generally to indicate program exit
   */
  void update_textures(ImageGrid *grid,
                       TextureGrid* texture_grid,
                       INT_T zoom_index,
                       bool load_all,
                       INT_T &texture_copy_count,
                       std::atomic<bool> &keep_running);
  /**
   * Load a texture.
   *
   * @param source_square the square containing the RGB data
   *
   * @param dest_square the destination square to load the texture into
   *
   * @param zoom_index the zoom index of the texture being loaded
   *
   * @return if texture was actually copied
   */
  bool load_texture(TextureGridSquareZoomLevel *dest_square,
                    ImageGridSquareZoomLevel *source_square,
                    INT_T zoom_index,
                    GridPixelSize texture_pixel_size);
private:
  /** Threadsafe class for getting the state of the viewport */
  std::shared_ptr<ViewPortCurrentState> _viewport_current_state_texturegrid_update;
  GridCoordinate _viewport_grid;
  bool _grid_square_visible(INT_T i, INT_T j, TextureGrid *texture_grid, INT_T zoom_index);
};

#endif
