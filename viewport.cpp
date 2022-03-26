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

BlitItem::BlitItem(SDL_Surface* surface, int count, float xpixel, float ypixel, float wpixel, float hpixel) {
  blit_index=count;
  blit_surface=surface;
  viewport_xpixel=xpixel;
  viewport_ypixel=ypixel;
  viewport_wpixel=wpixel;
  viewport_hpixel=hpixel;
}

BlitItem::~BlitItem() {

}

void BlitItem::blit_this(SDL_Surface* screen_surface) {
  DEBUG("blit index: " << blit_index);
  DEBUG("Viewport x: " << viewport_xpixel << " Blit y: " << viewport_ypixel << " Viewport x size: " << viewport_wpixel << " Viewport y size: "  << viewport_hpixel);
  SDL_Rect scaled_rect;
  scaled_rect.x=(int)round(viewport_xpixel);
  scaled_rect.y=(int)round(viewport_ypixel);
  scaled_rect.w=(int)round(viewport_wpixel);
  scaled_rect.h=(int)round(viewport_hpixel);
  DEBUG("blitscaled()");
  SDL_BlitScaled(blit_surface, NULL, screen_surface, &scaled_rect);
  DEBUG("blitscaled() done");
}

ViewPort::ViewPort() {
  screen_pixel_width=SCREEN_WIDTH;
  screen_pixel_height=SCREEN_HEIGHT;

  viewport_xgrid=1.0;
  viewport_ygrid=1.0;
  // 0.5 or 0.25 better
  zoom=1.0;
  zoom_speed=1.0;
}

ViewPort::~ViewPort() {

}

void ViewPort::find_viewport(ImageGrid *grid, TextureGrid* texture_grid, vector<BlitItem> &blititems) {
  float half_width, half_height, viewport_left_distance_grid, viewport_right_distance_grid, viewport_top_distance_grid, viewport_bottom_distance_grid, viewport_left_grid, viewport_right_grid, viewport_top_grid, viewport_bottom_grid, leftmost_grid, rightmost_grid, topmost_grid, bottommost_grid;
  max_wpixel=texture_grid->textures_max_wpixel;
  max_hpixel=texture_grid->textures_max_hpixel;

  DEBUG("max_grid_width: " << max_wpixel);
  DEBUG("max_grid_height: " << max_hpixel);
  int blit_count=0;
  int zoom_index=floor(log2(1.0/zoom));
  DEBUG("zoom: " << zoom << " zoom index 1: " << zoom_index << " max: " << texture_grid->textures_max_zoom);
  if (zoom_index >= (texture_grid->textures_max_zoom - 1)) {
    zoom_index=(texture_grid->textures_max_zoom-1);
  }
  if (zoom_index < 0) {
    zoom_index=0;
  }
  DEBUG("zoom index 2: " << zoom_index);
  find_viewport_extents_grid(texture_grid, zoom, half_width, half_height, viewport_left_distance_grid, viewport_right_distance_grid, viewport_top_distance_grid, viewport_bottom_distance_grid, viewport_left_grid, viewport_right_grid, viewport_top_grid, viewport_bottom_grid, leftmost_grid, rightmost_grid, topmost_grid, bottommost_grid);
  ////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////
  // load/unload appopriate textures
  int max_zoom=texture_grid->textures_max_zoom;
  DEBUG("max_zoom: " << max_zoom);
  for (int z = 0; z < max_zoom; z++) {
    // only load/update current zoom, current zoom+-1 and max_zoom
    // this keeps things smoother
    if (z != max_zoom && z != zoom_index-1 && z !=zoom_index && z != zoom_index+1 ) {
      continue;
    }
    // calculate these with max resolution, rather than actual viewport
    // TODO: simplify the math
    // int max_zoom_this_level=1.0/(pow(2,(ceil(log2(1.0/zoom)))));
    float max_zoom_this_level=1.0/(2.0+z);
    float max_zoom_left=viewport_xgrid-((MAX_SCREEN_WIDTH/2)/max_wpixel/max_zoom_this_level);
    float max_zoom_right=viewport_xgrid+((MAX_SCREEN_WIDTH/2)/max_wpixel/max_zoom_this_level);
    float max_zoom_top=viewport_ygrid-((MAX_SCREEN_HEIGHT/2)/max_hpixel/max_zoom_this_level);
    float max_zoom_bottom=viewport_ygrid+((MAX_SCREEN_HEIGHT/2)/max_hpixel/max_zoom_this_level);
    float threshold=2.0;
    // find out if this is 3x3 or full
    // edge is less than 0.5*threshold away from center
    // TODO: have better thresholds to stop jumping
    DEBUG("zoom level: " << z << " viewport_xgrid: " << viewport_xgrid << " viewport_ygrid: " << viewport_ygrid << " max_zoom_this_level: " << max_zoom_this_level << " max_zoom_left: " << max_zoom_left << " max_zoom_right: " << max_zoom_right << " max_zoom_top: " << max_zoom_top << " max_zoom_bottom: " << max_zoom_bottom);
    if (!(z+1 == max_zoom) && (abs(max_zoom_left - viewport_xgrid) <= 0.5*threshold) && (abs(max_zoom_top - viewport_ygrid) <= 0.5*threshold)) {
      // can load a 3x3 grid
      DEBUG("Updating textures to 3x3 at zoom level: " << z);
      texture_grid->update_textures(grid, viewport_xgrid, viewport_ygrid, z, false);
    } else {
      // always load everthing for last zoom level
      // just make sure everything is loaded for this zoom level
      DEBUG("Updating textures to full at zoom level: " << z);
      texture_grid->update_textures(grid, viewport_xgrid, viewport_ygrid, z, true);
    }
  }
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
      DEBUG("Max zoom: " << texture_grid->textures_max_zoom << " zoom index:" << zoom_index);
      do {
        if ( !(texture_grid->squares[i][j].image_array[actual_zoom] == nullptr)) {
          blititems.push_back(BlitItem(texture_grid->squares[i][j].image_array[actual_zoom],
                                       gi,
                                       viewport_xpixel,viewport_ypixel,
                                       max_wpixel*zoom,
                                       max_hpixel*zoom));
          DEBUG("blititem actual zoom: " << actual_zoom << " viewport pixel x: " << blititems[blit_count].viewport_xpixel << " viewport pixel y: " << blititems[blit_count].viewport_ypixel);
          DEBUG("blititem actual zoom: " << actual_zoom << " viewport pixel size x: " << blititems[blit_count].viewport_wpixel << " viewport pixel size y: " << blititems[blit_count].viewport_hpixel);
        }
        // TODO: else raise error if things are terrible
        actual_zoom+=1;
      } while ( (texture_grid->squares[i][j].image_array[actual_zoom-1] == nullptr) && (actual_zoom <= texture_grid->textures_max_zoom) );
      // TODO: actual zoom
      // TODO: not good debug anymore
      DEBUG("++ End Blittable");
      blit_count++;
    }
  }
}

