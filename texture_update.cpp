/**
 * Implementation of updating the loaded and zoomed textures.
 */
#include "common.hpp"
#include "coordinates.hpp"
#include "imagegrid/imagegrid.hpp"
#include "texturegrid.hpp"
#include "texture_update.hpp"
#include "viewport_current_state.hpp"
// C headers
// TODO: move out code like this
#include <cstring>
// C compatible headers
#include "c_misc/buffer_manip.hpp"

TextureUpdate::TextureUpdate(std::shared_ptr<ViewPortTransferState> viewport_current_state_texturegrid_update) {
  this->_viewport_current_state_texturegrid_update=viewport_current_state_texturegrid_update;
}

void TextureUpdate::find_current_textures (const ImageGrid* const grid,
                                           TextureGrid* const texture_grid,
                                           std::atomic<bool> &keep_running) {
  INT_T texture_copy_count=0;
  auto viewport_current_state=this->_viewport_current_state_texturegrid_update->GetGridValues();
  // if (view_changed) {
  // don't do anything here if viewport_current_state hasn't been initialized
  if (!viewport_current_state.current_grid_coordinate().invalid()) {
    for (INT_T j=0L; j < texture_grid->grid_image_size().himage(); j++) {
      for (INT_T i=0L; i < texture_grid->grid_image_size().wimage(); i++) {
        this->load_new_textures(i,j,viewport_current_state,grid,texture_grid,texture_copy_count,keep_running);
        this->clear_textures(i,j,viewport_current_state,texture_grid,keep_running);
        this->add_filler_textures(i,j,viewport_current_state,texture_grid,keep_running);
      }
    }
  }
}

