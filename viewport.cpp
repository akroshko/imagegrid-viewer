#include "common.hpp"
#include "imagegrid/gridsetup.hpp"
#include "coordinates.hpp"
#include "texture_overlay.hpp"
#include "viewport.hpp"
#include "viewport_current_state.hpp"
// C compatible headers
#include "c_sdl/sdl.hpp"
// C++ headers
#include <atomic>
#include <iostream>
#include <mutex>
#include <vector>
#include <utility>
// C headers
#include <cmath>
#include <cstddef>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

BlitItem::BlitItem(TextureGridSquareZoomLevel* const square, INT_T count,
                   const ViewportPixelCoordinate &l_viewport_pixel_coordinate,
                   const ViewportPixelSize& grid_image_size_zoomed) {
  this->_blit_index=count;
  this->blit_square=square;
  this->viewport_pixel_coordinate=ViewportPixelCoordinate(l_viewport_pixel_coordinate);
  this->image_pixel_size_viewport=ViewportPixelSize(grid_image_size_zoomed);
}

void BlitItem::blit_this(SDLDrawableSurface* screen_surface) {
  if (this->blit_square->get_image_filler()) {
    blit_square->filler_texture_wrapper()->blit_texture(screen_surface,
                                                        this->viewport_pixel_coordinate,
                                                        this->image_pixel_size_viewport);
  } else {
    blit_square->display_texture_wrapper()->blit_texture(screen_surface,
                                                         this->viewport_pixel_coordinate,
                                                         this->image_pixel_size_viewport);
  }
}

ViewPort::ViewPort(std::shared_ptr<ViewPortTransferState> viewport_current_state_texturegrid_update,
                   std::shared_ptr<ViewPortTransferState> viewport_current_state_imagegrid_update) {
  this->_viewport_current_state_texturegrid_update=viewport_current_state_texturegrid_update;
  this->_viewport_current_state_imagegrid_update=viewport_current_state_imagegrid_update;
  this->update_viewport_info(INITIAL_X,INITIAL_Y);
}

void ViewPort::set_image_max_size(const GridPixelSize& image_max_size) {
  this->_image_max_size=GridPixelSize(image_max_size);
}

void ViewPort::find_viewport_blit(TextureGrid* const texture_grid,
                                  TextureOverlay* const texture_overlay,
                                  SDLApp* const sdl_app) {
  // locking the textures
  // Stores the next items to be blit to the viewport.
  std::vector<std::unique_ptr<BlitItem>> blititems;
  // lifted out of the old find_viewport_extents_grid function since it's only called from here
  // will be put into function eventually, but I wanted to use new data structures
  // I don't use objects for some things because I want to use FLOAT_T for intermediate calculations

  auto max_zoom_index=texture_grid->textures_zoom_index_length()-1;
  // TODO refactor this out
  if (max_zoom_index < 0) { max_zoom_index=0; }
  auto zoom_index=ViewPortTransferState::find_zoom_index_bounded(this->_zoom,0,max_zoom_index);
  ////////////////////////////////////////////////////////////////////////////////
  // now loop over grid squares
  // get the lock on the overlay
  // TODO: for now don't display if I can't get it
  std::unique_lock<std::mutex> overlay_lock(texture_overlay->display_mutex, std::defer_lock);
  if (overlay_lock.try_lock()) {
    for (INT_T i=0L; i < texture_grid->grid_image_size().wimage(); i++) {
      for (INT_T j=0L; j < texture_grid->grid_image_size().himage(); j++) {
        auto gi=j*texture_grid->grid_image_size().wimage()+i;
        auto upperleft_gridsquare=GridCoordinate(i,j);
        // auto viewport_pixel_0_grid=GridCoordinate(viewport_left_grid,viewport_top_grid);
        auto pixel_0=ViewportPixelCoordinate(0,0);
        auto viewport_pixel_0_grid=GridCoordinate(pixel_0,
                                                  this->_zoom,
                                                  this->_viewport_pixel_size,
                                                  this->_viewport_grid,
                                                  this->_image_max_size);
        auto new_viewport_pixel_size=ViewportPixelSize(this->_image_max_size.wpixel(),this->_image_max_size.hpixel());
        auto viewport_pixel_coordinate=ViewportPixelCoordinate(upperleft_gridsquare,this->_zoom,viewport_pixel_0_grid,new_viewport_pixel_size);
        // TODO: this is where I chose zoom
        auto actual_zoom=zoom_index;
        // for testing max zoom
        bool texture_loaded=false;
        do {
          auto texture_square_zoom=texture_grid->squares[i][j]->texture_array[actual_zoom].get();
          // filler should never get past here
          if (texture_square_zoom->is_loaded &&
              texture_square_zoom->is_displayable) {
            // TODO: would like more RAII way of dealing with this mutex
            if (texture_square_zoom->display_mutex.try_lock()) {
              if (texture_square_zoom->is_loaded &&
                  texture_square_zoom->is_displayable &&
                  texture_square_zoom->display_texture_wrapper()->is_valid()) {
                texture_loaded=true;
                auto grid_image_size_zoomed=ViewportPixelSize((int)round(this->_image_max_size.wpixel()*this->_zoom),
                                                              (int)round(this->_image_max_size.hpixel()*this->_zoom));
                auto new_blit_item=std::make_unique<BlitItem>(texture_square_zoom,
                                                              gi,
                                                              viewport_pixel_coordinate,
                                                              grid_image_size_zoomed);
                blititems.push_back(std::move(new_blit_item));
              } else {
                texture_square_zoom->display_mutex.unlock();
                MSG("Couldn't lock normal: " << i << " " << j);
                texture_loaded=false;
              }
            }
          }
          // TODO: else raise error if things are terrible
          actual_zoom++;
        } while ((actual_zoom <= max_zoom_index) && !texture_loaded);
        // texture didn't load load so do filler
        if (!texture_loaded) {
          // go through loop again
          auto texture_square_zoom=texture_grid->squares[i][j]->texture_array[zoom_index].get();
          if (texture_square_zoom->get_image_filler()) {
            // TODO: would like more RAII way of dealing with this mutex
            if (texture_square_zoom->display_mutex.try_lock()) {
              if (texture_square_zoom->get_image_filler() &&
                  texture_square_zoom->filler_texture_wrapper()->is_valid()) {
                auto grid_image_size_zoomed=ViewportPixelSize((int)round(this->_image_max_size.wpixel()*this->_zoom),
                                                              (int)round(this->_image_max_size.hpixel()*this->_zoom));
                auto new_blit_item=std::make_unique<BlitItem>(texture_square_zoom,
                                                              gi,
                                                              viewport_pixel_coordinate,
                                                              grid_image_size_zoomed);
                blititems.push_back(std::move(new_blit_item));
              } else {
                texture_square_zoom->display_mutex.unlock();
              }
            } else {
              // TODO: this should never happen, so this error is in here while I investigate
              MSG("Couldn't lock filler: " << i << " " << j);
            }
          }
        }
      }
    }
    // blit blitables
    auto viewport_pixel_size=ViewportPixelSize(this->_current_window_w,this->_current_window_h);
    std::unique_ptr<SDLDrawableSurface> drawable_surface=std::make_unique<SDLDrawableSurface>(sdl_app,viewport_pixel_size);
    for (size_t i=0; i < blititems.size(); i++) {
      blititems[i]->blit_this(drawable_surface.get());
      blititems[i]->blit_square->display_mutex.unlock();
    }
    // TODO: move sdl into it's own wrapper
    if (texture_overlay->overlay_message_surface()) {
      SDL_Rect message_rect;
      message_rect.x = OVERLAY_X;
      message_rect.y = OVERLAY_Y;
      message_rect.w =texture_overlay->overlay_message_surface()->w;
      message_rect.h =texture_overlay->overlay_message_surface()->h;
      SDL_BlitScaled(texture_overlay->overlay_message_surface(),
                     NULL,
                     drawable_surface->screen_surface(),
                     &message_rect);
    }
    overlay_lock.unlock();
  }
}

