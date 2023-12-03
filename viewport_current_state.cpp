// local headers
#include "common.hpp"
#include "datatypes/coordinates.hpp"
#include "viewport_current_state.hpp"
// C++ headers
#include <mutex>
// C headers
#include <cmath>

// I'm purposely not using pass by reference here because I want new
// objects each time the state is updated
ViewPortCurrentState::ViewPortCurrentState(GridCoordinate current_grid_coordinate,
                                           GridPixelSize image_max_size,
                                           FLOAT64 zoom,
                                           BufferPixelSize screen_size,
                                           BufferPixelCoordinate pointer_pixel_coordinate,
                                           BufferPixelCoordinate center_pixel_coordinate) {
  this->_current_grid_coordinate=current_grid_coordinate;
  this->_image_max_size=image_max_size;
  this->_zoom=zoom;
  this->_screen_size=screen_size;
  this->_pointer_pixel_coordinate=pointer_pixel_coordinate;
  this->_center_pixel_coordinate=center_pixel_coordinate;
}

GridCoordinate ViewPortCurrentState::current_grid_coordinate () const {
  return this->_current_grid_coordinate;
}

GridPixelSize ViewPortCurrentState::image_max_size () const {
  return this->_image_max_size;
}

FLOAT64 ViewPortCurrentState::zoom () const {
  return this->_zoom;
}

BufferPixelSize ViewPortCurrentState::screen_size () const {
  return this->_screen_size;
}

BufferPixelCoordinate ViewPortCurrentState::pointer() const {
  return this->_pointer_pixel_coordinate;
}

BufferPixelCoordinate ViewPortCurrentState::center() const {
  return this->_center_pixel_coordinate;
}

ViewPortTransferState::ViewPortTransferState () {
  this->_zoom=NAN;
}

void ViewPortTransferState::UpdateGridValues(FLOAT64 zoom,
                                             const GridCoordinate& gridarg,
                                             const GridPixelSize& image_max_size,
                                             const BufferPixelSize& screen_size,
                                             const BufferPixelCoordinate& pointer_pixel_coordinate,
                                             const BufferPixelCoordinate& center_pixel_coordinate) {
  std::lock_guard<std::mutex> guard(this->_using_mutex);
  this->_zoom=zoom;
  this->_grid=GridCoordinate(gridarg);
  this->_image_max_size=GridPixelSize(image_max_size);
  this->_grid_last=GridCoordinate(gridarg);
  this->_screen_size=BufferPixelSize(screen_size);
  this->_pointer_pixel_coordinate=pointer_pixel_coordinate;
  this->_center_pixel_coordinate=center_pixel_coordinate;
}

ViewPortCurrentState ViewPortTransferState::GetGridValues() {
  std::lock_guard<std::mutex> guard(this->_using_mutex);
  auto viewport_current_state=ViewPortCurrentState(this->_grid,
                                                   this->_image_max_size,
                                                   this->_zoom,
                                                   this->_screen_size,
                                                   this->_pointer_pixel_coordinate,
                                                   this->_center_pixel_coordinate);
  return viewport_current_state;
}

INT64 ViewPortTransferState::find_zoom_out_shift_bounded(FLOAT64 zoom,
                                                         INT64 min_zoom_out_shift,
                                                         INT64 max_zoom_out_shift) {
  auto return_value=(INT64)floor(log2(1.0/zoom));
  if (return_value < min_zoom_out_shift) {
    return_value=min_zoom_out_shift;
  }
  if (return_value > max_zoom_out_shift) {
    return_value=max_zoom_out_shift;
  }
  return return_value;
}

FLOAT64 ViewPortTransferState::find_zoom_upper(INT64 zoom_out_shift) {
  if (zoom_out_shift < 0) {
    zoom_out_shift=0;
  }
  return 1.0/(pow(2.0,(FLOAT64)(zoom_out_shift+1)));
}

FLOAT64 ViewPortTransferState::_find_max_zoom(FLOAT64 zoom) {
  // round down to a fractional power of 2
  auto return_value=1.0/(pow(2.0,floor(log2(1.0/zoom))+1.0));
  // if (return_value >= 0.5) {
  //   return_value=0.5;
  // }
  return return_value;
}

FLOAT64 ViewPortTransferState::find_leftmost_visible(const ViewPortCurrentState& viewport_current_state) {
  auto max_zoom_this_level=_find_max_zoom(viewport_current_state.zoom());
  // calculate these with max reasonable resolution, rather than actual viewport
  auto half_wpixel=((FLOAT64)MAX_SCREEN_WIDTH/2.0);
  return (FLOAT64)viewport_current_state.current_grid_coordinate().x()-((FLOAT64)half_wpixel/(FLOAT64)viewport_current_state.image_max_size().w()/max_zoom_this_level);
}

FLOAT64 ViewPortTransferState::find_rightmost_visible(const ViewPortCurrentState& viewport_current_state) {
  auto max_zoom_this_level=_find_max_zoom(viewport_current_state.zoom());
  // calculate these with max reasonable resolution, rather than actual viewport
  auto half_wpixel=((FLOAT64)MAX_SCREEN_WIDTH/2.0);
  return (FLOAT64)viewport_current_state.current_grid_coordinate().x()+(half_wpixel/(FLOAT64)viewport_current_state.image_max_size().w()/max_zoom_this_level);
}

FLOAT64 ViewPortTransferState::find_topmost_visible(const ViewPortCurrentState& viewport_current_state) {
  auto max_zoom_this_level=_find_max_zoom(viewport_current_state.zoom());
  // calculate these with max reasonable resolution, rather than actual viewport
  auto half_hpixel=((FLOAT64)MAX_SCREEN_HEIGHT/2.0);
  return (FLOAT64)viewport_current_state.current_grid_coordinate().y()-(half_hpixel/(FLOAT64)viewport_current_state.image_max_size().h()/max_zoom_this_level);
}

FLOAT64 ViewPortTransferState::find_bottommost_visible(const ViewPortCurrentState& viewport_current_state) {
  auto max_zoom_this_level=_find_max_zoom(viewport_current_state.zoom());
  // calculate these with max reasonable resolution, rather than actual viewport
  auto half_hpixel=((FLOAT64)MAX_SCREEN_HEIGHT/2.0);
  return (FLOAT64)viewport_current_state.current_grid_coordinate().y()+(half_hpixel/(FLOAT64)viewport_current_state.image_max_size().h()/max_zoom_this_level);
}

bool ViewPortTransferState::grid_index_visible(INT64 i, INT64 j,
                                               const ViewPortCurrentState& viewport_current_state) {
    auto visible_left=ViewPortTransferState::find_leftmost_visible(
      viewport_current_state);
    auto visible_right=ViewPortTransferState::find_rightmost_visible(
      viewport_current_state);
    auto visible_top=ViewPortTransferState::find_topmost_visible(
      viewport_current_state);
    auto visible_bottom=ViewPortTransferState::find_bottommost_visible(
      viewport_current_state);
    return (i >= (INT64)floor(visible_left) && i <= (INT64)floor(visible_right) &&
            j >= (INT64)floor(visible_top) && j <= (INT64)floor(visible_bottom));
}
