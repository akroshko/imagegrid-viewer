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
#include "viewport_current_state.hpp"
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
    for (INT_T j=0l; j < texture_grid->grid_image_size().himage(); j++) {
      for (INT_T i=0l; i < texture_grid->grid_image_size().wimage(); i++) {
        this->load_new_textures(i,j,zoom,grid,texture_grid,texture_copy_count,keep_running);
        this->clear_textures(i,j,texture_grid,keep_running);
        this->add_filler_textures(i,j,texture_grid,keep_running);
      }
    }
  }
}

void TextureUpdate::load_new_textures(INT_T i,
                                      INT_T j,
                                      FLOAT_T zoom,
                                      const ImageGrid* const grid,
                                      TextureGrid* const texture_grid,
                                      INT_T &texture_copy_count,
                                      std::atomic<bool> &keep_running) {
  auto max_zoom_index=texture_grid->textures_zoom_index_length()-1;
  // auto current_zoom_index=ViewPortCurrentState::find_zoom_index_bounded(zoom,0,max_zoom_index);
  for (INT_T zoom_index=0; zoom_index <= max_zoom_index; zoom_index++) {
    if (!keep_running ||
        (texture_copy_count >= LOAD_TEXTURES_BATCH)) {
      break;
    }
    // only load/update current zoom and max_zoom
    // TODO: want this to be initial, but then fill out textures in background
    // if (zoom_index != max_zoom_index && zoom_index != current_zoom_index) {
    //   continue;
    // }
    auto upper_zoom=ViewPortCurrentState::find_zoom_upper(zoom_index);
    auto dest_square=texture_grid->squares[i][j]->texture_array[zoom_index];
    auto load_all=(zoom_index == max_zoom_index);
    if (load_all || _grid_square_visible(i,j,texture_grid,upper_zoom)) {
      auto load_index=zoom_index;
      bool texture_copy_successful=false;
      do {
        auto image_square=grid->squares[i][j]->image_array[load_index];
        if (image_square->is_loaded &&
            (!dest_square->is_loaded ||
             dest_square->last_load_index>load_index)) {
          std::unique_lock<std::mutex> load_lock(image_square->load_mutex, std::defer_lock);
          if (load_lock.try_lock()) {
            std::unique_lock<std::mutex> display_lock(dest_square->display_mutex, std::defer_lock);
            if (display_lock.try_lock()) {
              // try same condition again after all locks aquired
              if (image_square->is_loaded &&
                  (!dest_square->is_loaded ||
                   dest_square->last_load_index>load_index)) {
                // TODO: this line needs to be changed, I don't like doing this here
                texture_grid->squares[i][j]->texture_pixel_size=GridPixelSize(texture_grid->max_pixel_size());
                texture_copy_successful=this->load_texture(dest_square,
                                                           image_square,
                                                           zoom_index,
                                                           texture_grid->squares[i][j]->texture_pixel_size);
                if (texture_copy_successful) {
                  dest_square->set_image_loaded(load_index);
                  texture_copy_count+=1;
                }
              }
              display_lock.unlock();
            }
            load_lock.unlock();
          }
        }
        load_index++;
      } while (!texture_copy_successful && load_index < grid->zoom_index_length());
    }
  }
}

void TextureUpdate::clear_textures(INT_T i,
                                   INT_T j,
                                   TextureGrid* const texture_grid,
                                   std::atomic<bool> &keep_running) {
  auto max_zoom_index=texture_grid->textures_zoom_index_length()-1;
  // never clear out top level index
  for (INT_T zoom_index=0l; zoom_index < max_zoom_index-1; zoom_index++) {
    if (!keep_running) { break; }
    auto upper_zoom=ViewPortCurrentState::find_zoom_upper(zoom_index);
    auto dest_square=texture_grid->squares[i][j]->texture_array[zoom_index];
    if (!_grid_square_visible(i,j,texture_grid,upper_zoom)) {
      // unload anything not visible that is loadable or displayable
      if (dest_square->is_loaded || dest_square->is_displayable) {
        std::unique_lock<std::mutex> display_lock(dest_square->display_mutex, std::defer_lock);
        if (display_lock.try_lock()) {
          dest_square->unload_texture();
          display_lock.unlock();
        }
      }
    }
  }
}

