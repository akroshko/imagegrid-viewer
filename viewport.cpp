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

void BlitItem::blit_this(SDLApp* sdl_app) {
  DEBUG("blit index: " << blit_index);
  DEBUG("Viewport x: " << this->viewport_pixel_coordinate->xpixel << " Blit y: " << this->viewport_pixel_coordinate->ypixel
        << " Viewport x size: " << this->image_pixel_size_viewport->wpixel << " Viewport y size: "  << this->image_pixel_size_viewport->hpixel);
  SDL_Rect scaled_rect;
  scaled_rect.x=this->viewport_pixel_coordinate->xpixel();
  scaled_rect.y=this->viewport_pixel_coordinate->ypixel();
  scaled_rect.w=this->image_pixel_size_viewport->wpixel();
  scaled_rect.h=this->image_pixel_size_viewport->hpixel();
  DEBUG("blitscaled()");
  SDL_BlitScaled(blit_square->display_texture, NULL, sdl_app->screen_surface, &scaled_rect);
  DEBUG("blitscaled() done");
}

ViewPortCurrentState::ViewPortCurrentState () {
  this->zoom=NAN;
  this->grid = nullptr;
  this->zoom_last=NAN;
  this->grid_last = nullptr;
  this->been_updated=false;
}

void ViewPortCurrentState::UpdateGridValues(FLOAT_T zoom, GridCoordinate *gridarg) {
  std::lock_guard<std::mutex> guard(this->using_mutex);
  // (this->grid == nullptr)
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
  this->viewport_grid=new GridCoordinate(1.0,1.0);
  // 0.5 or 0.25 better
  this->zoom=1.0;
  this->zoom_speed=1.0;
  viewport_current_state->UpdateGridValues(zoom,this->viewport_grid);
}

INT_T ViewPort::find_zoom_index(FLOAT_T zoom) {
  return ::find_zoom_index(zoom);
}

void ViewPort::find_viewport_blit(TextureGrid* texture_grid, SDLApp* sdl_app) {
  INT_T blit_count=0;
  // locking the textures
  std::vector<std::unique_lock<std::mutex>> mutex_vector{};
  // lifted out of the old find_viewport_extents_grid function since it's only called from here
  // will be put into function eventually, but I wanted to use new data structures
  // I don't use objects for some things because I want to use FLOAT_T for intermediate calculations
  auto half_width = this->viewport_pixel_size->wpixel() / 2.0;
  auto half_height = this->viewport_pixel_size->hpixel() / 2.0;
  auto viewport_left_distance_grid=(half_width/this->image_max_size->wpixel()/zoom);
  auto viewport_top_distance_grid=(half_height/this->image_max_size->hpixel()/zoom);
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
      DEBUG("++ Adding Blittable " << blit_count << " at index " << gi << " ++++++++++++++++++++");
      auto upperleft_gridsquare=new GridCoordinate(i,j);
      auto viewport_pixel_0_grid=new GridCoordinate(viewport_left_grid,viewport_top_grid);
      auto new_viewport_pixel_size=new ViewportPixelSize(this->image_max_size->wpixel(),this->image_max_size->hpixel());
      auto viewport_cooridinate_pixel=new ViewportPixelCoordinate(upperleft_gridsquare,zoom,viewport_pixel_0_grid,new_viewport_pixel_size);
      // TODO: this is where I chose zoom
      auto actual_zoom=zoom_index;
      auto max_zoom=texture_grid->textures_max_zoom;
      auto max_zoom_index=max_zoom-1;
      if (actual_zoom > max_zoom_index) {
        actual_zoom=max_zoom_index;
      } else if (actual_zoom < 0) {
        actual_zoom=0;
      }
      // for testing max zoom
      // int actual_zoom=texture_grid->textures_max_zoom-1;
      DEBUG("Max zoom: " << max_zoom << " zoom index:" << zoom_index);
      auto lock_succeeded=false;
      bool texture_loaded;
      do {
        lock_succeeded=false;
        mutex_vector.emplace_back(std::unique_lock<std::mutex>{texture_grid->squares[i][j].texture_array[actual_zoom]->display_mutex,std::defer_lock});
        if (mutex_vector.back().try_lock()) {
          lock_succeeded=true;
          if ( !(texture_grid->squares[i][j].texture_array[actual_zoom]->display_texture == nullptr)) {
            texture_loaded=true;
            auto grid_image_size_zoomed=new ViewportPixelSize((int)round(this->image_max_size->wpixel()*zoom),(int)round(this->image_max_size->hpixel()*zoom));
            auto new_blit_item=BlitItem(texture_grid->squares[i][j].texture_array[actual_zoom],
                                        gi,
                                        viewport_cooridinate_pixel,
                                        grid_image_size_zoomed);
            this->blititems.push_back(new_blit_item);
            DEBUG("blititem actual zoom: " << actual_zoom << " viewport pixel x: " << this->blititems[blit_count].viewport_pixel_coordinate->xpixel << " viewport pixel y: " << blititems[blit_count].viewport_pixel_coordinate->ypixel);
            DEBUG("blititem actual zoom: " << actual_zoom << " viewport pixel size x: " << this->blititems[blit_count].image_pixel_size_viewport->wpixel << " viewport pixel size y: " << blititems[blit_count].image_pixel_size_viewport->hpixel);
          } else {
            texture_loaded=false;
          }
        }
        if (!lock_succeeded) {
          mutex_vector.pop_back();
        }
        // TODO: else raise error if things are terrible
        actual_zoom+=1;
      } while ( (actual_zoom <= max_zoom_index) && (!lock_succeeded || !texture_loaded)  );


      DEBUG("++ End Blittable");
      blit_count++;
    }
  }
  // blit blitables
  this->blank_viewport(sdl_app);
  for (INT_T i = 0; i < this->blititems.size(); i++) {
    DEBUG("blit loop:" << i);
    this->blititems[i].blit_this(sdl_app);
  }
  SDL_UpdateWindowSurface(sdl_app->window);
}

bool ViewPort::do_input(SDLApp* sdl_app) {
  auto xgrid=this->viewport_grid->xgrid();
  auto ygrid=this->viewport_grid->ygrid();
  auto keep_going = sdl_app->do_input(this->current_speed_x,this->current_speed_y,this->current_speed_zoom,this->zoom, this->zoom_speed, this->image_max_size, xgrid, ygrid);
  this->viewport_grid=new GridCoordinate(xgrid,ygrid);
  // update the viewport
  this->viewport_current_state->UpdateGridValues(zoom,this->viewport_grid);
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
}
