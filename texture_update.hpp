/**
 * Headers for updating loaded and zoomed textures.
 */
#ifndef TEXTURE_UPDATE_HPP
#define TEXTURE_UPDATE_HPP

#include "common.hpp"
#include "coordinates.hpp"
#include "imagegrid/imagegrid.hpp"
#include "texturegrid.hpp"
#include "viewport_current_state.hpp"
// C++ headers
#include <atomic>
#include <memory>

/**
 * This class updates the currently loaded textures
 */
class TextureUpdate {
public:
  TextureUpdate()=delete;
  TextureUpdate(std::shared_ptr<ViewPortTransferState> viewport_current_state_texturegrid_update);
  ~TextureUpdate()=default;
  TextureUpdate(const TextureUpdate&)=delete;
  TextureUpdate(const TextureUpdate&&)=delete;
  TextureUpdate& operator=(const TextureUpdate&)=delete;
  TextureUpdate& operator=(const TextureUpdate&&)=delete;
  /**
   * Find the textures needed to render the current viewport
   *
   * @param keeping_running Set true to stop what's happening,
   *                        generally to indicate program exit.
   */
  void find_current_textures(const ImageGrid* grid,
                             TextureGrid* texture_grid,
                             std::atomic<bool> &keep_running);
  /**
   * Load textures based on the current coordinates and zoom level.
   *
   * @param i The index along the width of the grid.
   * @param j The index along the height of the grid.
   * @param zoom The current zoom.
   * @param grid The image grid.
   * @param texture_grid The texture grid.
   * @param texture_copy_count Keeps track of numbers of textures.
   *                           copied
   * @param keeping_running Set true to stop what's happening,
   *                        generally to indicate program exit.
   */
  void load_new_textures(INT_T i,
                         INT_T j,
                         const ViewPortCurrentState& viewport_current_state,
                         const ImageGrid* const grid,
                         TextureGrid* const texture_grid,
                         INT_T &texture_copy_count,
                         std::atomic<bool> &keep_running);
  /**
   * Clear textures based on the current coordinates.
   *
   * @param i The index along the width of the grid.
   * @param j The index along the height of the grid.
   * @param texture_grid The texture grid.
   * @param keeping_running flag to stop what's happening, generally to indicate program exit
   */
  void clear_textures(INT_T i,
                      INT_T j,
                      const ViewPortCurrentState& viewport_current_state,
                      TextureGrid* const texture_grid,
                      std::atomic<bool> &keep_running);
  /**
   * Add filler textures where nothing can be loaded.
   *
   * @param i The index along the width of the grid.
   * @param j The index along the height of the grid.
   * @param texture_grid The texture grid.
   * @param keeping_runnin Set true to stop what's happening,
   *                       generally to indicate program exit.
   */
  void add_filler_textures(INT_T i,
                           INT_T j,
                           const ViewPortCurrentState& viewport_current_state,
                           TextureGrid* const texture_grid,
                           std::atomic<bool> &keep_running);
  /**
   * Load a texture.
   *
   * @param source_square The square containing the RGB data.
   * @param dest_square The destination square to load the texture into.
   * @param zoom_index The zoom index of the texture being loaded.
   * @param texture_pixel_size The original size of the image to be copied.
   * @return If texture was actually copied.
   */
  static bool load_texture(TextureGridSquareZoomLevel* dest_square,
                           const ImageGridSquareZoomLevel* source_square,
                           INT_T zoom_index,
                           GridPixelSize texture_pixel_size);
private:
  /** Threadsafe class for getting the state of the viewport */
  std::shared_ptr<ViewPortTransferState> _viewport_current_state_texturegrid_update;
  bool _grid_square_visible(INT_T i, INT_T j,
                            const ViewPortCurrentState& viewport_current_state);
};

#endif