void TextureUpdate::add_filler_textures(INT_T i,
                                  INT_T j,
                                  TextureGrid* const texture_grid,
                                  std::atomic<bool> &keep_running) {
  auto max_zoom_index=texture_grid->textures_zoom_index_length()-1;
  for (INT_T zoom_index=max_zoom_index; zoom_index >= 0l; zoom_index--) {
    if (!keep_running) { break; }
    auto upper_zoom=ViewPortCurrentState::find_zoom_upper(zoom_index);
    auto load_all=(zoom_index == max_zoom_index);
    auto dest_square=texture_grid->squares[i][j]->texture_array[zoom_index];
    if (load_all || _grid_square_visible(i,j,texture_grid,upper_zoom)) {
      // try to copy over filler if texture not sucessful
      bool load_filler_successful=false;
      if (!dest_square->is_displayable) {
        std::unique_lock<std::mutex> display_lock(dest_square->display_mutex, std::defer_lock);
        if (display_lock.try_lock()) {
          if (!dest_square->is_displayable) {
            // TODO: this line needs to be changed, I don't like doing this here
            texture_grid->squares[i][j]->texture_pixel_size=GridPixelSize(texture_grid->max_pixel_size());
            load_filler_successful=this->load_filler(dest_square,
                                                     zoom_index,
                                                     texture_grid->squares[i][j]->texture_pixel_size);
            if (load_filler_successful) {
              dest_square->set_image_filler();
            }
          }
          display_lock.unlock();
        }
      }
    }
  }
}


bool TextureUpdate::_grid_square_visible(INT_T i, INT_T j,
                                         const TextureGrid* const texture_grid,
                                         FLOAT_T zoom) {
  auto max_wpixel=texture_grid->max_pixel_size().wpixel();
  auto max_hpixel=texture_grid->max_pixel_size().hpixel();
  auto xgrid=this->_viewport_grid.xgrid();
  auto ygrid=this->_viewport_grid.ygrid();
  auto return_value=(ViewPortCurrentState::grid_index_visible(i, j,
                                                              xgrid, ygrid,
                                                              max_wpixel, max_hpixel,
                                                              zoom) ||
                     (i >= floor(xgrid)-1 && i <= floor(xgrid)+1 &&
                      j >= floor(ygrid)-1 && j <= floor(ygrid)+1));
  return return_value;
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
  if (dest_square->display_texture != nullptr) {
    dest_square->unload_texture();
  }
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
        auto source_zoom_out_value=source_square->zoom_out_value();
        auto dest_zoom_index=texture_zoom_reduction;
        if (source_zoom_out_value <= dest_zoom_index) {
          auto skip=dest_zoom_index/source_zoom_out_value;
          buffer_copy_reduce_generic(source_data,source_wpixel,source_hpixel,
                                     (unsigned char*)dest_array,dest_wpixel,dest_hpixel,
                                     skip);
        } else {
          // we are expanding the surface
          auto skip=source_zoom_out_value/dest_zoom_index;
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

bool TextureUpdate::load_filler(TextureGridSquareZoomLevel* const dest_square,
                                INT_T zoom_index,
                                GridPixelSize texture_pixel_size) {
  auto successful=true;
  auto texture_zoom_reduction=((INT_T)pow(2,zoom_index));
  auto dest_wpixel=texture_pixel_size.wpixel()/texture_zoom_reduction;
  auto dest_hpixel=texture_pixel_size.hpixel()/texture_zoom_reduction;
  dest_wpixel=dest_wpixel + (TEXTURE_ALIGNMENT - (dest_wpixel % TEXTURE_ALIGNMENT));
  if (dest_square->display_texture != nullptr) {
    dest_square->unload_texture();
  }
  dest_square->display_texture=SDL_CreateRGBSurfaceWithFormat(0,dest_wpixel,dest_hpixel,24,SDL_PIXELFORMAT_RGB24);
  if (dest_square->display_texture) {
    auto lock_surface_return=SDL_LockSurface(dest_square->display_texture);
    if (lock_surface_return == 0) {
      auto dest_array=dest_square->display_texture->pixels;
      // do the things we are copying exist?
      if (dest_array != nullptr) {
        // copy over gray
        for (INT_T l=0l; l < dest_hpixel; l++) {
          for (INT_T k=0l; k < dest_wpixel; k++) {
            auto dest_index=(l*dest_wpixel+k)*3;
            ((unsigned char *)dest_array)[dest_index]=FILLER_LEVEL;
            ((unsigned char *)dest_array)[dest_index+1]=FILLER_LEVEL;
            ((unsigned char *)dest_array)[dest_index+2]=FILLER_LEVEL;
          }
        }
      } else {
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