bool ViewPort::do_input(SDLApp* const sdl_app) {
  auto xgrid=this->_viewport_grid.xgrid();
  auto ygrid=this->_viewport_grid.ygrid();
  auto keep_going=sdl_app->do_input(this->_current_speed_x, this->_current_speed_y,
                                    this->_current_speed_zoom, this->_zoom, this->_zoom_speed,
                                    this->_image_max_size, xgrid, ygrid,
                                    this->_current_mouse_xpixel, this->_current_mouse_ypixel,
                                    this->_current_window_w,
                                    this->_current_window_h);
  this->update_viewport_info(xgrid,ygrid);
  return keep_going;
}

void ViewPort::update_viewport_info(FLOAT_T xgrid, FLOAT_T ygrid) {
  this->_viewport_grid=GridCoordinate(xgrid,ygrid);
  this->_viewport_pixel_size=ViewportPixelSize(this->_current_window_w,this->_current_window_h);
  // update the viewport
  //  TODO: too much duplicate code here
  this->_viewport_current_state_texturegrid_update->UpdateGridValues(this->_zoom,
                                                                     this->_viewport_grid,
                                                                     this->_image_max_size,
                                                                     this->_viewport_pixel_size,
                                                                     ViewportPixelCoordinate(this->_current_mouse_xpixel,
                                                                                             this->_current_mouse_ypixel),
                                                                     ViewportPixelCoordinate(this->_viewport_pixel_size.wpixel()/2,
                                                                                             this->_viewport_pixel_size.hpixel()/2));
  this->_viewport_current_state_imagegrid_update->UpdateGridValues(this->_zoom,
                                                                   this->_viewport_grid,
                                                                   this->_image_max_size,
                                                                   this->_viewport_pixel_size,
                                                                   ViewportPixelCoordinate(this->_current_mouse_xpixel,
                                                                                           this->_current_mouse_ypixel),
                                                                   ViewportPixelCoordinate(this->_viewport_pixel_size.wpixel()/2,
                                                                                           this->_viewport_pixel_size.hpixel()/2));

}

void ViewPort::adjust_initial_location(const GridSetup* const grid_setup) {
  // adjust initial position for small grids
  FLOAT_T new_xgrid,new_ygrid;
  if (grid_setup->grid_image_size().wimage() == 1) {
    new_xgrid=0.5;
  } else {
    new_xgrid=this->_viewport_grid.xgrid();
  }
  if (grid_setup->grid_image_size().himage() == 1) {
    new_ygrid=0.5;
  } else {
    new_ygrid=this->_viewport_grid.ygrid();
  }
  this->update_viewport_info(new_xgrid,new_ygrid);
}
