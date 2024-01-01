#include "common.hpp"
#include "imagegrid/gridsetup.hpp"
#include "datatypes/coordinates.hpp"
#include "texture_overlay.hpp"
#include "utility.hpp"
#include "viewport.hpp"
#include "viewport_current_state.hpp"
// C compatible headers
#include "c_sdl2/sdl2.hpp"
// C++ headers
#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <vector>
#include <utility>
// C headers
#include <cmath>
#include <cstddef>

void blit_this(SDLDrawableSurface* screen_surface,
               TextureGridSquareZoomLevel* const blit_square,
               const BufferPixelCoordinate &l_viewport_pixel_coordinate,
               const BufferPixelSize& grid_image_size_zoomed) {
   auto viewport_pixel_coordinate=BufferPixelCoordinate(l_viewport_pixel_coordinate);
   auto image_pixel_size_viewport=BufferPixelSize(grid_image_size_zoomed);
   if (blit_square->image_filler()) {
     screen_surface->draw_rect(viewport_pixel_coordinate, image_pixel_size_viewport, FILLER_LEVEL);
  } else {
     FLOAT64 texture_square_wpixel=blit_square->texture_square_pixel_size().w();
     FLOAT64 texture_square_hpixel=blit_square->texture_square_pixel_size().h();
     for (INT64 j=0; j < blit_square->tile_size().h(); j++) {
       for (INT64 i=0; i < blit_square->tile_size().w(); i++) {
         auto tile_index=BufferTileIndex(i,j);
         FLOAT64 texture_xpixel_start,texture_xpixel_end;
         FLOAT64 texture_ypixel_start,texture_ypixel_end;
         FLOAT64 texture_wpixel, texture_hpixel;
         FLOAT64 vp_xpixel_start,vp_xpixel_end;
         FLOAT64 vp_ypixel_start,vp_ypixel_end;
         FLOAT64 vp_wpixel, vp_hpixel;
         // TODO: this assumes uniform sized textures for now
         FLOAT64 texture_wpixel_base=(FLOAT64)blit_square->display_texture_wrapper(tile_index)->texture_size_visible().w();
         FLOAT64 texture_hpixel_base=(FLOAT64)blit_square->display_texture_wrapper(tile_index)->texture_size_visible().h();
         FLOAT64 vp_wpixel_base=(FLOAT64)image_pixel_size_viewport.w()*(FLOAT64)texture_wpixel_base/(FLOAT64)texture_square_wpixel;
         FLOAT64 vp_hpixel_base=(FLOAT64)image_pixel_size_viewport.h()*(FLOAT64)texture_hpixel_base/(FLOAT64)texture_square_hpixel;
         // if this is the last column
         if (i == blit_square->tile_size().w()-1) {
           texture_xpixel_start=(FLOAT64)i*texture_wpixel_base;
           texture_xpixel_end=texture_square_wpixel;
           texture_wpixel=texture_xpixel_end-texture_xpixel_start;
           vp_xpixel_start=floor((FLOAT64)i*vp_wpixel_base);
           vp_xpixel_end=(FLOAT64)image_pixel_size_viewport.w();
           vp_wpixel=vp_xpixel_end-vp_xpixel_start;
         } else {
           texture_xpixel_start=(FLOAT64)i*texture_wpixel_base;
           texture_xpixel_end=(FLOAT64)(i+1)*texture_wpixel_base;
           texture_wpixel=texture_xpixel_end-texture_xpixel_start;
           vp_xpixel_start=floor((FLOAT64)i*vp_wpixel_base);
           vp_xpixel_end=ceil_minus_one((FLOAT64)(i+1)*vp_wpixel_base);
           vp_wpixel=vp_xpixel_end-vp_xpixel_start;
         }
         // if this is the last row
         if (j == blit_square->tile_size().h()-1) {
           texture_ypixel_start=(FLOAT64)j*texture_hpixel_base;
           texture_ypixel_end=texture_square_hpixel;
           texture_hpixel=texture_ypixel_end-texture_ypixel_start;
           vp_ypixel_start=floor((FLOAT64)j*vp_hpixel_base);
           vp_ypixel_end=(FLOAT64)image_pixel_size_viewport.h();
           vp_hpixel=vp_ypixel_end-vp_ypixel_start;
         } else {
           texture_ypixel_start=(FLOAT64)j*texture_hpixel_base;
           texture_ypixel_end=(FLOAT64)(j+1)*texture_hpixel_base;
           texture_hpixel=texture_ypixel_end-texture_ypixel_start;
           vp_ypixel_start=floor((FLOAT64)j*vp_hpixel_base);
           vp_ypixel_end=ceil_minus_one((FLOAT64)(j+1)*vp_hpixel_base);
           vp_hpixel=vp_ypixel_end-vp_ypixel_start;
         }
         auto viewport_pixel_coordinate_local=BufferPixelCoordinate((INT64)((FLOAT64)viewport_pixel_coordinate.x()+vp_xpixel_start),
                                                                    (INT64)((FLOAT64)viewport_pixel_coordinate.y()+vp_ypixel_start));
         auto viewport_pixel_size=BufferPixelSize((INT64)vp_wpixel,
                                                  (INT64)vp_hpixel);
         blit_square->display_texture_wrapper(tile_index)->blit_texture(screen_surface,
                                                                        BufferPixelSize((INT64)texture_wpixel,
                                                                                        (INT64)texture_hpixel),
                                                                        viewport_pixel_coordinate_local,
                                                                        viewport_pixel_size);
       }
     }
   }
}

