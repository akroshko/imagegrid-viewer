// local headers
#include "debug.hpp"
#include "error.hpp"
#include "types.hpp"
#include "defaults.hpp"
#include "utility.hpp"
#include "coordinates.hpp"
#include "viewport_current_state.hpp"
#include <cmath>
#include <mutex>

ViewPortCurrentState::ViewPortCurrentState () {
  this->zoom=NAN;
  this->zoom_last=NAN;
  this->been_updated=false;
}

void ViewPortCurrentState::UpdateGridValues(FLOAT_T zoom, const GridCoordinate &gridarg) {
  std::lock_guard<std::mutex> guard(this->using_mutex);
  if (std::isnan(this->zoom_last) || (this->grid_last.invalid()) || (zoom != this->zoom_last) || (gridarg.xgrid() != this->grid_last.xgrid()) || (gridarg.ygrid() != this->grid_last.ygrid())) {
    this->zoom=zoom;
    this->grid=GridCoordinate(gridarg);
    this->zoom_last=zoom;
    this->grid_last=GridCoordinate(gridarg);
    this->been_updated=true;
  }
}

bool ViewPortCurrentState::GetGridValues(FLOAT_T &zoom, GridCoordinate &gridarg) {
  std::lock_guard<std::mutex> guard(this->using_mutex);
  if (this->been_updated) {
    zoom=this->zoom;
    gridarg=GridCoordinate(this->grid);
    this->been_updated=false;
    return true;
  } else {
    return false;
  }
}

INT_T ViewPortCurrentState::find_zoom_index_bounded(FLOAT_T zoom,
                                                    INT_T min_zoom_index,
                                                    INT_T max_zoom_index) {
  auto return_value=floor(log2(1.0/zoom));
  if (return_value < min_zoom_index) {
    return_value=min_zoom_index;
  }
  if (return_value > max_zoom_index) {
    return_value=max_zoom_index;
  }
  return return_value;
}

FLOAT_T ViewPortCurrentState::find_zoom_upper(INT_T zoom_index) {
  if (zoom_index < 0) {
    zoom_index=0;
  }
  return 1.0/(pow(2.0,zoom_index+1));
}

FLOAT_T ViewPortCurrentState::_find_max_zoom(FLOAT_T zoom) {
  // round down to a fractional power of 2
  auto return_value=1.0/(pow(2.0,floor(log2(1.0/zoom))+1.0));
  // if (return_value >= 0.5) {
  //   return_value=0.5;
  // }
  return return_value;
}

FLOAT_T ViewPortCurrentState::find_leftmost_visible(FLOAT_T xgrid, FLOAT_T ygrid,
                                                    INT_T max_wpixel, INT_T max_hpixel,
                                                    FLOAT_T zoom) {
  auto max_zoom_this_level=_find_max_zoom(zoom);
  // calculate these with max reasonable resolution, rather than actual viewport
  auto half_pixel_width=((FLOAT_T)MAX_SCREEN_WIDTH/2.0);
  return xgrid-(half_pixel_width/max_wpixel/max_zoom_this_level);
}

FLOAT_T ViewPortCurrentState::find_rightmost_visible(FLOAT_T xgrid, FLOAT_T ygrid,
                                                     INT_T max_wpixel, INT_T max_hpixel,
                                                     FLOAT_T zoom) {
  auto max_zoom_this_level=_find_max_zoom(zoom);
  // calculate these with max reasonable resolution, rather than actual viewport
  auto half_pixel_width=((FLOAT_T)MAX_SCREEN_WIDTH/2.0);
  return xgrid+(half_pixel_width/max_wpixel/max_zoom_this_level);
}

FLOAT_T ViewPortCurrentState::find_topmost_visible(FLOAT_T xgrid, FLOAT_T ygrid,
                                                   INT_T max_wpixel, INT_T max_hpixel,
                                                   FLOAT_T zoom) {
  auto max_zoom_this_level=_find_max_zoom(zoom);
  // calculate these with max reasonable resolution, rather than actual viewport
  auto half_pixel_height=((FLOAT_T)MAX_SCREEN_HEIGHT/2.0);
  return ygrid-(half_pixel_height/max_hpixel/max_zoom_this_level);
}

FLOAT_T ViewPortCurrentState::find_bottommost_visible(FLOAT_T xgrid, FLOAT_T ygrid,
                                                      INT_T max_wpixel, INT_T max_hpixel,
                                                      FLOAT_T zoom) {
  auto max_zoom_this_level=_find_max_zoom(zoom);
  // calculate these with max reasonable resolution, rather than actual viewport
  auto half_pixel_height=((FLOAT_T)MAX_SCREEN_HEIGHT/2.0);
  return ygrid+(half_pixel_height/max_hpixel/max_zoom_this_level);
}

bool ViewPortCurrentState::grid_index_visible(INT_T i, INT_T j,
                                              FLOAT_T xgrid, FLOAT_T ygrid,
                                              INT_T max_wpixel, INT_T max_hpixel,
                                              FLOAT_T zoom) {
    auto visible_left=ViewPortCurrentState::find_leftmost_visible(
      xgrid,ygrid,
      max_wpixel,max_hpixel,
      zoom);
    auto visible_right=ViewPortCurrentState::find_rightmost_visible(
      xgrid,ygrid,
      max_wpixel,max_hpixel,
      zoom);
    auto visible_top=ViewPortCurrentState::find_topmost_visible(
      xgrid,ygrid,
      max_wpixel,max_hpixel,
      zoom);
    auto visible_bottom=ViewPortCurrentState::find_bottommost_visible(
      xgrid,ygrid,
      max_wpixel,max_hpixel,
      zoom);
    return (i >= floor(visible_left) && i <= floor(visible_right) &&
            j >= floor(visible_top) && j <= floor(visible_bottom));
}
