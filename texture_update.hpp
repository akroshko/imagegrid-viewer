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
#include "gridclasses.hpp"


/**
 * This class updates the currently loaded textures
 */
class TextureUpdate {
public:
  TextureUpdate()=delete;
  TextureUpdate(ViewPortCurrentState *viewport_current_state_texturegrid_update);
  ~TextureUpdate()=default;
  TextureUpdate(const TextureUpdate&)=delete;
  TextureUpdate(const TextureUpdate&&)=delete;
  TextureUpdate& operator=(const TextureUpdate&)=delete;
  TextureUpdate& operator=(const TextureUpdate&&)=delete;
  /** Threadsafe class for getting the state of the viewport */
  ViewPortCurrentState *viewport_current_state_texturegrid_update;
  /** Find zoom index (for indexing textures) based on actual zoom. */
  int find_zoom_index(FLOAT_T zoom);
  /** Find the textures needed to render the current viewport */
  void find_current_textures(ImageGrid *grid, TextureGrid* texture_grid);
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
   * @param loadall load all textures at this zoom level, otherwise a 3x3 is loaded
   */
  void update_textures(ImageGrid *grid,
                       TextureGrid* texture_grid,
                       INT_T zoom_level,
                       bool load_all);
  /**
   * Load a texture.
   *
   * @param source_square the square containing the RGB data
   *
   * @param dest_square the destination square to load the texture into
   *
   * @param zoom_level
   */
  bool load_texture(TextureGridSquareZoomLevel *dest_square,
                    ImageGridSquareZoomLevel *source_square,
                    INT_T zoom_level,
                    INT_T wpixel,
                    INT_T hpixel);
  GridCoordinate viewport_grid;
};

#endif
