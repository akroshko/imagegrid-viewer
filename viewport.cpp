#include "debug.hpp"
#include "error.hpp"
#include "types.hpp"
#include "utility.hpp"
#include "gridsetup.hpp"
#include "coordinates.hpp"
#include "viewport.hpp"
#include "sdl.hpp"
// C++ headers
#include <vector>
#include <iostream>
#include <string>

BlitItem::BlitItem(TextureGridSquareZoomLevel* square, INT_T count, ViewportPixelCoordinate viewport_pixel_coordinate, ViewportPixelSize grid_image_size_zoomed) {
  blit_index=count;
  blit_square=square;
  this->viewport_pixel_coordinate=new ViewportPixelCoordinate(viewport_pixel_coordinate);
  this->image_pixel_size_viewport=new ViewportPixelSize(grid_image_size_zoomed);
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
  DEBUG("BlitItem::blit_this() blit index: " << blit_index);
  DEBUG("Viewport x: " << this->viewport_pixel_coordinate->xpixel() << " Blit y: " << this->viewport_pixel_coordinate->ypixel()
        << " Viewport x size: " << this->image_pixel_size_viewport->wpixel() << " Viewport y size: "  << this->image_pixel_size_viewport->hpixel());
  SDL_Rect scaled_rect;
  scaled_rect.x=this->viewport_pixel_coordinate->xpixel();
  scaled_rect.y=this->viewport_pixel_coordinate->ypixel();
  scaled_rect.w=this->image_pixel_size_viewport->wpixel();
  scaled_rect.h=this->image_pixel_size_viewport->hpixel();
  SDL_BlitScaled(blit_square->display_texture, NULL, sdl_app->screen_surface, &scaled_rect);
  DEBUG("BlitItem::blit_this() done");
}

ViewPortCurrentState::ViewPortCurrentState () {
  this->zoom=NAN;
  this->grid = nullptr;
  this->zoom_last=NAN;
  this->grid_last = nullptr;
  this->been_updated=false;
}

ViewPortCurrentState::~ViewPortCurrentState () {
  if (this->grid != nullptr) {
    delete this->grid;
    this->grid=nullptr;
  }
  if (this->grid_last != nullptr) {
    delete this->grid_last;
    this->grid_last=nullptr;
  }
}

void ViewPortCurrentState::UpdateGridValues(FLOAT_T zoom, GridCoordinate *gridarg) {
  std::lock_guard<std::mutex> guard(this->using_mutex);
  if (isnan(this->zoom_last) || (this->grid_last == nullptr) || (zoom != this->zoom_last) || (gridarg->xgrid() != this->grid_last->xgrid()) || (gridarg->ygrid() != this->grid_last->ygrid())) {
    this->zoom=zoom;
    this->grid=new GridCoordinate(gridarg);
    this->zoom_last=zoom;
    this->grid_last=new GridCoordinate(gridarg);
    this->been_updated=true;
  }
}

bool ViewPortCurrentState::GetGridValues(FLOAT_T &zoom, GridCoordinate *&gridarg) {
  std::lock_guard<std::mutex> guard(this->using_mutex);
  if (this->been_updated) {
    zoom=this->zoom;
    gridarg=new GridCoordinate(this->grid);
    this->been_updated=false;
    return true;
  } else {
    return false;
  }
}

ViewPort::ViewPort(ViewPortCurrentState *viewport_current_state) {
  this->viewport_current_state=viewport_current_state;
  this->viewport_pixel_size=new ViewportPixelSize(SCREEN_WIDTH,SCREEN_HEIGHT);
  this->viewport_grid=new GridCoordinate(INITIAL_X,INITIAL_Y);
  this->viewport_current_state->UpdateGridValues(this->zoom,this->viewport_grid);
}

ViewPort::~ViewPort() {
  if (this->viewport_pixel_size != nullptr) {
    delete this->viewport_pixel_size;
    this->viewport_pixel_size=nullptr;
  }
  if (this->viewport_grid != nullptr) {
    delete this->viewport_grid;
    this->viewport_grid=nullptr;
  }
  if (this->_image_max_size != nullptr) {
    delete this->_image_max_size;
    this->_image_max_size=nullptr;
  }
}

INT_T ViewPort::find_zoom_index(FLOAT_T zoom) {
  return ::find_zoom_index(zoom);
}

void ViewPort::set_image_max_size(GridPixelSize * image_max_size) {
  this->_image_max_size=new GridPixelSize(image_max_size);
}

