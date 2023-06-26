/**
 * Implementation of the classes that represent different coordinates.
 *
 */
#include "debug.hpp"
#include "error.hpp"
#include "types.hpp"
#include "gridsetup.hpp"
#include "coordinates.hpp"
// C++ headers
#include <cmath>
#include <memory>

GridImageSize::GridImageSize(GridImageSize *grid_image_size) {
  this->_wimage=grid_image_size->_wimage;
  this->_himage=grid_image_size->_himage;
};

GridImageSize::GridImageSize(INT_T wimage, INT_T himage) {
  this->_wimage=wimage;
  this->_himage=himage;
};

INT_T GridImageSize::wimage() {
  return this->_wimage;
}

INT_T GridImageSize::himage() {
  return this->_himage;
}


GridCoordinateSize::GridCoordinateSize(GridCoordinateSize *grid_coordinate_size) {
  this->_wgrid=grid_coordinate_size->_wgrid;
  this->_hgrid=grid_coordinate_size->_hgrid;
}

GridCoordinateSize::GridCoordinateSize(FLOAT_T wgrid, FLOAT_T hgrid) {
  this->_wgrid=wgrid;
  this->_hgrid=hgrid;
}

FLOAT_T GridCoordinateSize::wgrid() {
  return this->_wgrid;
}

FLOAT_T GridCoordinateSize::hgrid() {
  return this->_hgrid;
}


GridCoordinate::GridCoordinate(GridCoordinate *grid_coordinate) {
  this->_xgrid=grid_coordinate->_xgrid;
  this->_ygrid=grid_coordinate->_ygrid;
}

GridCoordinate::GridCoordinate(FLOAT_T xgrid, FLOAT_T ygrid) {
  this->_xgrid=xgrid;
  this->_ygrid=ygrid;
};

FLOAT_T GridCoordinate::xgrid() {
  return this->_xgrid;
}

FLOAT_T GridCoordinate::ygrid() {
  return this->_ygrid;
}

GridPixelSize::GridPixelSize(GridPixelSize *grid_pixel_size) {
  this->_wpixel=grid_pixel_size->_wpixel;
  this->_hpixel=grid_pixel_size->_hpixel;
}

GridPixelSize::GridPixelSize(INT_T wpixel, INT_T hpixel) {
  this->_wpixel=wpixel;
  this->_hpixel=hpixel;
};

INT_T GridPixelSize::wpixel() {
  return this->_wpixel;
}
INT_T GridPixelSize::hpixel() {
  return this->_hpixel;
}

ViewportPixelCoordinate::ViewportPixelCoordinate(ViewportPixelCoordinate *viewport_pixel_coordinate) {
  this->_xpixel=viewport_pixel_coordinate->_xpixel;
  this->_ypixel=viewport_pixel_coordinate->_ypixel;
}

ViewportPixelCoordinate:: ViewportPixelCoordinate(INT_T xpixel, INT_T ypixel) {
  this->_xpixel=xpixel;
  this->_ypixel=ypixel;
}

ViewportPixelCoordinate::ViewportPixelCoordinate(GridCoordinate *grid_coordinate, FLOAT_T zoom, GridCoordinate *grid_coordinate_pixel_0, ViewportPixelSize *viewport_pixel_size) {
  this->_xpixel=(INT_T)round((grid_coordinate->xgrid() - grid_coordinate_pixel_0->xgrid())*viewport_pixel_size->wpixel()*zoom);
  this->_ypixel=(INT_T)round((grid_coordinate->ygrid() - grid_coordinate_pixel_0->ygrid())*viewport_pixel_size->hpixel()*zoom);
}

INT_T ViewportPixelCoordinate::xpixel() {
  return this->_xpixel;
}

INT_T ViewportPixelCoordinate::ypixel() {
  return this->_ypixel;
}

ViewportPixelSize::ViewportPixelSize(ViewportPixelSize *viewport_pixel_size) {
    this->_wpixel=viewport_pixel_size->_wpixel;
    this->_hpixel=viewport_pixel_size->_hpixel;
}

ViewportPixelSize::ViewportPixelSize(INT_T wpixel, INT_T hpixel) {
  this->_wpixel=wpixel;
  this->_hpixel=hpixel;
}

INT_T ViewportPixelSize::wpixel() {
  return this->_wpixel;
}

INT_T ViewportPixelSize::hpixel() {
  return this->_hpixel;
}
