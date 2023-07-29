/**
 * Implementation of updating the loaded and zoomed textures.
 */
#include "debug.hpp"
#include "error.hpp"
#include "types.hpp"
#include "utility.hpp"
#include "coordinates.hpp"
#include "imagegrid.hpp"
#include "texturegrid.hpp"
#include "texture_update.hpp"
// C compatible headers
#include "c_compatible/buffer_manip.hpp"
// C++ headers
#include <thread>

TextureUpdate::TextureUpdate(std::shared_ptr<ViewPortCurrentState> viewport_current_state_texturegrid_update) {
  this->_viewport_current_state_texturegrid_update=viewport_current_state_texturegrid_update;
}

void TextureUpdate::find_current_textures (const ImageGrid* const grid,
                                           TextureGrid* const texture_grid,
                                           std::atomic<bool> &keep_running) {
  FLOAT_T zoom;
  INT_T texture_copy_count=0;
  this->_viewport_current_state_texturegrid_update->GetGridValues(zoom,this->_viewport_grid);
  // if (view_changed) {
  // don't do anything here if viewport_current_state hasn't been initialized
  if (!this->_viewport_grid.invalid()) {
    auto max_zoom=texture_grid->textures_max_zoom_index();
    auto zoom_index=ViewPortCurrentState::find_zoom_index(zoom);
    if (zoom_index >= max_zoom) {
      zoom_index=max_zoom-1;
    }
    if (zoom_index < 0) {
      zoom_index=0;
    }
    auto max_zoom_index=max_zoom-1;
    if (max_zoom_index < 0) {
      max_zoom_index=0;
    }
    // TODO get rid of this for loop, unload seperately
    for (INT_T z=max_zoom_index; z >= 0l; z--) {
      if (!keep_running || texture_copy_count >= LOAD_TEXTURES_BATCH) { break; }
      // only load/update current zoom and max_zoom
      // TODO: this doesn't help with unloading
      if (z != max_zoom_index && z != zoom_index) {
        continue;
      }
      // calculate these with max resolution, rather than actual viewport
      // always load everything for last zoom level
      auto load_all=(z == max_zoom_index);
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
}

bool TextureUpdate::_grid_square_visible(INT_T i, INT_T j,
                                         const TextureGrid* const texture_grid,
                                         INT_T zoom_index) {
  auto max_wpixel=(FLOAT_T)texture_grid->max_pixel_size().wpixel();
  auto max_hpixel=(FLOAT_T)texture_grid->max_pixel_size().hpixel();
  auto xgrid=this->_viewport_grid.xgrid();
  auto ygrid=this->_viewport_grid.ygrid();
  FLOAT_T xgrid_left_max_zoom_index=ViewPortCurrentState::find_leftmost_visible(
    xgrid,ygrid,max_wpixel,max_hpixel,zoom_index);
  FLOAT_T xgrid_right_max_zoom_index=ViewPortCurrentState::find_rightmost_visible(
    xgrid,ygrid,max_wpixel,max_hpixel,zoom_index);
  FLOAT_T ygrid_top_max_zoom_index=ViewPortCurrentState::find_topmost_visible(
    xgrid,ygrid,max_wpixel,max_hpixel,zoom_index);
  FLOAT_T ygrid_bottom_max_zoom_index=ViewPortCurrentState::find_bottommost_visible(
    xgrid,ygrid,max_wpixel,max_hpixel,zoom_index);
  auto return_value=!((i < floor(xgrid_left_max_zoom_index)) ||
                      (i > floor(xgrid_right_max_zoom_index)) ||
                      (j < floor(ygrid_top_max_zoom_index)) ||
                      (j > floor(ygrid_bottom_max_zoom_index)));
  return return_value;
}

void TextureUpdate::update_textures(const ImageGrid* const grid,
                                    TextureGrid* const texture_grid,
                                    INT_T zoom_index,
                                    bool load_all,
                                    INT_T &texture_copy_count,
                                    std::atomic<bool> &keep_running) {
  // separated out deleting to make sure it always happens
  // TODO: make sure the one above is always available before deleting
  for (INT_T j=0l; j < texture_grid->grid_image_size().himage(); j++) {
    for (INT_T i=0l; i < texture_grid->grid_image_size().wimage(); i++) {
      if (!keep_running) {break;}
      auto dest_square=texture_grid->squares[i][j].texture_array[zoom_index];
      if (!load_all &&
          !_grid_square_visible(i,j,texture_grid,zoom_index)) {
        if (dest_square->is_loaded) {
          // skip everything if locked
          std::unique_lock<std::mutex> display_lock(dest_square->display_mutex, std::defer_lock);
          if (display_lock.try_lock()) {
            dest_square->unload_texture();
            dest_square->is_loaded=false;
            dest_square->last_load_index=INT_MAX;
            display_lock.unlock();
          } else {
            DEBUG("Unable to aquire display_mutex for deletion in TextureGrid::update_textures");
          }
        }
      }
    }
  }
  for (INT_T j=0l; j < texture_grid->grid_image_size().himage(); j++) {
    for (INT_T i=0l; i < texture_grid->grid_image_size().wimage(); i++) {
      if (!keep_running ||
          (texture_copy_count >= LOAD_TEXTURES_BATCH)) {
        break;
      }
      auto dest_square=texture_grid->squares[i][j].texture_array[zoom_index];
      // try and get the source square mutex
      if (load_all || _grid_square_visible(i,j,texture_grid,zoom_index)) {
        auto load_index=0;
        bool texture_copy_successful=false;
        do {
          auto image_square=grid->squares[i][j]->image_array[load_index];
          if (image_square->is_loaded || !dest_square->is_loaded) {
            std::unique_lock<std::mutex> load_lock(image_square->load_mutex, std::defer_lock);
            if (load_lock.try_lock()) {
              std::unique_lock<std::mutex> display_lock(dest_square->display_mutex, std::defer_lock);
              if (display_lock.try_lock()) {
                texture_grid->squares[i][j].texture_pixel_size=GridPixelSize(texture_grid->max_pixel_size());
                if (dest_square->display_texture == nullptr || dest_square->last_load_index>load_index) {
                  texture_copy_successful=this->load_texture(dest_square,
                                                             image_square,
                                                             zoom_index,
                                                             texture_grid->squares[i][j].texture_pixel_size);
                  if (texture_copy_successful) {
                    dest_square->last_load_index=load_index;
                    dest_square->is_loaded=true;
                    texture_copy_count+=1;
                  }
                }
                display_lock.unlock();
              }
              load_lock.unlock();
            }
          }
          load_index++;
        } while (!texture_copy_successful && load_index < grid->zoom_step_number());
      }
    }
  }
}

bool TextureUpdate::load_texture (TextureGridSquareZoomLevel* const dest_square,
                                  const ImageGridSquareZoomLevel* const source_square,
                                  INT_T zoom_index,
                                  GridPixelSize texture_pixel_size) {
  auto successful=true;
  auto source_wpixel=(INT_T)source_square->rgb_wpixel();
  auto source_hpixel=(INT_T)source_square->rgb_hpixel();
  auto texture_zoom_reduction=((INT_T)pow(2,zoom_index));
  auto dest_wpixel=texture_pixel_size.wpixel()/texture_zoom_reduction;
  auto dest_hpixel=texture_pixel_size.hpixel()/texture_zoom_reduction;
  dest_wpixel=dest_wpixel + (TEXTURE_ALIGNMENT - (dest_wpixel % TEXTURE_ALIGNMENT));
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
        auto source_zoom_index=source_square->zoom_index();
        auto dest_zoom_index=texture_zoom_reduction;
        if (source_zoom_index <= dest_zoom_index) {
          auto skip=dest_zoom_index/source_zoom_index;
          buffer_copy_reduce_generic(source_data,source_wpixel,source_hpixel,
                                     (unsigned char*)dest_array,dest_wpixel,dest_hpixel,
                                     skip);
        } else {
          // we are expanding the surface
          auto skip=source_zoom_index/dest_zoom_index;
          // loop over dest since we are expanding the source to match the dest
          for (INT_T l=0l; l < dest_hpixel; l++) {
            for (INT_T k=0l; k < dest_wpixel; k++) {
              // find appropriate source pixel
              auto ls=l/skip;
              auto ks=k/skip;
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
