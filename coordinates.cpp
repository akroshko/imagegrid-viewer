/**
 * Implementation of the classes that represent different coordinates.
 *
 */
#include "common.hpp"
#include "coordinates.hpp"
// C headers
#include <cmath>

INT64 GridImageSize::w() const {
  return this->_x1;
}

INT64 GridImageSize::h() const {
  return this->_x2;
}

FLOAT64 GridCoordinateSize::w() const {
  return this->_x1;
}

FLOAT64 GridCoordinateSize::h() const {
  return this->_x2;
}

GridCoordinate::GridCoordinate(ViewportPixelCoordinate& viewport_pixel_coordinate,
                               FLOAT64 zoom,
                               ViewportPixelSize& viewport_pixel_size,
                               GridCoordinate& viewport_grid_coordinate,
                               GridPixelSize& max_image_pixel_size) {
  auto viewport_x=viewport_grid_coordinate.x();
  auto viewport_y=viewport_grid_coordinate.y();
  auto max_image_wpixel=max_image_pixel_size.w();
  auto max_image_hpixel=max_image_pixel_size.h();
  auto half_width=viewport_pixel_size.w()/2.0;
  auto half_height=viewport_pixel_size.h()/2.0;
  auto viewport_horizontal_distance_grid=(half_width/(FLOAT64)max_image_wpixel/zoom);
  auto viewport_vertical_distance_grid=(half_height/(FLOAT64)max_image_hpixel/zoom);
  auto viewport_left_grid=viewport_x-viewport_horizontal_distance_grid;
  auto viewport_top_grid=viewport_y-viewport_vertical_distance_grid;
  this->_x1=(viewport_left_grid+((FLOAT64)viewport_pixel_coordinate.x()/(FLOAT64)max_image_wpixel/zoom));
  this->_x2=(viewport_top_grid+((FLOAT64)viewport_pixel_coordinate.y()/(FLOAT64)max_image_hpixel/zoom));
}


FLOAT64 GridCoordinate::x() const {
  return this->_x1;
}

FLOAT64 GridCoordinate::y() const {
  return this->_x2;
}

bool GridCoordinate::invalid() const {
  return (std::isnan(this->_x1) || std::isnan(this->_x2));
}

INT64 GridIndex::i() const {
  return this->_x1;
}

INT64 GridIndex::j() const {
  return this->_x2;
}

bool GridIndex::invalid() const {
  return (this->_x1 == INVALID_PIXEL_VALUE || this->_x2 == INVALID_PIXEL_VALUE);
}

INT64 SubGridIndex::i() const {
  return this->_x1;
}

INT64 SubGridIndex::j() const {
  return this->_x2;
}

INT64 GridPixelSize::w() const {
  return this->_x1;
}
INT64 GridPixelSize::h() const {
  return this->_x2;
}

ImagePixelCoordinate::ImagePixelCoordinate(GridCoordinate& grid_coordinate,
                                           GridPixelSize& grid_pixel_size) {
  this->_x1=round((grid_coordinate.x()-floor(grid_coordinate.x()))*grid_pixel_size.w());
  this->_x2=round((grid_coordinate.y()-floor(grid_coordinate.y()))*grid_pixel_size.h());
}

INT64 ImagePixelCoordinate::x() const {
  return this->_x1;
}

INT64 ImagePixelCoordinate::y() const {
  return this->_x2;
}

ViewportPixelCoordinate::ViewportPixelCoordinate(GridCoordinate& grid_coordinate, FLOAT64 zoom,
                                                 GridCoordinate& grid_coordinate_pixel_0,
                                                 ViewportPixelSize& viewport_pixel_size) {
  this->_x1=(INT64)round((grid_coordinate.x() - grid_coordinate_pixel_0.x())*viewport_pixel_size.w()*zoom);
  this->_x2=(INT64)round((grid_coordinate.y() - grid_coordinate_pixel_0.y())*viewport_pixel_size.h()*zoom);
}

INT64 ViewportPixelCoordinate::x() const {
  return this->_x1;
}

INT64 ViewportPixelCoordinate::y() const {
  return this->_x2;
}

INT64 ViewportPixelSize::w() const {
  return this->_x1;
}

INT64 ViewportPixelSize::h() const {
  return this->_x2;
}
