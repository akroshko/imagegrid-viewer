/**
 * Implementation of updating the loaded and zoomed textures.
 */
#include "common.hpp"
#include "coordinates.hpp"
#include "imagegrid/imagegrid.hpp"
#include "imagegrid/imagegrid_metadata.hpp"
#include "texturegrid.hpp"
#include "texture_update.hpp"
#include "viewport_current_state.hpp"
// C compatible headers
#include "c_misc/buffer_manip.hpp"
#include "c_sdl/sdl.hpp"
// C++ headers
#include <mutex>
#include <string>
#include <sstream>
// C headers
#include <cstring>
#include <cmath>

TextureUpdate::TextureUpdate(std::shared_ptr<ViewPortTransferState> viewport_current_state_texturegrid_update) {
  this->_viewport_current_state_texturegrid_update=viewport_current_state_texturegrid_update;
}

void TextureUpdate::find_current_textures(const ImageGrid* const grid,
                                          TextureGrid* const texture_grid,
                                          TextureOverlay* const texture_overlay,
                                          std::atomic<bool>& keep_running) {
  // bail in this function too to avoid spinning loop too much
  INT64 texture_copy_count=0;
  auto viewport_current_state=this->_viewport_current_state_texturegrid_update->GetGridValues();
  // don't do anything here if viewport_current_state hasn't been initialized
  // find the texture grid textures
  if (!viewport_current_state.current_grid_coordinate().invalid()) {
    for (INT64 j=0L; j < texture_grid->grid_image_size().himage(); j++) {
      for (INT64 i=0L; i < texture_grid->grid_image_size().wimage(); i++) {
        auto grid_index=GridIndex(i,j);
        auto grid_square_visible=this->_grid_square_visible(i,j,viewport_current_state);
        auto current_texture_grid_square=texture_grid->squares[i][j].get();
        this->load_new_textures(grid_square_visible,viewport_current_state,
                                grid->squares(grid_index),
                                current_texture_grid_square,
                                texture_copy_count,keep_running);
        this->clear_textures(grid_square_visible,
                             current_texture_grid_square,
                             keep_running);
        this->add_filler_textures(grid_square_visible,
                                  viewport_current_state,
                                  current_texture_grid_square,
                                  keep_running);
      }
    }
    // find the texture grid overlays
    // for (INT64 j=0L; j < texture_grid->grid_image_size().himage(); j++) {
    //   for (INT64 i=0L; i < texture_grid->grid_image_size().wimage(); i++) {
    //     auto grid_index=GridIndex(i,j);
    //     auto grid_square_visible=this->_grid_square_visible(i,j,viewport_current_state);
    //     // TODO: not implemented
    //   }
    // }
    //
    // find the viewport overlay information
    // TODO: sort out passing references and clean up so these autos aren't required
    auto pointer_pixel_coordinate=viewport_current_state.pointer();
    auto zoom=viewport_current_state.zoom();
    auto viewport_pixel_size=viewport_current_state.screen_size();
    auto viewport_grid_coordinate=viewport_current_state.current_grid_coordinate();
    auto image_max_size=viewport_current_state.image_max_size();
    auto cursor_grid=GridCoordinate(pointer_pixel_coordinate,
                                    zoom,
                                    viewport_pixel_size,
                                    viewport_grid_coordinate,
                                    image_max_size);
    std::ostringstream overlay_sstream;
    ImageGridMetadata imagegrid_metadata;
    MetadataInfo metadata_info;
    auto metadata_name=std::string("pixel_only");
    imagegrid_metadata.get_metadata(grid,metadata_name,cursor_grid,metadata_info);
    // overlay_sstream << "Grid: " << cursor_grid.xgrid() << " " << cursor_grid.ygrid();
    overlay_sstream << "Grid: " << metadata_info.pixel_coordinate.xpixel() << " "
                                << metadata_info.pixel_coordinate.ypixel();
    // lock the overlay texture
    std::unique_lock<std::mutex> overlay_lock(texture_overlay->display_mutex, std::defer_lock);
    if (overlay_lock.try_lock()) {
      texture_overlay->update_overlay(overlay_sstream.str());
    }
  }
}

void TextureUpdate::load_new_textures(bool grid_square_visible,
                                      const ViewPortCurrentState& viewport_current_state,
                                      const ImageGridSquare* const grid_square,
                                      TextureGridSquare* const texture_grid_square,
                                      INT64& texture_copy_count,
                                      std::atomic<bool>& keep_running) {
  auto max_zoom_index=texture_grid_square->parent_grid()->textures_zoom_index_length()-1;
  auto current_zoom_index=ViewPortTransferState::find_zoom_index_bounded(viewport_current_state.zoom(),0,max_zoom_index);
  for (INT64 zoom_index=0; zoom_index <= max_zoom_index; zoom_index++) {
    if (!keep_running ||
        (texture_copy_count >= LOAD_TEXTURES_BATCH)) {
      break;
    }
    // only load/update current zoom and max_zoom
    if (zoom_index != max_zoom_index && zoom_index != current_zoom_index) {
      continue;
    }
    auto dest_square=texture_grid_square->texture_array[zoom_index].get();
    auto load_all=(zoom_index == max_zoom_index);
    if (load_all || grid_square_visible) {
      auto load_index=zoom_index;
      bool texture_copy_successful=false;
      do {
        auto image_square=grid_square->image_array[load_index].get();
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
                                                           GridPixelSize(viewport_current_state.image_max_size()));
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
      } while (!texture_copy_successful && load_index < grid_square->parent_grid()->zoom_index_length());
    }
  }
}