ViewPort::ViewPort(std::shared_ptr<ViewPortTransferState> viewport_current_state_texturegrid_update,
                   std::shared_ptr<ViewPortTransferState> viewport_current_state_imagegrid_update) {
  this->_viewport_current_state_texturegrid_update=viewport_current_state_texturegrid_update;
  this->_viewport_current_state_imagegrid_update=viewport_current_state_imagegrid_update;
  this->update_viewport_info(GridCoordinate(INITIAL_X,INITIAL_Y));
}

void ViewPort::set_image_max_size(const GridPixelSize& image_max_size) {
  this->_image_max_size=GridPixelSize(image_max_size);
}

void ViewPort::find_viewport_blit(TextureGrid* const texture_grid,
                                  TextureOverlay* const texture_overlay,
                                  SDLApp* const sdl_app) {
  // locking the textures
  // Stores the next items to be blit to the viewport.
  // lifted out of the old find_viewport_extents_grid function since it's only called from here
  // will be put into function eventually, but I wanted to use new data structures
  // I don't use objects for some things because I want to use FLOAT64 for intermediate calculations
  auto max_zoom_out_shift=texture_grid->textures_zoom_out_shift_length()-1;
  // TODO refactor this out
  if (max_zoom_out_shift < 0) { max_zoom_out_shift=0; }
  auto zoom_out_shift=ViewPortTransferState::find_zoom_out_shift_bounded(this->_zoom,0,max_zoom_out_shift);
  ////////////////////////////////////////////////////////////////////////////////
  // now loop over grid squares
  auto viewport_pixel_size=BufferPixelSize(this->_current_window_w,this->_current_window_h);
  // this draws the surface when it goes out of scope
  std::unique_ptr<SDLDrawableSurface> drawable_surface=std::make_unique<SDLDrawableSurface>(sdl_app,viewport_pixel_size);
  auto pixel_0=BufferPixelCoordinate(0,0);
  auto viewport_pixel_0_grid=GridCoordinate(pixel_0,
                                            this->_zoom,
                                            this->_viewport_pixel_size,
                                            this->_viewport_grid,
                                            this->_image_max_size);
  auto new_viewport_pixel_size=BufferPixelSize(this->_image_max_size.w(),this->_image_max_size.h());
  for (const auto& grid_index : ImageGridBasicIterator(texture_grid->grid_setup())) {
    auto i=grid_index.i();
    auto j=grid_index.j();
    auto upperleft_gridcoordinate=GridCoordinate((FLOAT64)i,(FLOAT64)j);
    auto lowerright_gridcoordinate=GridCoordinate((FLOAT64)(i+1),(FLOAT64)(j+1));
    auto viewport_pixel_coordinate_upperleft=BufferPixelCoordinate(upperleft_gridcoordinate,this->_zoom,viewport_pixel_0_grid,new_viewport_pixel_size);
    auto viewport_pixel_coordinate_lowerright=BufferPixelCoordinate(lowerright_gridcoordinate,this->_zoom,viewport_pixel_0_grid,new_viewport_pixel_size);
    // TODO: possibly add some padding here
    if (viewport_pixel_coordinate_lowerright.x() < 0 || viewport_pixel_coordinate_lowerright.y() < 0 ||
        viewport_pixel_coordinate_upperleft.x() > MAX_SCREEN_WIDTH || viewport_pixel_coordinate_upperleft.y() > MAX_SCREEN_HEIGHT) {
          continue;
    }
    auto actual_zoom=zoom_out_shift;
    // for testing max zoom
    bool texture_loaded=false;
    do {
      auto texture_square_zoom=texture_grid->squares(GridIndex(i,j))->texture_array[actual_zoom];
      // filler should never get past here
      if (texture_square_zoom->is_loaded &&
          texture_square_zoom->is_displayable) {
        // TODO: would like more RAII way of dealing with this mutex
        if (texture_square_zoom->display_mutex.try_lock()) {
          if (texture_square_zoom->is_loaded &&
              texture_square_zoom->is_displayable &&
              texture_square_zoom->all_surfaces_valid()) {
            texture_loaded=true;
            auto grid_image_size_zoomed=BufferPixelSize((INT64)round((FLOAT64)this->_image_max_size.w()*this->_zoom),
                                                        (INT64)round((FLOAT64)this->_image_max_size.h()*this->_zoom));
            blit_this(drawable_surface.get(),
                      texture_square_zoom,
                      viewport_pixel_coordinate_upperleft,
                      grid_image_size_zoomed);
            texture_square_zoom->display_mutex.unlock();
          } else {
            texture_loaded=false;
            texture_square_zoom->display_mutex.unlock();
            MSG_LOCAL("Couldn't access surface: " << i << " " << j << " " << actual_zoom);
          }
        }
      }
      // TODO: else raise error if things are terrible
      actual_zoom++;
    } while ((actual_zoom <= max_zoom_out_shift) && !texture_loaded);
    // texture didn't load load so do filler
    if (!texture_loaded) {
      auto texture_square_zoom=texture_grid->squares(GridIndex(i,j))->texture_array[zoom_out_shift];
      if (texture_square_zoom->image_filler()) {
        // TODO: would like more RAII way of dealing with this mutex
        // still want a mutex here so indication that texture is to use filler doesn't change
        if (texture_square_zoom->display_mutex.try_lock()) {
          if (texture_square_zoom->image_filler()
              // &&
              // texture_square_zoom->filler_texture_wrapper()->is_valid()
            ) {
            auto grid_image_size_zoomed=BufferPixelSize((INT64)round((FLOAT64)this->_image_max_size.w()*this->_zoom),
                                                        (INT64)round((FLOAT64)this->_image_max_size.h()*this->_zoom));
            blit_this(drawable_surface.get(),
                      texture_square_zoom,
                      viewport_pixel_coordinate_upperleft,
                      grid_image_size_zoomed);
          }
          texture_square_zoom->display_mutex.unlock();
        } else {
          // TODO: this should never happen, so this error is in here while I investigate
          MSG_LOCAL("Couldn't lock filler: " << i << " " << j);
        }
      }
    }
  }
  // I guess we don't get an overlay if we don't get the lock
  // Could just discard the surface
  std::unique_lock<std::mutex> overlay_lock(texture_overlay->display_mutex, std::defer_lock);
  if (overlay_lock.try_lock()) {
    texture_overlay->draw_overlay(drawable_surface.get());
    overlay_lock.unlock();
  }
}

