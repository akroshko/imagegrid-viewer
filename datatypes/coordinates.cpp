/**
 * Implementation of the classes that represent different coordinates.
 *
 */
#include "../common.hpp"
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

GridCoordinate::GridCoordinate(BufferPixelCoordinate& buffer_pixel_coordinate,
                               FLOAT64 zoom,
                               BufferPixelSize& buffer_pixel_size,
                               GridCoordinate& buffer_grid_coordinate,
                               GridPixelSize& max_image_pixel_size) {
  auto buffer_x=buffer_grid_coordinate.x();
  auto buffer_y=buffer_grid_coordinate.y();
  auto max_image_wpixel=max_image_pixel_size.w();
  auto max_image_hpixel=max_image_pixel_size.h();
  auto half_width=buffer_pixel_size.w()/2.0;
  auto half_height=buffer_pixel_size.h()/2.0;
  auto buffer_horizontal_distance_grid=(half_width/(FLOAT64)max_image_wpixel/zoom);
  auto buffer_vertical_distance_grid=(half_height/(FLOAT64)max_image_hpixel/zoom);
  auto buffer_left_grid=buffer_x-buffer_horizontal_distance_grid;
  auto buffer_top_grid=buffer_y-buffer_vertical_distance_grid;
  this->_x1=(buffer_left_grid+((FLOAT64)buffer_pixel_coordinate.x()/(FLOAT64)max_image_wpixel/zoom));
  this->_x2=(buffer_top_grid+((FLOAT64)buffer_pixel_coordinate.y()/(FLOAT64)max_image_hpixel/zoom));
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

INT64 SubGridImageSize::w() const {
  return this->_x1;
}

INT64 SubGridImageSize::h() const {
  return this->_x2;
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

ImagePixelCoordinate::ImagePixelCoordinate(const GridCoordinate& grid_coordinate,
                                           const GridPixelSize& grid_pixel_size) {
  this->_x1=round((grid_coordinate.x()-floor(grid_coordinate.x()))*grid_pixel_size.w());
  this->_x2=round((grid_coordinate.y()-floor(grid_coordinate.y()))*grid_pixel_size.h());
}

INT64 ImagePixelCoordinate::x() const {
  return this->_x1;
}

INT64 ImagePixelCoordinate::y() const {
  return this->_x2;
}

BufferPixelCoordinate::BufferPixelCoordinate(GridCoordinate& grid_coordinate,
                                             FLOAT64 zoom,
                                             GridCoordinate& grid_coordinate_pixel_0,
                                             BufferPixelSize& buffer_pixel_size) {
  this->_x1=(INT64)round((grid_coordinate.x() - grid_coordinate_pixel_0.x())*buffer_pixel_size.w()*zoom);
  this->_x2=(INT64)round((grid_coordinate.y() - grid_coordinate_pixel_0.y())*buffer_pixel_size.h()*zoom);
}

INT64 BufferTileIndex::i() const {
  return this->_x1;
}

INT64 BufferTileIndex::j() const {
  return this->_x2;
}

INT64 BufferTileSize::w() const {
  return this->_x1;
}

INT64 BufferTileSize::h() const {
  return this->_x2;
}

INT64 BufferPixelCoordinate::x() const {
  return this->_x1;
}

INT64 BufferPixelCoordinate::y() const {
  return this->_x2;
}

INT64 BufferPixelSize::w() const {
  return this->_x1;
}

INT64 BufferPixelSize::h() const {
  return this->_x2;
}