void TextureUpdate::clear_textures(bool grid_square_visible,
                                   TextureGridSquare* const texture_grid_square,
                                   std::atomic<bool>& keep_running) {
  auto max_zoom_index=texture_grid_square->parent_grid()->textures_zoom_index_length()-1;
  // never clear out top level index
  for (INT64 zoom_index=0L; zoom_index < max_zoom_index; zoom_index++) {
    if (!keep_running) { break; }
    auto dest_square=texture_grid_square->texture_array[zoom_index].get();
    if (!grid_square_visible) {
      // unload anything not visible that is loadable or displayable
      if (dest_square->is_loaded) {
        std::unique_lock<std::mutex> display_lock(dest_square->display_mutex, std::defer_lock);
        if (display_lock.try_lock()) {
          dest_square->unload_texture();
          display_lock.unlock();
        }
      }
    }
  }
}

void TextureUpdate::add_filler_textures(bool grid_square_visible,
                                        const ViewPortCurrentState& viewport_current_state,
                                        TextureGridSquare* const texture_grid_square,
                                        std::atomic<bool>& keep_running) {
  auto max_zoom_index=texture_grid_square->parent_grid()->textures_zoom_index_length()-1;
  auto current_zoom_index=ViewPortTransferState::find_zoom_index_bounded(viewport_current_state.zoom(),0,max_zoom_index);
  for (INT64 zoom_index=max_zoom_index; zoom_index >= 0L; zoom_index--) {
    if (!keep_running) { break; }
    if (zoom_index != max_zoom_index && zoom_index != current_zoom_index) {
      continue;
    }
    auto load_all=(zoom_index == max_zoom_index);
    auto dest_square=texture_grid_square->texture_array[zoom_index].get();
    if (load_all || grid_square_visible) {
      if (!dest_square->is_loaded && !dest_square->get_image_filler()) {
        std::unique_lock<std::mutex> display_lock(dest_square->display_mutex, std::defer_lock);
        if (display_lock.try_lock()) {
          dest_square->set_image_filler();
          display_lock.unlock();
        }
      }
    }
  }
}


bool TextureUpdate::_grid_square_visible(INT64 i, INT64 j,
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
                                  INT64 zoom_index,
                                  GridPixelSize texture_pixel_size) {
  INT64 subimages_w=source_square->sub_w();
  INT64 subimages_h=source_square->sub_h();
  // TODO: change how this notifies about valid/invalid textures
  bool any_successful=false;
  auto texture_zoom_reduction=((INT64)pow(2,zoom_index));
  auto dest_wpixel=texture_pixel_size.wpixel()/texture_zoom_reduction;
  auto dest_hpixel=texture_pixel_size.hpixel()/texture_zoom_reduction;
  dest_wpixel=dest_wpixel + (TEXTURE_ALIGNMENT - (dest_wpixel % TEXTURE_ALIGNMENT));
  dest_square->unload_texture();
  dest_square->display_texture_wrapper()->create_surface(dest_wpixel, dest_hpixel);
  // skip if can't load texture
  if (dest_square->display_texture_wrapper()->is_valid()) {
    auto lock_surface_return=dest_square->display_texture_wrapper()->lock_surface();
    if (lock_surface_return == 0) {
      auto dest_array=dest_square->display_texture_wrapper()->pixels();
      bool no_data=true;
      // see if this square should be grayed out
      for (INT64 i_sub=0; i_sub < subimages_w; i_sub++) {
        for (INT64 j_sub=0; j_sub < subimages_h; j_sub++) {
          auto sub_index=SubGridIndex(i_sub,j_sub);
          auto source_data=source_square->get_rgba_data(sub_index);
          if (dest_array && source_data) {
            no_data=false;
          }
        }
      }
      if (dest_array && no_data) {
        std::memset((void*)dest_array,0,sizeof(PIXEL_RGBA)*dest_wpixel*dest_hpixel);
      }
      // everything is read, loop over
      for (INT64 i_sub=0; i_sub < subimages_w; i_sub++) {
        for (INT64 j_sub=0; j_sub < subimages_h; j_sub++) {
          auto sub_index=SubGridIndex(i_sub,j_sub);
          auto source_data=source_square->get_rgba_data(sub_index);
          auto source_wpixel=(INT64)source_square->rgba_wpixel(sub_index);
          auto source_hpixel=(INT64)source_square->rgba_hpixel(sub_index);
          auto source_data_origin_x=source_square->rgba_xpixel_origin(sub_index);
          auto source_data_origin_y=source_square->rgba_ypixel_orgin(sub_index);
          // do the things we are copying exist?
          if (dest_array && source_data) {
            // these should only be powers of 2, add an assert
            auto source_zoom_out=source_square->zoom_out();
            auto dest_zoom_index=texture_zoom_reduction;
            // TODO: move out once I restructure code appropriately
            if (source_zoom_out <= dest_zoom_index) {
              auto zoom_out=dest_zoom_index/source_zoom_out;
              buffer_copy_reduce_generic(source_data,source_wpixel,source_hpixel,
                                         source_data_origin_x, source_data_origin_y,
                                         (PIXEL_RGBA*)dest_array,dest_wpixel,dest_hpixel,
                                         zoom_out);
            } else {
              auto zoom_in=source_zoom_out/dest_zoom_index;
              buffer_copy_expand_generic(source_data,source_wpixel,source_hpixel,
                                         source_data_origin_x, source_data_origin_y,
                                         (PIXEL_RGBA*)dest_array,dest_wpixel,dest_hpixel,
                                         zoom_in);
            }
            any_successful=true;
          }
        }
      }
      dest_square->display_texture_wrapper()->unlock_surface();
    }
    if (!any_successful) {
      dest_square->unload_texture();
    }
  }
  return any_successful;
}
