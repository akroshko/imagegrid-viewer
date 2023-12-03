/**
 * Headers for updating loaded and zoomed textures.
 */
#ifndef TEXTURE_UPDATE_HPP
#define TEXTURE_UPDATE_HPP

#include "common.hpp"
#include "datatypes/coordinates.hpp"
#include "imagegrid/imagegrid.hpp"
#include "texture_overlay.hpp"
#include "texturegrid.hpp"
#include "viewport_current_state.hpp"
// C++ headers
#include <atomic>
#include <memory>

/**
 * This class updates the currently loaded textures in the grid.
 */
class TextureUpdate {
public:
  TextureUpdate()=delete;
  /**
   * @param The object that transfers viewport state.
   */
  TextureUpdate(std::shared_ptr<ViewPortTransferState> viewport_current_state_texturegrid_update,
                const GridPixelSize& grid_pixel_size);
  ~TextureUpdate()=default;
  TextureUpdate(const TextureUpdate&)=delete;
  TextureUpdate(const TextureUpdate&&)=delete;
  TextureUpdate& operator=(const TextureUpdate&)=delete;
  TextureUpdate& operator=(const TextureUpdate&&)=delete;
  /**
   * Find the textures needed to render the current viewport
   *
   * @param grid The image grid.
   * @param texture_grid The texture grid.
   * @param texture_overlay An overlay to use.
   * @param keeping_running Set true to stop what's happening,
   *                        generally to indicate program exit.
   */
  void find_current_textures(ImageGrid* grid,
                             TextureGrid* texture_grid,
                             TextureOverlay* texture_overlay,
                             std::atomic<bool>& keep_running);
  /**
   * Load textures based on the current coordinates and zoom level.
   *
   * @param grid_square_visible Is the current square visible?
   * @param viewport_current_state The current state of the viewport
   * @param grid_square The grid square.
   * @param texture_grid_square The texture grid square.
   * @param texture_copy_count Keeps track of numbers of textures.
   *                           copied
   * @param keeping_running Set true to stop what's happening,
   *                        generally to indicate program exit.
   */
  void load_new_textures(bool grid_square_visible,
                         const ViewPortCurrentState& viewport_current_state,
                         const ImageGridSquare* const grid_square,
                         TextureGridSquare* const texture_grid_square,
                         INT64& texture_copy_count,
                         std::atomic<bool>& keep_running);
  /**
   * Clear textures based on the current coordinates.
   *
   * @param grid_square_visible Is the current square visible?
   * @param texture_grid_square The texture grid square.
   * @param keeping_running flag to stop what's happening, generally to indicate program exit
   */
  void clear_textures(bool grid_square_visible,
                      TextureGridSquare* const texture_grid_square,
                      std::atomic<bool>& keep_running);
  /**
   * Add filler textures where nothing can be loaded.
   *
   * @param grid_square_visible Is the current square visible?
   * @param viewport_current_state The current state of the viewport.
   * @param texture_grid_square The texture grid square.
   * @param keeping_running Set true to stop what's happening,
   *                        generally to indicate program exit.
   */
  void add_filler_textures(bool grid_square_visible,
                           const ViewPortCurrentState& viewport_current_state,
                           TextureGridSquare* const texture_grid_square,
                           std::atomic<bool>& keep_running);
  /**
   * Load a texture.
   *
   * @param dest_square The destination square to load the texture into.
   * @param source_square The square containing the RGBA data.
   * @param zoom_out_shift The right shift that represents the zoom out of the texture being loaded.
   * @return If texture was actually copied.
   */
  static bool load_texture(TextureGridSquareZoomLevel* dest_square,
                           ImageGridSquareZoomLevel* source_square,
                           INT64 zoom_out_shift,
                           INT64* const row_buffer);
  std::unique_ptr <INT64[]> row_buffer_temp;
private:
  /** Threadsafe class for getting the state of the viewport */
  std::shared_ptr<ViewPortTransferState> _viewport_current_state_texturegrid_update;
  bool _grid_square_visible(const GridIndex& grid_index,
                            const ViewPortCurrentState& viewport_current_state);
};

#endif