bool ViewPort::do_input(SDLApp* const sdl_app) {
  auto xgrid=this->_viewport_grid.x();
  auto ygrid=this->_viewport_grid.y();
  auto keep_going=sdl_app->do_input(this->_current_speed_x, this->_current_speed_y,
                                    this->_current_speed_zoom, this->_zoom, this->_zoom_speed,
                                    this->_image_max_size, xgrid, ygrid,
                                    this->_current_mouse_xpixel, this->_current_mouse_ypixel,
                                    this->_current_window_w,
                                    this->_current_window_h);
  this->update_viewport_info(GridCoordinate(xgrid,ygrid));
  return keep_going;
}

void ViewPort::update_viewport_info(const GridCoordinate& grid_coordinate) {
  this->_viewport_grid=GridCoordinate(grid_coordinate);
  this->_viewport_pixel_size=BufferPixelSize(this->_current_window_w,this->_current_window_h);
  // update the viewport
  // TODO: too much duplicate code here
  this->_viewport_current_state_texturegrid_update->UpdateGridValues(this->_zoom,
                                                                     this->_viewport_grid,
                                                                     this->_image_max_size,
                                                                     this->_viewport_pixel_size,
                                                                     BufferPixelCoordinate(this->_current_mouse_xpixel,
                                                                                             this->_current_mouse_ypixel),
                                                                     BufferPixelCoordinate(this->_viewport_pixel_size.w()/2,
                                                                                             this->_viewport_pixel_size.h()/2));
  this->_viewport_current_state_imagegrid_update->UpdateGridValues(this->_zoom,
                                                                   this->_viewport_grid,
                                                                   this->_image_max_size,
                                                                   this->_viewport_pixel_size,
                                                                   BufferPixelCoordinate(this->_current_mouse_xpixel,
                                                                                           this->_current_mouse_ypixel),
                                                                   BufferPixelCoordinate(this->_viewport_pixel_size.w()/2,
                                                                                           this->_viewport_pixel_size.h()/2));

}

void ViewPort::adjust_initial_location(const GridSetup* const grid_setup) {
  // adjust initial position for small grids
  FLOAT64 new_xgrid,new_ygrid;
  if (grid_setup->grid_image_size().w() == 1) {
    new_xgrid=0.5;
  } else {
    new_xgrid=this->_viewport_grid.x();
  }
  if (grid_setup->grid_image_size().h() == 1) {
    new_ygrid=0.5;
  } else {
    new_ygrid=this->_viewport_grid.y();
  }
  this->update_viewport_info(GridCoordinate(new_xgrid,new_ygrid));
}
