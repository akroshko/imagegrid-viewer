#ifndef TEXTURE_UPDATE_HPP

#define TEXTURE_UPDATE_HPP

#include "config.hpp"
#include "debug.hpp"
#include "error.hpp"
#include "viewport.hpp"
#include "gridclasses.hpp"
#include "texture_update.hpp"

class TextureUpdate {
  /* This class updates the currently loaded textures */
public:
  TextureUpdate(ViewPortCurrentState *viewport_current_state);
  ~TextureUpdate();

  // threadsafe way of getting the state of the viewport
  ViewPortCurrentState *viewport_current_state;

  // find zoom index
  int find_zoom_index(float zoom);

  // find the textures for the current viewport
  void find_current_textures (ImageGrid *grid, TextureGrid* texture_grid);
};

#endif
