#include "config.hpp"
#include "debug.hpp"
#include "error.hpp"
#include "viewport.hpp"
// C++ headers
#include <vector>
#include <iostream>
#include <string>
using namespace std;
// C headers
#include <math.h>
// library headers
#include <SDL2/SDL.h>

BlitItem::BlitItem(TextureGridSquareZoomLevel* square, int count, float xpixel, float ypixel, float wpixel, float hpixel) {
  blit_index=count;
  blit_square=square;
  viewport_xpixel=xpixel;
  viewport_ypixel=ypixel;
  viewport_wpixel=wpixel;
  viewport_hpixel=hpixel;
}

BlitItem::~BlitItem() {

}

void BlitItem::blit_this(SDL_Surface* screen_surface) {
  /* Method to actually blit to the SDL surface */
  DEBUG("blit index: " << blit_index);
  DEBUG("Viewport x: " << viewport_xpixel << " Blit y: " << viewport_ypixel << " Viewport x size: " << viewport_wpixel << " Viewport y size: "  << viewport_hpixel);
  SDL_Rect scaled_rect;
  scaled_rect.x=(int)round(viewport_xpixel);
  scaled_rect.y=(int)round(viewport_ypixel);
  scaled_rect.w=(int)round(viewport_wpixel);
  scaled_rect.h=(int)round(viewport_hpixel);
  DEBUG("blitscaled()");
  SDL_BlitScaled(blit_square->display_texture, NULL, screen_surface, &scaled_rect);
  DEBUG("blitscaled() done");
}

ViewPortCurrentState::ViewPortCurrentState () {
  this->zoom=NAN;
  this->xgrid=NAN;
  this->ygrid=NAN;
  this->zoom_last=NAN;
  this->xgrid_last=NAN;
  this->ygrid_last=NAN;
  this->been_updated=false;
}

ViewPortCurrentState::~ViewPortCurrentState () {

}

void ViewPortCurrentState::UpdateGridValues(float zoom, float xgrid, float ygrid) {
  /* Update the values based the current state of the viewport. */
  lock_guard<std::mutex> guard(this->using_mutex);
  if (isnan(this->zoom_last) || isnan(this->xgrid_last) || isnan(this->xgrid_last) || zoom != this->zoom_last || xgrid != this->xgrid_last || ygrid != this->ygrid_last) {
    this->zoom=zoom;
    this->xgrid=xgrid;
    this->ygrid=ygrid;
    this->zoom_last=zoom;
    this->xgrid_last=xgrid;
    this->ygrid_last=ygrid;
    this->been_updated=true;
  }
}

bool ViewPortCurrentState::GetGridValues(float &zoom, float &xgrid, float &ygrid) {
  /* Get the current state of the viewport. */
  lock_guard<std::mutex> guard(this->using_mutex);
  if (this->been_updated) {
    zoom=this->zoom;
    xgrid=this->xgrid;
    ygrid=this->ygrid;
    this->been_updated=false;
    return true;
  } else {
    return false;
  }
}

ViewPort::ViewPort(ViewPortCurrentState *viewport_current_state) {
  this->viewport_current_state=viewport_current_state;

  screen_pixel_width=SCREEN_WIDTH;
  screen_pixel_height=SCREEN_HEIGHT;

  viewport_xgrid=1.0;
  viewport_ygrid=1.0;
  // 0.5 or 0.25 better
  zoom=1.0;
  zoom_speed=1.0;
  viewport_current_state->UpdateGridValues(zoom,viewport_xgrid,viewport_ygrid);
}

ViewPort::~ViewPort() {

}