void TextureUpdate::load_new_textures(INT_T i,
                                      INT_T j,
                                      const ViewPortCurrentState& viewport_current_state,
                                      const ImageGrid* const grid,
                                      TextureGrid* const texture_grid,
                                      INT_T &texture_copy_count,
                                      std::atomic<bool> &keep_running) {
  auto max_zoom_index=texture_grid->textures_zoom_index_length()-1;
  auto current_zoom_index=ViewPortTransferState::find_zoom_index_bounded(viewport_current_state.zoom(),0,max_zoom_index);
  for (INT_T zoom_index=0; zoom_index <= max_zoom_index; zoom_index++) {
    if (!keep_running ||
        (texture_copy_count >= LOAD_TEXTURES_BATCH)) {
      break;
    }
    // only load/update current zoom and max_zoom
    // TODO: want this to be initial, but then fill out textures in background
    if (zoom_index != max_zoom_index && zoom_index != current_zoom_index) {
      continue;
    }
    auto dest_square=texture_grid->squares[i][j]->texture_array[zoom_index].get();
    auto load_all=(zoom_index == max_zoom_index);
    if (load_all || _grid_square_visible(i,j,viewport_current_state)) {
      auto load_index=zoom_index;
      bool texture_copy_successful=false;
      do {
        auto image_square=grid->squares[i][j]->image_array[load_index].get();
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
                texture_copy_successful=this->load_texture(dest_square,
                                                           image_square,
                                                           zoom_index,
                                                           GridPixelSize(viewport_current_state.max_image_size()));
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
                                   const ViewPortCurrentState& viewport_current_state,
                                   TextureGrid* const texture_grid,
                                   std::atomic<bool> &keep_running) {
  auto max_zoom_index=texture_grid->textures_zoom_index_length()-1;
  // never clear out top level index
  for (INT_T zoom_index=0L; zoom_index < max_zoom_index-1; zoom_index++) {
    if (!keep_running) { break; }
    auto dest_square=texture_grid->squares[i][j]->texture_array[zoom_index].get();
    if (!_grid_square_visible(i,j,viewport_current_state)) {
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
                                        const ViewPortCurrentState& viewport_current_state,
                                        TextureGrid* const texture_grid,
                                        std::atomic<bool> &keep_running) {
  auto max_zoom_index=texture_grid->textures_zoom_index_length()-1;
  auto current_zoom_index=ViewPortTransferState::find_zoom_index_bounded(viewport_current_state.zoom(),0,max_zoom_index);
  for (INT_T zoom_index=max_zoom_index; zoom_index >= 0L; zoom_index--) {
    if (!keep_running) { break; }
    if (zoom_index != max_zoom_index && zoom_index != current_zoom_index) {
      continue;
    }
    auto load_all=(zoom_index == max_zoom_index);
    auto dest_square=texture_grid->squares[i][j]->texture_array[zoom_index].get();
    // take all the precautions for setting a texture as filler as
    // when we used to copy
    if (load_all || _grid_square_visible(i,j,viewport_current_state)) {
      if (!dest_square->is_displayable) {
        std::unique_lock<std::mutex> display_lock(dest_square->display_mutex, std::defer_lock);
        if (display_lock.try_lock()) {
          if (!dest_square->is_displayable) {
            dest_square->set_image_filler();
          }
          display_lock.unlock();
        }
      }
    }
  }
}


bool TextureUpdate::_grid_square_visible(INT_T i, INT_T j,
                                         const ViewPortCurrentState& viewport_current_state) {
  auto xgrid=viewport_current_state.current_grid_coordinate().xgrid();
  auto ygrid=viewport_current_state.current_grid_coordinate().ygrid();
  auto return_value=(ViewPortTransferState::grid_index_visible(i, j,
                                                               viewport_current_state) ||
                     (i >= floor(xgrid)-1 && i <= floor(xgrid)+1 &&
                      j >= floor(ygrid)-1 && j <= floor(ygrid)+1));
  return return_value;
}

bool TextureUpdate::load_texture (TextureGridSquareZoomLevel* const dest_square,
                                  const ImageGridSquareZoomLevel* const source_square,
                                  INT_T zoom_index,
                                  GridPixelSize texture_pixel_size) {
  INT_T subimages_w=1;
  INT_T subimages_h=1;
  auto successful=true;
  auto texture_zoom_reduction=((INT_T)pow(2,zoom_index));
  auto dest_wpixel=texture_pixel_size.wpixel()/texture_zoom_reduction;
  auto dest_hpixel=texture_pixel_size.hpixel()/texture_zoom_reduction;
  dest_wpixel=dest_wpixel + (TEXTURE_ALIGNMENT - (dest_wpixel % TEXTURE_ALIGNMENT));
  dest_square->unload_texture();
  dest_square->display_texture_wrapper()->create_surface(dest_wpixel, dest_hpixel);
  // skip if can't load texture
  if (dest_square->display_texture_wrapper()->is_valid()) {
    auto lock_surface_return=dest_square->display_texture_wrapper()->lock_surface();
    if (lock_surface_return == 0) {
      // everything is read, loop over
      for (INT_T i_sub=0; i_sub < subimages_w; i_sub++) {
        for (INT_T j_sub=0; j_sub < subimages_h; j_sub++) {
          auto source_wpixel=(INT_T)source_square->rgb_wpixel(i_sub, j_sub);
          auto source_hpixel=(INT_T)source_square->rgb_hpixel(i_sub, j_sub);
          auto source_data=source_square->get_rgb_data(i_sub,j_sub);
          auto source_data_origin_x=source_square->rgb_xpixel_origin(i_sub, j_sub);
          auto source_data_origin_y=source_square->rgb_ypixel_origin(i_sub, j_sub);
          auto dest_array=dest_square->display_texture_wrapper()->pixels();
          // do the things we are copying exist?
          if (dest_array != nullptr && source_data != nullptr) {
            // these should only be powers of 2, add an assert
            auto source_zoom_out_value=source_square->zoom_out_value();
            auto dest_zoom_index=texture_zoom_reduction;
            // TODO: move out once I restructure code appropriately
            std::memset((void*)dest_array,0,dest_wpixel*dest_hpixel*3);
            if (source_zoom_out_value <= dest_zoom_index) {
              auto skip=dest_zoom_index/source_zoom_out_value;
              buffer_copy_reduce_generic(source_data,source_wpixel,source_hpixel,
                                             source_data_origin_x, source_data_origin_y,
                                             (unsigned char*)dest_array,dest_wpixel,dest_hpixel,
                                             skip);
            } else {
              auto skip=source_zoom_out_value/dest_zoom_index;
              buffer_copy_expand_generic(source_data,source_wpixel,source_hpixel,
                                         source_data_origin_x, source_data_origin_y,
                                         (unsigned char*)dest_array,dest_wpixel,dest_hpixel,
                                         skip);
            }
          } else {
            dest_square->display_texture_wrapper()->unlock_surface();
            dest_square->unload_texture();
            successful=false;
          }
        }
      }
      dest_square->display_texture_wrapper()->unlock_surface();
    } else {
      dest_square->unload_texture();
      successful=false;
    }
  } else {
    successful=false;
  }
  return successful;
}
