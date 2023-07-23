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

INT_T ViewPortCurrentState::find_zoom_index(FLOAT_T zoom) {
  return floor(log2(1.0/zoom));
}

FLOAT_T ViewPortCurrentState::find_zoom(INT_T zoom_index) {
  return 1.0/(pow(2.0,zoom_index));
}

FLOAT_T ViewPortCurrentState::_find_max_zoom(INT_T zoom_index) {
  return ViewPortCurrentState::find_zoom(zoom_index+1);
}

FLOAT_T ViewPortCurrentState::_find_max_zoom(FLOAT_T zoom) {
  auto zoom_index=ViewPortCurrentState::find_zoom_index(zoom);
  return ViewPortCurrentState::find_zoom(zoom_index+1);
}

FLOAT_T ViewPortCurrentState::find_leftmost_visible(FLOAT_T xgrid, FLOAT_T ygrid,
                                                    INT_T max_wpixel, INT_T max_hpixel,
                                                    INT_T zoom_index) {
  auto max_zoom_this_level=_find_max_zoom(zoom_index);
  // calculate these with max reasonable resolution, rather than actual viewport
  auto half_pixel_width=((FLOAT_T)MAX_SCREEN_WIDTH/2.0);
  return xgrid-(half_pixel_width/max_wpixel/max_zoom_this_level);
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
                                                     INT_T zoom_index) {
  auto max_zoom_this_level=_find_max_zoom(zoom_index);
  // calculate these with max reasonable resolution, rather than actual viewport
  auto half_pixel_width=((FLOAT_T)MAX_SCREEN_WIDTH/2.0);
  return xgrid+(half_pixel_width/max_wpixel/max_zoom_this_level);
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
                                                   INT_T zoom_index) {
  auto max_zoom_this_level=_find_max_zoom(zoom_index);
  // calculate these with max reasonable resolution, rather than actual viewport
  auto half_pixel_height=((FLOAT_T)MAX_SCREEN_HEIGHT/2.0);
  return ygrid-(half_pixel_height/max_hpixel/max_zoom_this_level);
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
                                                      INT_T zoom_index) {
  auto max_zoom_this_level=_find_max_zoom(zoom_index);
  // calculate these with max reasonable resolution, rather than actual viewport
  auto half_pixel_height=((FLOAT_T)MAX_SCREEN_HEIGHT/2.0);
  return ygrid+(half_pixel_height/max_hpixel/max_zoom_this_level);
}

FLOAT_T ViewPortCurrentState::find_bottommost_visible(FLOAT_T xgrid, FLOAT_T ygrid,
                                                      INT_T max_wpixel, INT_T max_hpixel,
                                                      FLOAT_T zoom) {
  auto max_zoom_this_level=_find_max_zoom(zoom);
  // calculate these with max reasonable resolution, rather than actual viewport
  auto half_pixel_height=((FLOAT_T)MAX_SCREEN_HEIGHT/2.0);
  return ygrid+(half_pixel_height/max_hpixel/max_zoom_this_level);
}
