#include "debug.hpp"
#include "error.hpp"
#include "types.hpp"
#include "utility.hpp"
#include "gridsetup.hpp"
#include "coordinates.hpp"
#include "viewport.hpp"
#include "sdl.hpp"
// C++ headers
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

BlitItem::BlitItem(TextureGridSquareZoomLevel* square, INT_T count, const ViewportPixelCoordinate &viewport_pixel_coordinate, const ViewportPixelSize &grid_image_size_zoomed) {
  blit_index=count;
  blit_square=square;
  this->viewport_pixel_coordinate=ViewportPixelCoordinate(viewport_pixel_coordinate);
  this->image_pixel_size_viewport=ViewportPixelSize(grid_image_size_zoomed);
}

BlitItem::~BlitItem () {
  // if (this->viewport_pixel_coordinate != nullptr) {
  //   delete this->viewport_pixel_coordinate;
  //   this->viewport_pixel_coordinate=nullptr;
  // }
  // if (this->image_pixel_size_viewport != nullptr) {
  //   delete this->image_pixel_size_viewport;
  //   this->image_pixel_size_viewport=nullptr;
  // }
}

void BlitItem::blit_this(SDLApp* sdl_app) {
  SDL_Rect scaled_rect;
  scaled_rect.x=this->viewport_pixel_coordinate.xpixel();
  scaled_rect.y=this->viewport_pixel_coordinate.ypixel();
  scaled_rect.w=this->image_pixel_size_viewport.wpixel();
  scaled_rect.h=this->image_pixel_size_viewport.hpixel();
  SDL_BlitScaled(blit_square->display_texture, NULL, sdl_app->screen_surface, &scaled_rect);
}

ViewPort::ViewPort(std::shared_ptr<ViewPortCurrentState> viewport_current_state_texturegrid_update,
                   std::shared_ptr<ViewPortCurrentState> viewport_current_state_imagegrid_update) {
  this->viewport_current_state_texturegrid_update=viewport_current_state_texturegrid_update;
  this->viewport_current_state_imagegrid_update=viewport_current_state_imagegrid_update;
  this->viewport_pixel_size=ViewportPixelSize(SCREEN_WIDTH,SCREEN_HEIGHT);
  this->update_viewport_info(INITIAL_X,INITIAL_Y);
}

INT_T ViewPort::find_zoom_index(FLOAT_T zoom) {
  return ::find_zoom_index(zoom);
}

void ViewPort::set_image_max_size(const GridPixelSize& image_max_size) {
  this->_image_max_size=GridPixelSize(image_max_size);
}

