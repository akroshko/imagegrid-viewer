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
// C compatible headers
#include "c_compatible/buffer_manip.hpp"
// C++ headers
#include <thread>

TextureUpdate::TextureUpdate(std::shared_ptr<ViewPortCurrentState> viewport_current_state_texturegrid_update) {
  this->viewport_current_state_texturegrid_update=viewport_current_state_texturegrid_update;
}

int TextureUpdate::find_zoom_index(FLOAT_T zoom) {
  return ::find_zoom_index(zoom);
}

void TextureUpdate::find_current_textures (ImageGrid *grid, TextureGrid *texture_grid, std::atomic<bool> &keep_running) {
  FLOAT_T zoom;
  INT_T texture_copy_count=0;
  this->viewport_current_state_texturegrid_update->GetGridValues(zoom,this->viewport_grid);
  DEBUG("TextureUpdate::find_current_textures()");
  // if (view_changed) {
  // don't do anything here if viewport_current_state hasn't been initialized
  if (!this->viewport_grid.invalid()) {
    auto max_wpixel=texture_grid->max_pixel_size.wpixel();
    auto max_hpixel=texture_grid->max_pixel_size.hpixel();
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
    for (INT_T z=0l; z < max_zoom; z++) {
      if (texture_copy_count > LOAD_TEXTURES_BATCH) { continue; }
      if (!keep_running) { continue; }
      // only load/update current zoom, current zoom+-1 and max_zoom
      // this keeps things smoother
      if (z != max_zoom_index && z != zoom_index-1 && z !=zoom_index && z != zoom_index+1 ) {
        continue;
      }
      // calculate these with max resolution, rather than actual viewport
      auto max_zoom_this_level=1.0/(2.0+z);
      auto half_width=(MAX_SCREEN_WIDTH/2);
      auto half_height=(MAX_SCREEN_HEIGHT/2);
      auto max_zoom_left=(this->viewport_grid.xgrid())-(half_width/max_wpixel/max_zoom_this_level);
      // auto max_zoom_right=(viewport_grid->xgrid())+(half_width/max_wpixel/max_zoom_this_level);
      auto max_zoom_top=(this->viewport_grid.ygrid())-(half_height/max_hpixel/max_zoom_this_level);
      // auto max_zoom_bottom=(viewport_grid->ygrid())+(half_height/max_hpixel/max_zoom_this_level);
      auto threshold=2.0;
      // find out if this is 3x3 or full
      // edge is less than 0.5*threshold away from center
      DEBUG("zoom level: " << z << " viewport_xgrid: " << this->viewport_grid.xgrid() << " viewport_ygrid: " << this->viewport_grid.ygrid()
            << " max_zoom_this_level: " << max_zoom_this_level << " max_zoom_left: "
            << max_zoom_left // << " max_zoom_right: " << max_zoom_right
            << " max_zoom_top: " << max_zoom_top // << " max_zoom_bottom: " << max_zoom_bottom
        );
      auto load_all=false;
      if (!(z == max_zoom_index) &&
          (abs(max_zoom_left - this->viewport_grid.xgrid()) <= 0.5*threshold) &&
          (abs(max_zoom_top - this->viewport_grid.ygrid()) <= 0.5*threshold)) {
        // can load a 3x3 grid
        load_all=false;
        DEBUG("Updating textures to 3x3 at zoom level: " << z);
      } else {
        // always load everthing for last zoom level
        // just make sure everything is loaded for this zoom level
        load_all=true;
        DEBUG("Updating textures to full at zoom level: " << z);
      }
      this->update_textures(grid,
                            texture_grid,
                            z,
                            load_all,
                            texture_copy_count,
                            keep_running);
    }
  } else {
    DEBUG("Skipping find_current_textures due to invalid viewport.");
  }
  DEBUG("TextureUpdate::find_current_textures() end");
}

