// local headers
#include "common.hpp"
#include "coordinates.hpp"
#include "viewport_current_state.hpp"
// C++ headers
#include <mutex>
// C headers
#include <cmath>

ViewPortCurrentState::ViewPortCurrentState(GridCoordinate current_grid_coordinate,
                                           GridPixelSize max_image_size,
                                           FLOAT_T zoom,
                                           ViewportPixelSize screen_size) {
  this->_current_grid_coordinate=current_grid_coordinate;
  this->_max_image_size=max_image_size;
  this->_zoom=zoom;
  this->_screen_size=screen_size;
}

GridCoordinate ViewPortCurrentState::current_grid_coordinate () const {
  return this->_current_grid_coordinate;
}

GridPixelSize ViewPortCurrentState::max_image_size () const {
  return this->_max_image_size;
}

FLOAT_T ViewPortCurrentState::zoom () const {
  return this->_zoom;
}

ViewportPixelSize ViewPortCurrentState::screen_size () const {
  return this->_screen_size;
}

ViewPortTransferState::ViewPortTransferState () {
  this->_zoom=NAN;
}

void ViewPortTransferState::UpdateGridValues(FLOAT_T zoom,
                                             const GridCoordinate &gridarg,
                                             const GridPixelSize &max_image_size,
                                             const ViewportPixelSize &screen_size) {
  std::lock_guard<std::mutex> guard(this->_using_mutex);
  this->_zoom=zoom;
  this->_grid=GridCoordinate(gridarg);
  this->_max_image_size=GridPixelSize(max_image_size);
  this->_grid_last=GridCoordinate(gridarg);
  this->_screen_size=ViewportPixelSize(screen_size);
}

ViewPortCurrentState ViewPortTransferState::GetGridValues() {
  std::lock_guard<std::mutex> guard(this->_using_mutex);
  auto viewport_current_state=ViewPortCurrentState(GridCoordinate(this->_grid),
                                                   GridPixelSize(this->_max_image_size),
                                                   this->_zoom,
                                                   this->_screen_size=ViewportPixelSize(_screen_size));
  return viewport_current_state;
}

INT_T ViewPortTransferState::find_zoom_index_bounded(FLOAT_T zoom,
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

FLOAT_T ViewPortTransferState::find_zoom_upper(INT_T zoom_index) {
  if (zoom_index < 0) {
    zoom_index=0;
  }
  return 1.0/(pow(2.0,zoom_index+1));
}

FLOAT_T ViewPortTransferState::_find_max_zoom(FLOAT_T zoom) {
  // round down to a fractional power of 2
  auto return_value=1.0/(pow(2.0,floor(log2(1.0/zoom))+1.0));
  // if (return_value >= 0.5) {
  //   return_value=0.5;
  // }
  return return_value;
}

FLOAT_T ViewPortTransferState::find_leftmost_visible(const ViewPortCurrentState& viewport_current_state) {
  auto max_zoom_this_level=_find_max_zoom(viewport_current_state.zoom());
  // calculate these with max reasonable resolution, rather than actual viewport
  auto half_pixel_width=((FLOAT_T)MAX_SCREEN_WIDTH/2.0);
  return viewport_current_state.current_grid_coordinate().xgrid()-(half_pixel_width/viewport_current_state.max_image_size().wpixel()/max_zoom_this_level);
}

FLOAT_T ViewPortTransferState::find_rightmost_visible(const ViewPortCurrentState& viewport_current_state) {
  auto max_zoom_this_level=_find_max_zoom(viewport_current_state.zoom());
  // calculate these with max reasonable resolution, rather than actual viewport
  auto half_pixel_width=((FLOAT_T)MAX_SCREEN_WIDTH/2.0);
  return viewport_current_state.current_grid_coordinate().xgrid()+(half_pixel_width/viewport_current_state.max_image_size().wpixel()/max_zoom_this_level);
}

FLOAT_T ViewPortTransferState::find_topmost_visible(const ViewPortCurrentState& viewport_current_state) {
  auto max_zoom_this_level=_find_max_zoom(viewport_current_state.zoom());
  // calculate these with max reasonable resolution, rather than actual viewport
  auto half_pixel_height=((FLOAT_T)MAX_SCREEN_HEIGHT/2.0);
  return viewport_current_state.current_grid_coordinate().ygrid()-(half_pixel_height/viewport_current_state.max_image_size().hpixel()/max_zoom_this_level);
}

FLOAT_T ViewPortTransferState::find_bottommost_visible(const ViewPortCurrentState& viewport_current_state) {
  auto max_zoom_this_level=_find_max_zoom(viewport_current_state.zoom());
  // calculate these with max reasonable resolution, rather than actual viewport
  auto half_pixel_height=((FLOAT_T)MAX_SCREEN_HEIGHT/2.0);
  return viewport_current_state.current_grid_coordinate().ygrid()+(half_pixel_height/viewport_current_state.max_image_size().hpixel()/max_zoom_this_level);
}

bool ViewPortTransferState::grid_index_visible(INT_T i, INT_T j,
                                               const ViewPortCurrentState& viewport_current_state) {
    auto visible_left=ViewPortTransferState::find_leftmost_visible(
      viewport_current_state);
    auto visible_right=ViewPortTransferState::find_rightmost_visible(
      viewport_current_state);
    auto visible_top=ViewPortTransferState::find_topmost_visible(
      viewport_current_state);
    auto visible_bottom=ViewPortTransferState::find_bottommost_visible(
      viewport_current_state);
    return (i >= floor(visible_left) && i <= floor(visible_right) &&
            j >= floor(visible_top) && j <= floor(visible_bottom));
}
