/**
 * Implementation of updating the loaded and zoomed textures.
 */
#include "common.hpp"
#include "datatypes/coordinates.hpp"
#include "imagegrid/gridsetup.hpp"
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

void TextureUpdate::clear_nonvisible_textures(ImageGrid* const grid,
                                              TextureGrid* const texture_grid,
                                              std::atomic<bool>& keep_running) {
  auto viewport_current_state=this->_viewport_current_state_texturegrid_update->GetGridValues();
  if (!viewport_current_state.current_grid_coordinate().invalid()) {
    for (const auto& grid_index : ImageGridBasicIterator(grid->grid_setup())) {
      auto current_texture_grid_square=texture_grid->squares(GridIndex(grid_index));
      auto grid_square_visible=this->_grid_square_visible(grid_index,viewport_current_state);
      auto grid_square_adjacent=this->_grid_square_adjacent(grid_index,viewport_current_state);
      auto grid_square_center=this->_grid_square_center(grid_index,viewport_current_state);
      this->clear_textures(viewport_current_state,
                           grid_square_visible,
                           grid_square_adjacent,
                           grid_square_center,
                           current_texture_grid_square,
                           keep_running);
    }
  }
}

void TextureUpdate::find_current_textures(ImageGrid* const grid,
                                          TextureGrid* const texture_grid,
                                          TextureOverlay* const texture_overlay,
                                          TextureUpdateArea texture_update_area,
                                          INT64* const row_buffer_temp,
                                          std::atomic<bool>& keep_running) {
  // bail in this function too to avoid spinning loop too much
  INT64 texture_copy_count=0;
  auto viewport_current_state=this->_viewport_current_state_texturegrid_update->GetGridValues();
  // don't do anything here if viewport_current_state hasn't been initialized
  // find the texture grid textures
  if (!viewport_current_state.current_grid_coordinate().invalid()) {
    std::unique_ptr<ImageGridFromViewportIterator> grid_iterator;
    switch(texture_update_area) {
    case TextureUpdateArea::visible_area:
      grid_iterator=std::make_unique<ImageGridFromViewportVisibleIterator>(grid->grid_setup(),
                                                                           viewport_current_state);
      break;
    case TextureUpdateArea::adjacent_area:
      grid_iterator=std::make_unique<ImageGridFromViewportAdjacentIterator>(grid->grid_setup(),
                                                                            viewport_current_state);
      break;
    case TextureUpdateArea::center_area:
      grid_iterator=std::make_unique<ImageGridFromViewportCenterIterator>(grid->grid_setup(),
                                                                          viewport_current_state);
      break;
    }
    for (const auto& grid_index : *grid_iterator) {
      auto current_texture_grid_square=texture_grid->squares(GridIndex(grid_index));
      auto grid_square_visible=this->_grid_square_visible(grid_index,viewport_current_state);
      auto grid_square_adjacent=this->_grid_square_adjacent(grid_index,viewport_current_state);
      auto grid_square_center=this->_grid_square_center(grid_index,viewport_current_state);
      this->load_new_textures(viewport_current_state,
                              grid->squares(grid_index),
                              current_texture_grid_square,
                              grid_square_visible,
                              grid_square_adjacent,
                              grid_square_center,
                              texture_copy_count,
                              row_buffer_temp,
                              keep_running);
      this->add_filler_textures(viewport_current_state,
                                current_texture_grid_square,
                                keep_running);
    }
    // find the texture grid overlays
    // find the viewport overlay information
    // only update overlay in visible thread
    if (texture_update_area == TextureUpdateArea::visible_area) {
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
      overlay_sstream << "Grid: " << metadata_info.pixel_coordinate.x() << " "
                      << metadata_info.pixel_coordinate.y() << " " << zoom;
      // lock the overlay texture
      std::unique_lock<std::mutex> overlay_lock(texture_overlay->display_mutex, std::defer_lock);
      if (overlay_lock.try_lock()) {
        texture_overlay->update_overlay(overlay_sstream.str());
      }
    }
  }
}