void ViewPort::find_viewport_blit(TextureGrid* texture_grid, SDLApp* sdl_app) {
  INT_T blit_count=0;
  // locking the textures
  std::vector<std::unique_lock<std::mutex>> mutex_vector{};
  // Stores the next items to be blit to the viewport.
  std::vector<std::unique_ptr<BlitItem>> blititems;
  // lifted out of the old find_viewport_extents_grid function since it's only called from here
  // will be put into function eventually, but I wanted to use new data structures
  // I don't use objects for some things because I want to use FLOAT_T for intermediate calculations
  auto half_width=this->viewport_pixel_size.wpixel() / 2.0;
  auto half_height=this->viewport_pixel_size.hpixel() / 2.0;
  auto viewport_left_distance_grid=(half_width/this->_image_max_size.wpixel()/this->zoom);
  auto viewport_top_distance_grid=(half_height/this->_image_max_size.hpixel()/this->zoom);
  auto viewport_left_grid=this->viewport_grid.xgrid()-viewport_left_distance_grid;;
  auto viewport_top_grid=this->viewport_grid.ygrid()-viewport_top_distance_grid;
  auto zoom_index=this->find_zoom_index(this->zoom);
  ////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////
  // now loop over grid squares
  for (INT_T i=0l; i < texture_grid->grid_image_size.wimage(); i++) {
    for (INT_T j=0l; j < texture_grid->grid_image_size.himage(); j++) {
      auto gi=j*texture_grid->grid_image_size.wimage()+i;
      auto upperleft_gridsquare=GridCoordinate(i,j);
      auto viewport_pixel_0_grid=GridCoordinate(viewport_left_grid,viewport_top_grid);
      auto new_viewport_pixel_size=ViewportPixelSize(this->_image_max_size.wpixel(),this->_image_max_size.hpixel());
      auto viewport_pixel_coordinate=ViewportPixelCoordinate(upperleft_gridsquare,this->zoom,viewport_pixel_0_grid,new_viewport_pixel_size);
      // TODO: this is where I chose zoom
      auto actual_zoom=zoom_index;
      auto max_zoom=texture_grid->textures_max_zoom_index;
      auto max_zoom_index=max_zoom-1;
      if (actual_zoom > max_zoom_index) {
        actual_zoom=max_zoom_index;
      } else if (actual_zoom < 0) {
        actual_zoom=0;
      }
      // for testing max zoom
      // int actual_zoom=texture_grid->textures_max_zoom_index-1;
      auto lock_succeeded=false;
      bool texture_loaded=false;
      do {
        lock_succeeded=false;
        texture_loaded=false;
        mutex_vector.emplace_back(std::unique_lock<std::mutex>{texture_grid->squares[i][j].texture_array[actual_zoom]->display_mutex,std::defer_lock});
        if (mutex_vector.back().try_lock()) {
          lock_succeeded=true;
          if ( texture_grid->squares[i][j].texture_array[actual_zoom]->display_texture != nullptr) {
            texture_loaded=true;
            auto grid_image_size_zoomed=ViewportPixelSize((int)round(this->_image_max_size.wpixel()*zoom),
                                                          (int)round(this->_image_max_size.hpixel()*zoom));
            auto new_blit_item=std::make_unique<BlitItem>(texture_grid->squares[i][j].texture_array[actual_zoom],
                                                          gi,
                                                          viewport_pixel_coordinate,
                                                          grid_image_size_zoomed);
            blititems.push_back(std::move(new_blit_item));
          } else {
            texture_loaded=false;
            mutex_vector.back().unlock();
            mutex_vector.pop_back();
          }
        }
        if (!lock_succeeded) {
          mutex_vector.pop_back();
        }
        // TODO: else raise error if things are terrible
        actual_zoom++;

      } while ( (actual_zoom <= max_zoom_index) && (!lock_succeeded || !texture_loaded)  );
      blit_count++;
    }
  }
  // blit blitables
  // TODO should I clear the vector?
  this->blank_viewport(sdl_app);
  for (size_t i=0; i < blititems.size(); i++) {
    blititems[i]->blit_this(sdl_app);
  }
  SDL_UpdateWindowSurface(sdl_app->window);
  for (auto & m : mutex_vector) {
    m.unlock();
  }
}

bool ViewPort::do_input(SDLApp* sdl_app) {
  auto xgrid=this->viewport_grid.xgrid();
  auto ygrid=this->viewport_grid.ygrid();
  auto keep_going=sdl_app->do_input(this->current_speed_x, this->current_speed_y, this->current_speed_zoom,this->zoom, this->zoom_speed, this->_image_max_size, xgrid, ygrid);
  this->update_viewport_info(xgrid,ygrid);
  return keep_going;
}

void ViewPort::update_viewport_info(FLOAT_T xgrid, FLOAT_T ygrid) {
  this->viewport_grid=GridCoordinate(xgrid,ygrid);
  // update the viewport
  this->viewport_current_state_texturegrid_update->UpdateGridValues(this->zoom,this->viewport_grid);
  this->viewport_current_state_imagegrid_update->UpdateGridValues(this->zoom,this->viewport_grid);
}

void ViewPort::blank_viewport(SDLApp* sdl_app) {
  sdl_app->blank_viewport(viewport_pixel_size);
}

void ViewPort::adjust_initial_location(GridSetup *grid_setup) {
  // adjust initial position for small grids
  FLOAT_T new_xgrid,new_ygrid;
  if (grid_setup->grid_image_size.wimage() == 1) {
    new_xgrid=0.5;
  } else {
    new_xgrid=this->viewport_grid.xgrid();
  }
  if (grid_setup->grid_image_size.himage() == 1) {
    new_ygrid=0.5;
  } else {
    new_ygrid=this->viewport_grid.ygrid();
  }
  this->update_viewport_info(new_xgrid,new_ygrid);
}
