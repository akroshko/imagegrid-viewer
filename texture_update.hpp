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
  TextureUpdate(ViewPortCurrentState *viewport_current_state);
  ~TextureUpdate()=default;
  /** Threadsafe class for getting the state of the viewport */
  ViewPortCurrentState *viewport_current_state;
  /** Find zoom index (for indexing textures) based on actual zoom. */
  int find_zoom_index(FLOAT_T zoom);
  /** Find the textures needed to render the current viewport */
  void find_current_textures (ImageGrid *grid, TextureGrid* texture_grid);
};

#endif