void TextureUpdate::load_new_textures(const ViewPortCurrentState& viewport_current_state,
                                      const ImageGridSquare* const grid_square,
                                      TextureGridSquare* const texture_grid_square,
                                      bool grid_square_visible,
                                      bool grid_square_adjacent,
                                      bool grid_square_center,
                                      INT64& texture_copy_count,
                                      INT64* const row_buffer_temp,
                                      std::atomic<bool>& keep_running) {
  auto max_zoom_out_shift=texture_grid_square->parent_grid()->textures_zoom_out_shift_length()-1;
  auto current_zoom_out_shift=ViewPortTransferState::find_zoom_out_shift_bounded(viewport_current_state.zoom(),
                                                                                 0,
                                                                                 max_zoom_out_shift);
  for (INT64 zoom_out_shift=0; zoom_out_shift <= max_zoom_out_shift; zoom_out_shift++) {
    if (!keep_running ||
        (texture_copy_count >= LOAD_TEXTURES_BATCH)) {
      break;
    }
    if (this->_grid_square_current_load(grid_square_visible,
                                        grid_square_adjacent,
                                        grid_square_center,
                                        max_zoom_out_shift,
                                        current_zoom_out_shift,
                                        zoom_out_shift)) {
      auto dest_square=texture_grid_square->texture_array[zoom_out_shift];
      auto load_index=zoom_out_shift;
      bool texture_copy_successful=false;
      do {
        auto image_square=grid_square->image_array[load_index];
        if (image_square->is_loaded &&
            (!dest_square->is_loaded ||
             dest_square->last_load_index>load_index)) {
          std::unique_lock<std::mutex> load_lock(image_square->load_mutex, std::defer_lock);
          if (load_lock.try_lock()) {
            // try same conditions again after lock aquired
            if (image_square->is_loaded) {
              std::unique_lock<std::mutex> display_lock(dest_square->display_mutex, std::defer_lock);
              if (display_lock.try_lock()) {
                if (!dest_square->is_loaded ||
                    dest_square->last_load_index>load_index) {
                  texture_copy_successful=this->load_texture(dest_square,
                                                             image_square,
                                                             zoom_out_shift,
                                                             row_buffer_temp);
                  if (texture_copy_successful) {
                    dest_square->set_image_loaded(load_index);
                    texture_copy_count+=1;
                  }
                }
                display_lock.unlock();
              }
            }
            load_lock.unlock();
          }
        }
        load_index++;
      } while (!texture_copy_successful && load_index < grid_square->parent_grid()->max_zoom_out_shift());
    }
  }
}

