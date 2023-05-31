#include "config.hpp"
#include "debug.hpp"
#include "error.hpp"
#include "gridclasses.hpp"
#include "texture_update.hpp"

TextureUpdate::TextureUpdate(ViewPortCurrentState *viewport_current_state) {
  this->viewport_current_state = viewport_current_state;
}

TextureUpdate::~TextureUpdate() {

}

void TextureUpdate::find_current_textures (ImageGrid *grid, TextureGrid *texture_grid) {
  /* Finds the textures required for the current viewport. */
  int max_wpixel=texture_grid->textures_max_wpixel;
  int max_hpixel=texture_grid->textures_max_hpixel;

  float zoom, viewport_xgrid, viewport_ygrid;
  bool view_changed=this->viewport_current_state->GetGridValues(zoom,viewport_xgrid,viewport_ygrid);
  if (view_changed) {
      DEBUG("max_grid_width: " << max_wpixel);
      DEBUG("max_grid_height: " << max_hpixel);
      int zoom_index=floor(log2(1.0/zoom));
      DEBUG("zoom: " << zoom << " zoom index 1: " << zoom_index << " max: " << texture_grid->textures_max_zoom);
      if (zoom_index >= (texture_grid->textures_max_zoom - 1)) {
        zoom_index=(texture_grid->textures_max_zoom-1);
      }
      if (zoom_index < 0) {
        zoom_index=0;
      }
      DEBUG("zoom index 2: " << zoom_index);
      // load/unload appropriate textures
      int max_zoom=texture_grid->textures_max_zoom;
      int max_zoom_index=max_zoom-1;
      DEBUG("max_zoom: " << max_zoom);
      for (int z = 0; z < max_zoom; z++) {
        // only load/update current zoom, current zoom+-1 and max_zoom
        // this keeps things smoother
        if (z != max_zoom_index && z != zoom_index-1 && z !=zoom_index && z != zoom_index+1 ) {
          continue;
        }
        // calculate these with max resolution, rather than actual viewport
        float max_zoom_this_level=1.0/(2.0+z);
        float max_zoom_left=viewport_xgrid-((MAX_SCREEN_WIDTH/2)/max_wpixel/max_zoom_this_level);
        float max_zoom_right=viewport_xgrid+((MAX_SCREEN_WIDTH/2)/max_wpixel/max_zoom_this_level);
        float max_zoom_top=viewport_ygrid-((MAX_SCREEN_HEIGHT/2)/max_hpixel/max_zoom_this_level);
        float max_zoom_bottom=viewport_ygrid+((MAX_SCREEN_HEIGHT/2)/max_hpixel/max_zoom_this_level);
        float threshold=2.0;
        // find out if this is 3x3 or full
        // edge is less than 0.5*threshold away from center
        DEBUG("zoom level: " << z << " viewport_xgrid: " << viewport_xgrid << " viewport_ygrid: " << viewport_ygrid << " max_zoom_this_level: " << max_zoom_this_level << " max_zoom_left: " << max_zoom_left << " max_zoom_right: " << max_zoom_right << " max_zoom_top: " << max_zoom_top << " max_zoom_bottom: " << max_zoom_bottom);
        if (!(z == max_zoom_index) && (abs(max_zoom_left - viewport_xgrid) <= 0.5*threshold) && (abs(max_zoom_top - viewport_ygrid) <= 0.5*threshold)) {
          // can load a 3x3 grid
          DEBUG("Updating textures to 3x3 at zoom level: " << z);
          texture_grid->update_textures(grid, viewport_xgrid, viewport_ygrid, z, false);
        } else {
          // always load everthing for last zoom level
          // just make sure everything is loaded for this zoom level
          DEBUG("Updating textures to full at zoom level: " << z);
          texture_grid->update_textures(grid, viewport_xgrid, viewport_ygrid, z, true);
        }
      }
  }
}