void ViewPort::find_viewport_blit(TextureGrid* texture_grid, SDLApp* sdl_app) {
  DEBUG("ViewPort::find_viewport_blit()");
  INT_T blit_count=0;
  // locking the textures
  std::vector<std::unique_lock<std::mutex>> mutex_vector{};
  // lifted out of the old find_viewport_extents_grid function since it's only called from here
  // will be put into function eventually, but I wanted to use new data structures
  // I don't use objects for some things because I want to use FLOAT_T for intermediate calculations
  auto half_width = this->viewport_pixel_size->wpixel() / 2.0;
  auto half_height = this->viewport_pixel_size->hpixel() / 2.0;
  auto viewport_left_distance_grid=(half_width/this->_image_max_size->wpixel()/this->zoom);
  auto viewport_top_distance_grid=(half_height/this->_image_max_size->hpixel()/this->zoom);
  auto viewport_left_grid = this->viewport_grid->xgrid()-viewport_left_distance_grid;;
  auto viewport_top_grid = this->viewport_grid->ygrid()-viewport_top_distance_grid;
  auto zoom_index=this->find_zoom_index(this->zoom);
  ////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////
  // now loop over grid squares
  this->blititems.clear();
  for (INT_T i = 0; i < texture_grid->grid_image_size->wimage(); i++) {
    for (INT_T j = 0; j < texture_grid->grid_image_size->himage(); j++) {
      auto gi=j*texture_grid->grid_image_size->wimage()+i;
      DEBUG("== Adding Blittable " << blit_count << " at index " << gi << " ==========");
      auto upperleft_gridsquare=new GridCoordinate(i,j);
      auto viewport_pixel_0_grid=new GridCoordinate(viewport_left_grid,viewport_top_grid);
      auto new_viewport_pixel_size=new ViewportPixelSize(this->_image_max_size->wpixel(),this->_image_max_size->hpixel());
      auto viewport_pixel_coordinate=new ViewportPixelCoordinate(upperleft_gridsquare,this->zoom,viewport_pixel_0_grid,new_viewport_pixel_size);
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
      DEBUG("Max zoom: " << max_zoom << " zoom index:" << zoom_index);
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
            auto grid_image_size_zoomed=new ViewportPixelSize((int)round(this->_image_max_size->wpixel()*zoom),
                                                              (int)round(this->_image_max_size->hpixel()*zoom));
            auto new_blit_item=BlitItem(texture_grid->squares[i][j].texture_array[actual_zoom],
                                        gi,
                                        viewport_pixel_coordinate,
                                        grid_image_size_zoomed);
            this->blititems.push_back(new_blit_item);
            DEBUG("blititem actual zoom: " << actual_zoom << " viewport pixel x: " << this->blititems.back().viewport_pixel_coordinate->xpixel() << " viewport pixel y: " << blititems.back().viewport_pixel_coordinate->ypixel());
            DEBUG("blititem actual zoom: " << actual_zoom << " viewport pixel size x: " << this->blititems.back().image_pixel_size_viewport->wpixel() << " viewport pixel size y: " << blititems.back().image_pixel_size_viewport->hpixel());
          } else {
            DEBUG("Texture failed in ViewPort::find_viewport_blit at zoom level: " << actual_zoom);
            texture_loaded=false;
            mutex_vector.back().unlock();
            mutex_vector.pop_back();
          }
        }
        if (!lock_succeeded) {
          DEBUG("Lock failed in ViewPort::find_viewport_blit at zoom level: " << actual_zoom);
          mutex_vector.pop_back();
        }
        // TODO: else raise error if things are terrible
        actual_zoom+=1;

      } while ( (actual_zoom <= max_zoom_index) && (!lock_succeeded || !texture_loaded)  );
      // clean up
      delete upperleft_gridsquare;
      delete viewport_pixel_0_grid;
      delete new_viewport_pixel_size;
      delete viewport_pixel_coordinate;
      DEBUG("++ End Blittable");
      blit_count++;
    }
  }
  // blit blitables
  DEBUG("ViewPort::find_viewport_blit() blitting");
  this->blank_viewport(sdl_app);
  for (INT_T i = 0; i < this->blititems.size(); i++) {
    this->blititems[i].blit_this(sdl_app);
  }
  SDL_UpdateWindowSurface(sdl_app->window);
  for (auto & m : mutex_vector) {
    m.unlock();
  }
  DEBUG("ViewPort::find_viewport_blit() end");
}

bool ViewPort::do_input(SDLApp* sdl_app) {
  auto xgrid=this->viewport_grid->xgrid();
  auto ygrid=this->viewport_grid->ygrid();
  auto keep_going = sdl_app->do_input(this->current_speed_x, this->current_speed_y, this->current_speed_zoom,this->zoom, this->zoom_speed, this->_image_max_size, xgrid, ygrid);
  this->viewport_grid=new GridCoordinate(xgrid,ygrid);
  // update the viewport
  this->viewport_current_state->UpdateGridValues(this->zoom,this->viewport_grid);
  return keep_going;
}

void ViewPort::blank_viewport(SDLApp* sdl_app) {
  sdl_app->blank_viewport(viewport_pixel_size);
}

void ViewPort::adjust_initial_location(GridSetup *grid_setup) {
  // adjust initial position for small grids
  FLOAT_T new_xgrid,new_ygrid;
  if (grid_setup->grid_image_size->wimage() == 1) {
    new_xgrid = 0.5;
  } else {
    new_xgrid=this->viewport_grid->xgrid();
  }
  if (grid_setup->grid_image_size->himage() == 1) {
    new_ygrid = 0.5;
  } else {
    new_ygrid=this->viewport_grid->ygrid();
  }
  this->viewport_grid=new GridCoordinate(new_xgrid,new_ygrid);
  this->viewport_current_state->UpdateGridValues(this->zoom,this->viewport_grid);
}