void TextureUpdate::clear_textures(const ViewPortCurrentState& viewport_current_state,
                                   bool grid_square_visible,
                                   bool grid_square_adjacent,
                                   bool grid_square_center,
                                   TextureGridSquare* const texture_grid_square,
                                   std::atomic<bool>& keep_running) {
  auto max_zoom_out_shift=texture_grid_square->parent_grid()->textures_zoom_out_shift_length()-1;
  auto current_zoom_out_shift=ViewPortTransferState::find_zoom_out_shift_bounded(viewport_current_state.zoom(),
                                                                                 0,
                                                                                 max_zoom_out_shift);
  for (INT64 zoom_out_shift=0L; zoom_out_shift <= max_zoom_out_shift; zoom_out_shift++) {
    if (!keep_running) { break; }
    if (!this->_grid_square_current_load(grid_square_visible,
                                         grid_square_adjacent,
                                         grid_square_center,
                                         max_zoom_out_shift,
                                         current_zoom_out_shift,
                                         zoom_out_shift)) {
      auto dest_square=texture_grid_square->texture_array[zoom_out_shift];
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

void TextureUpdate::add_filler_textures(const ViewPortCurrentState& viewport_current_state,
                                        TextureGridSquare* const texture_grid_square,
                                        std::atomic<bool>& keep_running) {
  // this just sets the square as filler for now, but it is likely I
  // will want to draw something specific for invalid/unloaded squares
  // in the future
  auto max_zoom_out_index=texture_grid_square->parent_grid()->textures_zoom_out_shift_length()-1;
  auto current_zoom_out_shift=ViewPortTransferState::find_zoom_out_shift_bounded(viewport_current_state.zoom(),0,max_zoom_out_index);
  for (INT64 zoom_out_shift=max_zoom_out_index; zoom_out_shift >= 0L; zoom_out_shift--) {
    if (!keep_running) { break; }
    if (zoom_out_shift != max_zoom_out_index && zoom_out_shift != current_zoom_out_shift) {
      continue;
    }
    auto dest_square=texture_grid_square->texture_array[zoom_out_shift];
    if (!dest_square->is_loaded && !dest_square->image_filler()) {
      std::unique_lock<std::mutex> display_lock(dest_square->display_mutex, std::defer_lock);
      if (display_lock.try_lock()) {
        dest_square->set_image_filler();
        display_lock.unlock();
      }
    }
  }
}

bool TextureUpdate::load_texture (TextureGridSquareZoomLevel* const dest_square,
                                  ImageGridSquareZoomLevel* const source_square,
                                  INT64 zoom_out_shift,
                                  INT64* const row_buffer) {
  // TODO: change how this notifies about valid/invalid textures
  bool any_successful=false;
  dest_square->_source_square=source_square;
  // TODO: double check this is unnecessary
  // dest_square->unload_all_textures();
  GridPixelSize texture_display_size=static_cast<GridPixelSize>(source_square->parent_square()->parent_grid()->image_max_pixel_size() >> zoom_out_shift);
  dest_square->_texture_display_size=BufferPixelSize(texture_display_size.w(),
                                                      texture_display_size.h());
  auto dest_tile_size=TILE_PIXEL_BASE_SIZE;
  auto next_dest_tile_size=dest_tile_size >> 1L;
  while (next_dest_tile_size > dest_square->_texture_display_size.w() ||
         next_dest_tile_size > dest_square->_texture_display_size.h()) {
    dest_tile_size >>= 1L;
    next_dest_tile_size >>= 1L;
  }
  dest_square->create_surfaces(dest_tile_size);
  // TODO: doesn't seem to be necessary, but check for artifacts
  // dest_square->clear_all_surfaces();
  // skip if can't load texture
  if (dest_square->all_surfaces_valid()) {
    // everything is read, loop over
    auto grid_index=*source_square->parent_square()->grid_index();
    for (const auto& subgrid_index : ImageSubGridBasicIterator(source_square->parent_square()->grid_setup(),
                                                           grid_index)) {
      auto source_data=source_square->rgba_data(subgrid_index);
      auto source_size=BufferPixelSize(source_square->rgba_wpixel(subgrid_index),
                                       source_square->rgba_hpixel(subgrid_index));
      auto source_data_origin_x=source_square->rgba_xpixel_origin(subgrid_index);
      auto source_data_origin_y=source_square->rgba_ypixel_origin(subgrid_index);
      if (source_data) {
        auto source_zoom_out_shift=source_square->zoom_out_shift();
        auto zoom_left_shift=zoom_out_shift-source_zoom_out_shift;
        auto source_texture_size=shift_left_signed(dest_tile_size,zoom_left_shift);
        // which tile is the origin of the source is one
        auto tile_origin_i=source_data_origin_x/source_texture_size;
        auto tile_origin_j=source_data_origin_y/source_texture_size;
        // which tile the source ends on
        auto tile_end_i=(source_data_origin_x+source_size.w())/source_texture_size;
        if (((source_data_origin_x+source_size.w()) % source_texture_size) == 0) {
          tile_end_i-=1;
        }
        auto tile_end_j=(source_data_origin_y+source_size.h())/source_texture_size;
        if (((source_data_origin_y+source_size.h()) % source_texture_size) == 0) {
          tile_end_j-=1;
        }
        // how large is the source on the first tile
        auto source_w_first=source_texture_size-(source_data_origin_x % source_texture_size);
        auto source_h_first=source_texture_size-(source_data_origin_y % source_texture_size);
        for (INT64 tj=tile_origin_j; tj <= tile_end_j; tj++) {
          for (INT64 ti=tile_origin_i; ti <= tile_end_i; ti++) {
            auto tile_index=BufferTileIndex(ti,tj);
            // TODO: get return code from this
            // TODO: use RAII
            dest_square->lock_surface(tile_index);
            // what is the x coordinate of the source that starts the current tile
            // this doesn't get used and/or gets reset in cases where the 1 subtraction would be zero
            INT64 current_tile_source_start_x,current_tile_source_start_y;
            if (ti == tile_origin_i) {
              current_tile_source_start_x=0;
            } else {
              current_tile_source_start_x=(ti-tile_origin_i-1)*source_texture_size+source_w_first;
            }
            if (tj == tile_origin_j) {
              current_tile_source_start_y=0;
            } else {
              current_tile_source_start_y=(tj-tile_origin_j-1)*source_texture_size+source_h_first;
            }
            INT64 dest_start_x,dest_start_y;
            if (ti == tile_origin_i) {
              dest_start_x=shift_right_signed(source_data_origin_x,zoom_left_shift) % dest_tile_size;
            } else {
              dest_start_x=0;
            }
            if (tj == tile_origin_j) {
              dest_start_y=shift_right_signed(source_data_origin_y,zoom_left_shift) % dest_tile_size;
            } else {
              dest_start_y=0;
            }
            INT64 current_tile_source_wpixel,current_tile_source_hpixel;
            if (ti == tile_origin_i && ti == tile_end_i) {
              current_tile_source_wpixel=source_size.w();
            } else if (ti == tile_origin_i) {
              current_tile_source_wpixel=source_w_first;
            } else if (ti == tile_end_i) {
              current_tile_source_wpixel=(source_size.w()-source_w_first)%source_texture_size;
            } else {
              current_tile_source_wpixel=source_texture_size;
            }
            if (tj == tile_origin_j && tj == tile_end_j) {
              current_tile_source_hpixel=source_size.h();
            } else if (tj == tile_origin_j) {
              current_tile_source_hpixel=source_h_first;
            } else if (tj == tile_end_j) {
              current_tile_source_hpixel=(source_size.h()-source_h_first)%source_texture_size;
            } else {
              current_tile_source_hpixel=source_texture_size;
            }
            auto source_start=BufferPixelCoordinate(current_tile_source_start_x, current_tile_source_start_y);
            auto source_copy_size=BufferPixelSize(current_tile_source_wpixel, current_tile_source_hpixel);
            auto dest_start=BufferPixelCoordinate(dest_start_x, dest_start_y);
            //
            auto dest_array=dest_square->get_rgba_pixels(tile_index);
            auto dest_size=dest_square->display_texture_wrapper(tile_index)->texture_size_aligned();
            auto dest_size_visible=dest_square->display_texture_wrapper(tile_index)->texture_size_visible();
            if (zoom_left_shift >= 0) {
              buffer_copy_reduce_standard(source_data,
                                          source_size,
                                          source_start,
                                          source_copy_size,
                                          dest_array,
                                          dest_size,
                                          dest_size_visible,
                                          dest_start,
                                          zoom_left_shift,
                                          row_buffer);
            } else {
              buffer_copy_expand_generic(source_data,
                                         source_size,
                                         source_start,
                                         source_copy_size,
                                         dest_array,
                                         dest_size,
                                         dest_size_visible,
                                         dest_start,
                                         -zoom_left_shift);
            }
            dest_square->unlock_surface(tile_index);
          }
        }
      }
      any_successful=true;
    }
  }
  dest_square->unlock_all_surfaces();
  if (!any_successful) {
    dest_square->unload_all_textures();
  }
  return any_successful;
}

bool TextureUpdate::_grid_square_current_load(bool grid_square_visible,
                                              bool grid_square_adjacent,
                                              bool grid_square_center,
                                              INT64 max_zoom_out_shift,
                                              INT64 current_zoom_out_shift,
                                              INT64 trial_zoom_out_shift) {
  return (
    // always load in everything bigger than or equal to max_zoom_out_shift
    ((trial_zoom_out_shift >= max_zoom_out_shift) ||
     // always load in everything for the center
     grid_square_center ||
     // load in adjacent if zoomed out equal to or more than current
     // XXXX: may want to optionally disable loading all adjacent for certain sizes of images
     //       and/or make good judgments on selectively loading tiles
     (grid_square_adjacent
      // commenting out this bumps memory usage from 70% to 80% on my
      // 16GB macine for a typical dataset I use
      // && (trial_zoom_out_shift >= current_zoom_out_shift)
       ) ||
     // only load in other visible at current zoom
     (grid_square_visible && (trial_zoom_out_shift == current_zoom_out_shift))));
}

bool TextureUpdate::_grid_square_visible(const GridIndex& grid_index,
                                         const ViewPortCurrentState& viewport_current_state) {
  auto i=grid_index.i();
  auto j=grid_index.j();
  auto return_value=(ViewPortTransferState::grid_index_visible(i,
                                                               j,
                                                               viewport_current_state));
  return return_value;
}

bool TextureUpdate::_grid_square_adjacent(const GridIndex& grid_index,
                                          const ViewPortCurrentState& viewport_current_state) {
  auto i=grid_index.i();
  auto j=grid_index.j();
  auto xgrid=viewport_current_state.current_grid_coordinate().x();
  auto ygrid=viewport_current_state.current_grid_coordinate().y();
  auto return_value = (i-1 <= ((int)xgrid) && i+1 >= ((int)xgrid) &&
                       j-1 <= ((int)ygrid) && j+1 >= ((int)ygrid) &&
                       i != (int)xgrid && j != (int)ygrid);
  return return_value;
}

bool TextureUpdate::_grid_square_center(const GridIndex& grid_index,
                                        const ViewPortCurrentState& viewport_current_state) {
  auto i=grid_index.i();
  auto j=grid_index.j();
  auto xgrid=viewport_current_state.current_grid_coordinate().x();
  auto ygrid=viewport_current_state.current_grid_coordinate().y();
  auto return_value = (i == ((int)xgrid) && j == ((int)ygrid));
  return return_value;
}
