/**
 * Implementation of updating the loaded and zoomed textures.
 */
#include "debug.hpp"
#include "error.hpp"
#include "types.hpp"
#include "utility.hpp"
#include "coordinates.hpp"
#include "gridclasses.hpp"
#include "texture_update.hpp"

TextureUpdate::TextureUpdate(ViewPortCurrentState *viewport_current_state) {
  this->viewport_current_state = viewport_current_state;
}

int TextureUpdate::find_zoom_index(FLOAT_T zoom) {
  return ::find_zoom_index(zoom);
}

void TextureUpdate::find_current_textures (ImageGrid *grid, TextureGrid *texture_grid) {
  FLOAT_T zoom;
  GridCoordinate *viewport_grid=nullptr;
  auto view_changed=this->viewport_current_state->GetGridValues(zoom,viewport_grid);
  DEBUG("TextureUpdate::find_current_textures()");
  // if (view_changed) {
  // don't do anything here if viewport_curret_state hasn't been initialized
  if (viewport_grid != nullptr) {
    auto max_wpixel=texture_grid->max_pixel_size->wpixel();
    auto max_hpixel=texture_grid->max_pixel_size->hpixel();
    auto max_zoom=texture_grid->textures_max_zoom_index;
    auto max_zoom_index=max_zoom-1;
    auto zoom_index=this->find_zoom_index(zoom);
    DEBUG("zoom: " << zoom << " zoom index 1: " << zoom_index << " max: " << texture_grid->textures_max_zoom_index);
    if (zoom_index > max_zoom_index) {
      zoom_index=max_zoom_index;
    }
    if (zoom_index < 0) {
      zoom_index=0;
    }
    DEBUG("zoom index 2: " << zoom_index);
    DEBUG("max_zoom: " << max_zoom);
    for (INT_T z = 0; z < max_zoom; z++) {
      // only load/update current zoom, current zoom+-1 and max_zoom
      // this keeps things smoother
      if (z != max_zoom_index && z != zoom_index-1 && z !=zoom_index && z != zoom_index+1 ) {
        continue;
      }
      // calculate these with max resolution, rather than actual viewport
      auto max_zoom_this_level=1.0/(2.0+z);
      auto half_width=(MAX_SCREEN_WIDTH/2);
      auto half_height=(MAX_SCREEN_HEIGHT/2);
      auto max_zoom_left=(viewport_grid->xgrid())-(half_width/max_wpixel/max_zoom_this_level);
      // auto max_zoom_right=(viewport_grid->xgrid())+(half_width/max_wpixel/max_zoom_this_level);
      auto max_zoom_top=(viewport_grid->ygrid())-(half_height/max_hpixel/max_zoom_this_level);
      // auto max_zoom_bottom=(viewport_grid->ygrid())+(half_height/max_hpixel/max_zoom_this_level);
      auto threshold=2.0;
      // find out if this is 3x3 or full
      // edge is less than 0.5*threshold away from center
      DEBUG("zoom level: " << z << " viewport_xgrid: " << viewport_grid->xgrid() << " viewport_ygrid: " << viewport_grid->ygrid()
            << " max_zoom_this_level: " << max_zoom_this_level << " max_zoom_left: "
            << max_zoom_left // << " max_zoom_right: " << max_zoom_right
            << " max_zoom_top: " << max_zoom_top // << " max_zoom_bottom: " << max_zoom_bottom
        );
      if (!(z == max_zoom_index) && (abs(max_zoom_left - viewport_grid->xgrid()) <= 0.5*threshold) && (abs(max_zoom_top - viewport_grid->ygrid()) <= 0.5*threshold)) {
        // can load a 3x3 grid
        DEBUG("Updating textures to 3x3 at zoom level: " << z);
        texture_grid->update_textures(grid, viewport_grid, z, false);
      } else {
        // always load everthing for last zoom level
        // just make sure everything is loaded for this zoom level
        DEBUG("Updating textures to full at zoom level: " << z);
        texture_grid->update_textures(grid, viewport_grid, z, true);
      }
    }
    delete viewport_grid;
  } else {
    DEBUG("Skipping find_current_textures due to invalid viewport.");
  }
  DEBUG("TextureUpdate::find_current_textures() end");
  // }
}
