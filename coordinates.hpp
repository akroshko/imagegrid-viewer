#ifndef COORDINATES_HPP

#define COORDINATES_HPP


// local headers
#include "config.hpp"
#include "debug.hpp"
#include "error.hpp"

class GridCoordinate {
public:
  GridCoordinate();
  ~GridCoordinate();
  // the width of this grid in images
  size_t images_wgrid;
  // the height of this grid in images
  size_t images_hgrid;
  // the maximum width of an image square in pixels
  size_t images_max_wpixel;
  // the maximum height of an image square in pixels
  size_t images_max_hpixel;
  void find_viewport_extents_grid(float actual_zoom, float screen_pixel_width, float screen_pixel_height, float viewport_xgrid, float viewport_ygrid,
                                  float &viewport_left_grid, float &viewport_right_grid, float &viewport_top_grid, float &viewport_bottom_grid);
  void grid_to_pixel(float zoom, float xgrid, float ygrid, float pixel_0_xgrid, float pixel_0_ygrid, float &xpixel, float &ypixel);
};

#endif