void ViewPort::find_viewport_blit(TextureGrid* texture_grid, vector<BlitItem> &blititems,SDL_Surface* screen_surface, SDL_PixelFormat *format) {
  /* Find the textures that need to be blit to this viewport. */
  int blit_count=0;
  float viewport_left_grid, viewport_right_grid, viewport_top_grid, viewport_bottom_grid;
  // locking the textures
  vector<unique_lock<mutex>> mutex_vector{};
  find_viewport_extents_grid(zoom, viewport_left_grid, viewport_right_grid, viewport_top_grid, viewport_bottom_grid);
  int zoom_index=floor(log2(1.0/zoom));
  ////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////
  // now loop over grid squares
  blititems.clear();
  for (size_t i = 0; i < texture_grid->textures_wgrid; i++) {
    for (size_t j = 0; j < texture_grid->textures_hgrid; j++) {
      int gi=j*texture_grid->textures_wgrid+i;
      DEBUG("++ Adding Blittable " << blit_count << " at index " << gi << " ++++++++++++++++++++");
      float viewport_xpixel;
      float viewport_ypixel;
      grid_to_pixel(i,j,
                    viewport_left_grid,viewport_top_grid,
                    viewport_xpixel,
                    viewport_ypixel);
      // TODO: this is where I chose zoom
      int actual_zoom=zoom_index;
      // for testing max zoom
      // int actual_zoom=texture_grid->textures_max_zoom-1;
      DEBUG("Max zoom: " << texture_grid->textures_max_zoom << " zoom index:" << zoom_index);
      bool lock_succeeded=false;
      do {
        lock_succeeded=false;
        mutex_vector.emplace_back(std::unique_lock<std::mutex>{texture_grid->squares[i][j].image_array[actual_zoom]->display_mutex,std::defer_lock});
        if (mutex_vector.back().try_lock()) {
          lock_succeeded=true;
          if ( !(texture_grid->squares[i][j].image_array[actual_zoom]->display_texture == nullptr)) {
            BlitItem new_blit_item=BlitItem(texture_grid->squares[i][j].image_array[actual_zoom],
                                            gi,
                                            viewport_xpixel,viewport_ypixel,
                                            max_wpixel*zoom,max_hpixel*zoom);
            blititems.push_back(new_blit_item);
            DEBUG("blititem actual zoom: " << actual_zoom << " viewport pixel x: " << blititems[blit_count].viewport_xpixel << " viewport pixel y: " << blititems[blit_count].viewport_ypixel);
            DEBUG("blititem actual zoom: " << actual_zoom << " viewport pixel size x: " << blititems[blit_count].viewport_wpixel << " viewport pixel size y: " << blititems[blit_count].viewport_hpixel);
          }
        }
        if (!lock_succeeded) {
          mutex_vector.pop_back();
        }
        // TODO: else raise error if things are terrible
        actual_zoom+=1;
      } while ( !lock_succeeded && (texture_grid->squares[i][j].image_array[actual_zoom-1]->display_texture == nullptr) && (actual_zoom <= texture_grid->textures_max_zoom) );
      // TODO: actual zoom
      // TODO: not good debug anymore
      DEBUG("++ End Blittable");
      blit_count++;
    }
  }
  // blit blitables
  this->blank_viewport(screen_surface, format);
  for (size_t i = 0; i < blititems.size(); i++) {
    DEBUG("blit loop:" << i);
    blititems[i].blit_this(screen_surface);
  }
}

void ViewPort::find_viewport_extents_grid(float actual_zoom,float &viewport_left_grid, float &viewport_right_grid, float &viewport_top_grid, float &viewport_bottom_grid) {
  /* Find the coordinates of the viewport on the grid.  */
  float half_width, half_height, viewport_left_distance_grid, viewport_right_distance_grid, viewport_top_distance_grid, viewport_bottom_distance_grid, leftmost_grid, rightmost_grid, topmost_grid, bottommost_grid;
  // TODO: set these somewhereelse
  half_width = screen_pixel_width / 2.0;
  half_height = screen_pixel_height / 2.0;
  // find viewport dimensions in grid space
  viewport_left_distance_grid=(half_width/max_wpixel/actual_zoom);
  viewport_right_distance_grid=(half_width/max_wpixel/actual_zoom);
  viewport_top_distance_grid=(half_height/max_hpixel/actual_zoom);
  viewport_bottom_distance_grid=(half_height/max_hpixel/actual_zoom);
  viewport_left_grid=viewport_xgrid-viewport_left_distance_grid;
  viewport_right_grid=viewport_xgrid+viewport_right_distance_grid;
  viewport_top_grid=viewport_ygrid-viewport_top_distance_grid;
  viewport_bottom_grid=viewport_ygrid+viewport_bottom_distance_grid;
  // get a list of textures to blit
  // start at top left
  // leftmost_grid=floor(viewport_left_grid);
  // rightmost_grid=floor(viewport_right_grid);
  // topmost_grid=floor(viewport_top_grid);
  // bottommost_grid=floor(viewport_bottom_grid);
}

