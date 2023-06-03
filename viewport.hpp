#ifndef VIEWPORT_HPP

#define VIEWPORT_HPP

// local headers
#include "config.hpp"
#include "debug.hpp"
#include "error.hpp"
#include "defaults.hpp"
#include "gridclasses.hpp"
// C++ headers
#include <vector>
#include <iostream>
#include <string>
#include <mutex>

// library headers
#include <SDL2/SDL.h>

class BlitItem {
  /* Class that stores an item that ready to be blit to the screen. */
public:
  BlitItem(TextureGridSquareZoomLevel* square, int count, float xpixel, float ypixel, float wpixel, float hpixel);
  ~BlitItem();
  // the square that will be blit to the screen
  TextureGridSquareZoomLevel* blit_square;
  // just counting the number of items to be blit, mostly for debugging
  int blit_index;
  // the x location on the viewport that this texture is being blit to
  float viewport_xpixel;
  // the y location on the viewport that this texture is being blit to
  float viewport_ypixel;
  // the width on the viewport of this texture
  float viewport_wpixel;
  // the height on the viewport of this texture
  float viewport_hpixel;
  // method that does the blitting
  void blit_this(SDL_Surface* screen_surface);
};

// TODO: convert to threadsafe singleton
class ViewPortCurrentState {
  /* Class for transfering the current state of the viewport between
   * threads. Updated every single time the viewport changes.  Should
   * be a singleton class that is only meant to be produced in one
   * place and consumed in another.
   */
public:
  ViewPortCurrentState();
  ~ViewPortCurrentState();

  void UpdateGridValues(float zoom, float xgrid, float ygrid);

  bool GetGridValues(float &zoom, float &xgrid, float &ygrid);

private:
  float zoom=NAN;
  float xgrid=NAN;
  float ygrid=NAN;

  float zoom_last=NAN;
  float xgrid_last=NAN;
  float ygrid_last=NAN;

  bool been_updated=false;

  std::mutex using_mutex;
};


class ViewPort {
  /* Class that represents a viewport. */
public:
  ViewPort(ViewPortCurrentState *viewport_current_state);
  ~ViewPort();
  // the current width of the window in pixels
  float screen_pixel_width;
  // the current height of the window in pxiels
  float screen_pixel_height;
  // max width an image can be
  float max_wpixel;
  // max height an image can be
  float max_hpixel;
  // the x grid coordinates of the center of the viewport
  float viewport_xgrid=0.0;
  // the y grid coordinates of the center of the viewport
  float viewport_ygrid=0.0;
  // the zoom level, 1.0 indicates all pixels are 1:1, 0.5 indicates zoomed out by a factor of 2
  float zoom=1.0;
  // a zoom speed per SDL frame
  float zoom_speed=0.0;
  // the current x speed of movement per SDL frame
  float current_speed_x = 0.0;
  // the current y speed of movement per SDL frame
  float current_speed_y = 0.0;
  // the current speed of zoom per SDL frame
  float current_speed_zoom = 0.0;

  // object for transfering the state of the viewport in a threadsafe manner
  ViewPortCurrentState *viewport_current_state;
  void find_viewport_blit(TextureGrid* texture_grid, std::vector<BlitItem> &blititems, SDL_Surface* screen_surface, SDL_PixelFormat *format);
  // convert grid coordinates to pixel coordinates based on the coordinates of an origin for pixel (0,0)
  //   grid_x: the x of the grid coordinate to convert
  //   grid_y: the y of the grid coordinate to convert
  //   pixel_0_grid_x: the x of the origin of the pixel coordinates
  //   pixel_0_grid_y: the y of the origin of the pixel coordinates
  //   pixel_x: reference that is updated with x value of pixel coordinate of (grid_x,grid_y)
  //   pixel_y: reference that is updated with y value of pixel coordinate of (grid_x,grid_y)
  void grid_to_pixel(float grid_x, float grid_y, float pixel_0_grid_x, float pixel_0_grid_y, float &pixel_x, float &pixel_y);
  // update the values in this class with current keyboard/joystick/etc. input
  bool do_input();
  // clear old textures from the viewport
  //   screen_surface: the actual screen surface
  //   format: the format of pixels on the surface
  void blank_viewport(SDL_Surface* screen_surface, SDL_PixelFormat *format);
  // find the grid coordinates the viewport actual lies on, based on the current coordinates and zoom
  void find_viewport_extents_grid(float actual_zoom,float &viewport_left_grid, float &viewport_right_grid, float &viewport_top_grid, float &viewport_bottom_grid);
};

#endif
