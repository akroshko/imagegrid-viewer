// local headers
#include "config.hpp"
#include "debug.hpp"
#include "error.hpp"
#include "coordinates.hpp"

GridCoordinate::GridCoordinate () {

}

GridCoordinate::~GridCoordinate () {

}

void GridCoordinate::find_viewport_extents_grid(float actual_zoom, float screen_pixel_width, float screen_pixel_height, float viewport_xgrid, float viewport_ygrid,
                                                float &viewport_left_grid, float &viewport_right_grid, float &viewport_top_grid, float &viewport_bottom_grid) {
  /* Find the coordinates of the viewport on the grid. */
  float half_width, half_height, viewport_left_distance_grid, viewport_right_distance_grid, viewport_top_distance_grid, viewport_bottom_distance_grid;
  // , leftmost_grid, rightmost_grid, topmost_grid, bottommost_grid;
  // TODO: set these somewhereelse
  half_width = screen_pixel_width / 2.0;
  half_height = screen_pixel_height / 2.0;
  // find viewport dimensions in grid space
  viewport_left_distance_grid=(half_width/this->images_max_wpixel/actual_zoom);
  viewport_right_distance_grid=(half_width/this->images_max_wpixel/actual_zoom);
  viewport_top_distance_grid=(half_height/this->images_max_hpixel/actual_zoom);
  viewport_bottom_distance_grid=(half_height/this->images_max_hpixel/actual_zoom);
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

void GridCoordinate::grid_to_pixel(float zoom, float xgrid, float ygrid, float pixel_0_xgrid, float pixel_0_ygrid, float &xpixel, float &ypixel) {
  /* Convert grid coordinates to pixel coordinates */
  DEBUG("grid_to_pixel() x: " << xgrid << " y: " << ygrid << " 0_x: " << pixel_0_xgrid << " 0_y: " << pixel_0_ygrid);
  xpixel=(xgrid - pixel_0_xgrid)*this->images_max_wpixel*zoom;
  ypixel=(ygrid - pixel_0_ygrid)*this->images_max_hpixel*zoom;
}