void ViewPort::find_viewport_extents_grid(TextureGrid* texture_grid, float actual_zoom, float &half_width, float &half_height, float &viewport_left_distance_grid, float &viewport_right_distance_grid, float &viewport_top_distance_grid, float &viewport_bottom_distance_grid, float &viewport_left_grid, float &viewport_right_grid, float &viewport_top_grid, float &viewport_bottom_grid, float &leftmost_grid, float &rightmost_grid, float &topmost_grid, float &bottommost_grid) {
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
  // ???
  // get a list of textures to blit
  // start at top left
  leftmost_grid=floor(viewport_left_grid);
  rightmost_grid=floor(viewport_right_grid);
  topmost_grid=floor(viewport_top_grid);
  bottommost_grid=floor(viewport_bottom_grid);
}

void ViewPort::grid_to_pixel(float xgrid, float ygrid, float pixel_0_xgrid, float pixel_0_ygrid, float &xpixel, float &ypixel) {
  DEBUG("grid_to_pixel() x: " << xgrid << " y: " << ygrid << " 0_x: " << pixel_0_xgrid << " 0_y: " << pixel_0_ygrid);
  xpixel=(xgrid - pixel_0_xgrid)*max_wpixel*zoom;
  ypixel=(ygrid - pixel_0_ygrid)*max_hpixel*zoom;
}

bool ViewPort::do_input() {
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
  return keep_going;
}

void ViewPort::blank_viewport(SDL_Surface* screen_surface, SDL_PixelFormat *format) {
  SDL_Rect screen_rect;
  screen_rect.x=0;
  screen_rect.y=0;
  screen_rect.w=screen_pixel_width;
  screen_rect.h=screen_pixel_height;
  SDL_FillRect(screen_surface, &screen_rect, SDL_MapRGB(format,0,0,0));
}