void ViewPort::grid_to_pixel(float xgrid, float ygrid, float pixel_0_xgrid, float pixel_0_ygrid, float &xpixel, float &ypixel) {
  /* Convert grid coordinates to pixel coordinates */
  DEBUG("grid_to_pixel() x: " << xgrid << " y: " << ygrid << " 0_x: " << pixel_0_xgrid << " 0_y: " << pixel_0_ygrid);
  xpixel=(xgrid - pixel_0_xgrid)*max_wpixel*zoom;
  ypixel=(ygrid - pixel_0_ygrid)*max_hpixel*zoom;
}

bool ViewPort::do_input() {
  /* Update the viewport based on inputs */
  SDL_Event e;
  bool keep_going=true;
  while (SDL_PollEvent(&e)) {
    float pixel_size=(100.0/max_wpixel/zoom);
    if (e.type == SDL_QUIT) {
      return false;
    } else if (e.type == SDL_JOYAXISMOTION) {
      // TODO: deal with dead zones
      if (e.jaxis.which == 0) {
        DEBUGIO("jaxis 0: " << e.jaxis.axis << " ++ " << e.jaxis.value);
        if (e.jaxis.axis == 0) {
          current_speed_x=(e.jaxis.value/JOY_MAX)*(JOY_BASE_MOVE*pixel_size);
        } else if (e.jaxis.axis == 1) {
          current_speed_y=(e.jaxis.value/JOY_MAX)*(JOY_BASE_MOVE*pixel_size);
        } else if (e.jaxis.axis == 4) {
          current_speed_zoom=0.05*(e.jaxis.value/JOY_MAX);
        }
      }
    } else if (e.type == SDL_KEYDOWN) {
      switch(e.key.keysym.sym) {
      case SDLK_LEFT:
        // move 5% of a screen left
        viewport_xgrid-=(KEY_PRESS_MOVE*pixel_size);
        break;

      case SDLK_RIGHT:
        // move 5% of a screen right
        viewport_xgrid+=(KEY_PRESS_MOVE*pixel_size);
        break;

      case SDLK_UP:
        // move 5% of a screen up
        viewport_ygrid-=(KEY_PRESS_MOVE*pixel_size);
        break;

      case SDLK_DOWN:
        // move 5% of a screen down
        viewport_ygrid+=(KEY_PRESS_MOVE*pixel_size);
        break;

      case SDLK_EQUALS:
      case SDLK_PLUS:
        zoom*=(1.2*zoom_speed);
        break;

      case SDLK_MINUS:
        zoom/=(1.2*zoom_speed);
        break;

      case SDLK_q:
        keep_going=false;
        break;

      default:
        break;
      }
    }
  }
  // speed
  viewport_xgrid+=(current_speed_x*JOY_BASE_MOVE);
  viewport_ygrid+=(current_speed_y*JOY_BASE_MOVE);
  zoom=(1.0-current_speed_zoom)*zoom;
  // update the viewport
  this->viewport_current_state->UpdateGridValues(zoom,viewport_xgrid,viewport_ygrid);
  return keep_going;
}

void ViewPort::blank_viewport(SDL_Surface* screen_surface, SDL_PixelFormat *format) {
  /* Blank the current viewport */
  SDL_Rect screen_rect;
  screen_rect.x=0;
  screen_rect.y=0;
  screen_rect.w=screen_pixel_width;
  screen_rect.h=screen_pixel_height;
  SDL_FillRect(screen_surface, &screen_rect, SDL_MapRGB(format,0,0,0));
}