// TODO: the grid showing what's going on doesn't work in cases right now
// TextureGrid::
void TextureUpdate::update_textures(ImageGrid *grid,
                                    TextureGrid *texture_grid,
                                    INT_T zoom_level,
                                    bool load_all,
                                    INT_T &texture_copy_count,
                                    std::atomic<bool> &keep_running) {
  bool keep_going=true;
  for (INT_T j=0l; j < texture_grid->grid_image_size.himage(); j++) {
    if (!keep_going ||
        !keep_running ||
        (texture_copy_count > LOAD_TEXTURES_BATCH)) {
      keep_going=false;
      continue;
    }
    for (INT_T i=0l; i < texture_grid->grid_image_size.wimage(); i++) {
      if (!keep_going ||
          !keep_running ||
          (texture_copy_count > LOAD_TEXTURES_BATCH)) {
        keep_going=false;
        continue;
      }
      auto dest_square=texture_grid->squares[i][j].texture_array[zoom_level];
      // try and get the source square mutex
      if (!load_all &&
          ((i < next_smallest(this->viewport_grid.xgrid())) ||
           (i > next_largest(this->viewport_grid.xgrid())) ||
           (j < next_smallest(this->viewport_grid.ygrid())) ||
           (j > next_largest(this->viewport_grid.ygrid())))) {
        // skip everything if locked
        std::unique_lock<std::mutex> display_lock(dest_square->display_mutex, std::defer_lock);
        if (display_lock.try_lock()) {
          dest_square->unload_texture();
          display_lock.unlock();
        } else {
          DEBUG("Unable to aquire display_mutex for deletion in TextureGrid::update_textures");
        }
      } else {
        // auto load_index=IMAGE_GRID_BASE_INDEX;
        auto load_index=0;
        bool texture_copy_successful=false;
        do {
          auto image_square=grid->squares[i][j].image_array[load_index];
          if (image_square->is_loaded) {
            std::unique_lock<std::mutex> load_lock(image_square->load_mutex, std::defer_lock);
            if (load_lock.try_lock()) {
              std::unique_lock<std::mutex> display_lock(dest_square->display_mutex, std::defer_lock);
              if (display_lock.try_lock()) {
                texture_grid->squares[i][j].texture_pixel_size=GridPixelSize(texture_grid->max_pixel_size);
                if (dest_square->display_texture == nullptr || dest_square->last_load_index>load_index) {
                  texture_copy_successful=this->load_texture(dest_square,
                                                             image_square,
                                                             zoom_level,
                                                             texture_grid->squares[i][j].texture_pixel_size.wpixel(),
                                                             texture_grid->squares[i][j].texture_pixel_size.hpixel());
                  if (texture_copy_successful) {
                    dest_square->last_load_index=load_index;
                    texture_copy_count+=1;
                  }
                }
                display_lock.unlock();
              }
              load_lock.unlock();
            }
          }
          load_index++;
        } while (!texture_copy_successful && load_index < IMAGE_GRID_LENGTH);
      }
    }
  }
}

bool TextureUpdate::load_texture (TextureGridSquareZoomLevel *dest_square,
                                  ImageGridSquareZoomLevel *source_square,
                                  INT_T zoom_level,
                                  INT_T wpixel,
                                  INT_T hpixel) {
  auto successful=true;
  auto source_wpixel=(INT_T)source_square->rgb_wpixel;
  auto source_hpixel=(INT_T)source_square->rgb_hpixel;
  auto texture_zoom_reduction=((INT_T)pow(2,zoom_level));
  auto dest_wpixel=wpixel/texture_zoom_reduction;
  auto dest_hpixel=hpixel/texture_zoom_reduction;
  dest_wpixel=dest_wpixel + (TEXTURE_ALIGNMENT - (dest_wpixel % TEXTURE_ALIGNMENT));
  // dest_wpixel=pad(dest_wpixel,TEXTURE_ALIGNMENT);
  dest_square->display_texture=SDL_CreateRGBSurfaceWithFormat(0,dest_wpixel,dest_hpixel,24,SDL_PIXELFORMAT_RGB24);
  // skip if can't load texture
  if (dest_square->display_texture) {
    auto lock_surface_return=SDL_LockSurface(dest_square->display_texture);
    if (lock_surface_return == 0) {
      auto source_data=source_square->rgb_data;
      auto dest_array=dest_square->display_texture->pixels;
      // do the things we are copying exist?
      if (dest_array != nullptr && source_data != nullptr) {
        // these should only be powers of 2, add an assert
        auto source_zoom_level=source_square->zoom_level;
        auto dest_zoom_level=texture_zoom_reduction;
        if (source_zoom_level <= dest_zoom_level) {
          auto skip=dest_zoom_level/source_zoom_level;
          buffer_copy_reduce_generic(source_data,source_wpixel,source_hpixel,
                                     (unsigned char*)dest_array,dest_wpixel,dest_hpixel,
                                     skip);
        } else {
          // we are expanding the surface
          auto skip=source_zoom_level/dest_zoom_level;
          // loop over dest since we are expanding the source to match the dest
          for (INT_T l=0l; l < dest_hpixel; l++) {
            for (INT_T k=0l; k < dest_wpixel; k++) {
              // find appropriate source pixel
              auto ls=l / skip;
              auto ks=k / skip;
              auto dest_index=(l*dest_wpixel+k)*3;
              auto source_index=(ls*source_wpixel+ks)*3;
              if (ks < source_wpixel && ls < source_hpixel) {
                ((unsigned char *)dest_array)[dest_index]=source_data[source_index];
                ((unsigned char *)dest_array)[dest_index+1]=source_data[source_index+1];
                ((unsigned char *)dest_array)[dest_index+2]=source_data[source_index+2];
              }
            }
          }
        }
      } else {
        DEBUG("Source or dest data is null in TextureGrid::load_texture");
        if (dest_square->display_texture != nullptr) {
          SDL_UnlockSurface(dest_square->display_texture);
        }
        dest_square->unload_texture();
        successful=false;
      }
      if (dest_square->display_texture != nullptr) {
        SDL_UnlockSurface(dest_square->display_texture);
      }
    } else {
      dest_square->unload_texture();
      successful=false;
    }
  } else {
    successful=false;
  }
  return successful;
}
