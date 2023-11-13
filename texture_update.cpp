/**
 * Implementation of updating the loaded and zoomed textures.
 */
#include "common.hpp"
#include "coordinates.hpp"
#include "imagegrid/imagegrid.hpp"
#include "imagegrid/imagegrid_metadata.hpp"
#include "texturegrid.hpp"
#include "texture_update.hpp"
#include "utility.hpp"
#include "viewport_current_state.hpp"
// C compatible headers
#include "c_misc/buffer_manip.hpp"
#include "c_sdl2/sdl2.hpp"
// C++ headers
#include <mutex>
#include <string>
#include <sstream>
// C headers
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
                                                           zoom_index);
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
          dest_square->unload_all_textures();
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
                                  ImageGridSquareZoomLevel* const source_square,
                                  INT64 zoom_out_shift) {
  INT64 subimages_w=source_square->sub_w();
  INT64 subimages_h=source_square->sub_h();
  // TODO: change how this notifies about valid/invalid textures
  bool any_successful=false;
  dest_square->_source_square=source_square;
  dest_square->unload_all_textures();
  auto tile_pixel_size=TILE_PIXEL_BASE_SIZE >> zoom_out_shift;
  dest_square->create_surfaces(tile_pixel_size);
  // skip if can't load texture
  if (dest_square->all_surfaces_valid()) {
    auto all_lock_successful=dest_square->lock_all_surfaces();
    if (all_lock_successful) {
      bool no_data=true;
      // see if this square should be grayed out
      for (INT64 i_sub=0; i_sub < subimages_w; i_sub++) {
        for (INT64 j_sub=0; j_sub < subimages_h; j_sub++) {
          auto sub_index=SubGridIndex(i_sub,j_sub);
          auto source_data=source_square->get_rgba_data(sub_index);
          if (source_data) {
            no_data=false;
          }
        }
      }
      // zero out any array
      if (no_data) {
        dest_square->clear_all_surfaces();
      }
      // a working buffer for copying, conservatively sized
      auto row_temp_buffer=std::make_unique<INT64[]>(tile_pixel_size*3);
      // everything is read, loop over
      for (INT64 i_sub=0; i_sub < subimages_w; i_sub++) {
        for (INT64 j_sub=0; j_sub < subimages_h; j_sub++) {
          auto sub_index=SubGridIndex(i_sub,j_sub);
          auto source_data=source_square->get_rgba_data(sub_index);
          auto source_wpixel=(INT64)source_square->rgba_wpixel(sub_index);
          auto source_hpixel=(INT64)source_square->rgba_hpixel(sub_index);
          auto source_data_origin_x=source_square->rgba_xpixel_origin(sub_index);
          auto source_data_origin_y=source_square->rgba_ypixel_origin(sub_index);
          if (source_data) {
            auto source_zoom_out_shift=source_square->zoom_out_shift();
            // set pixel size for whole grid square that actually gets displayed
            dest_square->_texture_display_wpixel=source_square->parent_square()->parent_grid()->get_image_max_pixel_size().wpixel() >> zoom_out_shift;
            dest_square->_texture_display_hpixel=source_square->parent_square()->parent_grid()->get_image_max_pixel_size().hpixel() >> zoom_out_shift;
            auto zoom_left_shift=zoom_out_shift-source_zoom_out_shift;
            auto source_texture_size=shift_left_signed(tile_pixel_size,zoom_left_shift);
            // which tile is the origin of the source is one
            auto tile_origin_i=source_data_origin_x/source_texture_size;
            auto tile_origin_j=source_data_origin_y/source_texture_size;
            // which tile the source ends on
            auto tile_end_i=(source_data_origin_x+source_wpixel)/source_texture_size;
            auto tile_end_j=(source_data_origin_y+source_hpixel)/source_texture_size;
            // how large is the source on the first tile
            auto tile_pixel_size_source=TILE_PIXEL_BASE_SIZE >> source_zoom_out_shift;
            auto source_end_x_first=tile_pixel_size_source-(source_data_origin_x % tile_pixel_size_source);
            auto source_end_y_first=tile_pixel_size_source-(source_data_origin_y % tile_pixel_size_source);
            for (INT64 ti=tile_origin_i; ti <= tile_end_i; ti++) {
              for (INT64 tj=tile_origin_j; tj <= tile_end_j; tj++) {
                // what is the x coordinate of the source that starts the current tile
                // this doesn't get used and/or gets reset in cases where the 1 subtraction would be zero
                auto current_tile_source_start_x=(ti-tile_origin_i-1)*source_texture_size+source_end_x_first;
                auto current_tile_source_start_y=(tj-tile_origin_j-1)*source_texture_size+source_end_y_first;
                // find source coordinates for this particular tile
                // now get the pixel range for this tile
                INT64 current_tile_source_wpixel,current_tile_source_hpixel;
                // where on the destination does the the current tile start
                auto dest_start_x=shift_right_signed(source_data_origin_x,zoom_left_shift) % tile_pixel_size;
                auto dest_start_y=shift_right_signed(source_data_origin_y,zoom_left_shift) % tile_pixel_size;
                // find tile i coordinates
                if (ti == tile_origin_i && ti == tile_end_i) {
                  current_tile_source_start_x=0;
                  current_tile_source_wpixel=source_wpixel;
                  // dest_start_x=use default
                } else if (ti == tile_origin_i) {
                  current_tile_source_start_x=0;
                  current_tile_source_wpixel=source_end_x_first;
                  // dest_start_x=use default
                } else if (ti == tile_end_i) {
                  // current_tile_source_start_x=use default
                  current_tile_source_wpixel=(source_wpixel-source_end_x_first)%source_texture_size;
                  dest_start_x=0;
                } else {
                  // current_tile_source_start_x=use default
                  current_tile_source_wpixel=source_texture_size;
                  dest_start_x=0;
                }
                // find tile j coordinates
                if (tj == tile_origin_j && tj == tile_end_j) {
                  current_tile_source_start_y=0;
                  current_tile_source_hpixel=source_hpixel;
                  // dest_start_y=use default
                } else if (tj == tile_origin_j) {
                  current_tile_source_start_y=0;
                  current_tile_source_hpixel=source_end_y_first;
                  // dest_start_y=use default
                } else if (tj == tile_end_j) {
                  // current_tile_source_start_y=use default
                  current_tile_source_hpixel=(source_hpixel-source_end_y_first)%source_texture_size;
                  dest_start_y=0;
                } else {
                  // current_tile_source_start_y=use default
                  current_tile_source_hpixel=source_texture_size;
                  dest_start_y=0;
                }
                auto dest_array=dest_square->get_rgba_pixels(ti,tj);
                auto wpixel_aligned=dest_square->display_texture_wrapper(ti,tj)->texture_wpixel_aligned();
                auto hpixel_aligned=dest_square->display_texture_wrapper(ti,tj)->texture_hpixel_aligned();
                auto wpixel_unaligned=dest_square->display_texture_wrapper(ti,tj)->texture_wpixel_unaligned();
                auto hpixel_unaligned=dest_square->display_texture_wrapper(ti,tj)->texture_hpixel_unaligned();
                if (zoom_left_shift >= 0) {
                  buffer_copy_reduce_generic(source_data,source_wpixel,source_hpixel,
                                             current_tile_source_start_x, current_tile_source_start_y,
                                             current_tile_source_wpixel,current_tile_source_hpixel,
                                             (PIXEL_RGBA*)dest_array,
                                             wpixel_aligned,
                                             hpixel_aligned,
                                             wpixel_unaligned,
                                             hpixel_unaligned,
                                             dest_start_x, dest_start_y,
                                             zoom_left_shift,
                                             row_temp_buffer.get());
                } else {
                  buffer_copy_expand_generic(source_data,source_wpixel,source_hpixel,
                                             current_tile_source_start_x, current_tile_source_start_y,
                                             current_tile_source_wpixel,current_tile_source_hpixel,
                                             (PIXEL_RGBA*)dest_array,
                                             wpixel_aligned,
                                             hpixel_aligned,
                                             wpixel_unaligned,
                                             hpixel_unaligned,
                                             dest_start_x, dest_start_y,
                                             -zoom_left_shift);
                }
              }
            }
          }
        }
        any_successful=true;
      }
    }
  }
  dest_square->unlock_all_surfaces();
  if (!any_successful) {
    dest_square->unload_all_textures();
  }
  return any_successful;
}
