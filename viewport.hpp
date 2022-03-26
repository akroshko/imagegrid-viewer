#ifndef VIEWPORT_HPP

#define VIEWPORT_HPP

// local headers
#include "gridclasses.hpp"
// C++ headers
#include <vector>
#include <iostream>
#include <string>
using namespace std;

// library headers
#include <SDL2/SDL.h>

// an item to blit to the screen
class BlitItem {
public:
  // create
    // surface: the SDL_Surface to blit
    // count: just counting the number of items to be blit, mostly for debugging
    // xpixel: the x location on the viewport that this texture is being blitted to
    // ypixel: the y location on the viewport that this texture is being blitted to
    // wpixel: the width on the viewport of this texture
    // hpixel: the height on the viewport of this texture
  BlitItem(SDL_Surface* surface, int count, float xpixel, float ypixel, float wpixel, float hpixel);
  ~BlitItem();
  // the SDL_Surface to blit
  SDL_Surface* blit_surface;
  // just counting the number of items to be blit, mostly for debugging
  int blit_index;
  // the x location on the viewport that this texture is being blitted to
  float viewport_xpixel;
  // the y location on the viewport that this texture is being blitted to
  float viewport_ypixel;
  // the width on the viewport of this texture
  float viewport_wpixel;
  // the height on the viewport of this texture
  float viewport_hpixel;
  void blit_this(SDL_Surface* screen_surface);
};

class ViewPort {
public:
  ViewPort();
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
  // calculate the viewport and set up the items to be blit to the screen
  //   grid: the current ImageGrid
  //   texturegrid: the current TextureGrid
  //   blititems: reference to blititems found when viewport is calculated
  void find_viewport(ImageGrid *grid, TextureGrid* texture_grid, vector<BlitItem> &blititems);
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
  //   TODO: upate this
  void find_viewport_extents_grid(TextureGrid* texture_grid, float actual_zoom, float &half_width, float &half_height, float &viewport_left_distance_grid, float &viewport_right_distance_grid, float &viewport_top_distance_grid, float &viewport_bottom_distance_grid, float &viewport_left_grid, float &viewport_right_grid, float &viewport_top_grid, float &viewport_bottom_grid, float &leftmost_grid, float &rightmost_grid, float &topmost_grid, float &bottommost_grid);
};

#endif
