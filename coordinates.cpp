/**
 * Implementation of the classes that represent different coordinates.
 *
 */
#include "common.hpp"
#include "coordinates.hpp"
// C headers
#include <cmath>

GridImageSize::GridImageSize() {
  // initializing to zero is best for now
  // add member to throw exception if invalid
  this->_wimage=INVALID_PIXEL_VALUE;
  this->_himage=INVALID_PIXEL_VALUE;
}

GridImageSize::GridImageSize(const GridImageSize &grid_image_size) {
  this->_wimage=grid_image_size._wimage;
  this->_himage=grid_image_size._himage;
};

GridImageSize& GridImageSize::operator=(const GridImageSize &grid_image_size) {
  this->_wimage=grid_image_size._wimage;
  this->_himage=grid_image_size._himage;
  return *this;
};


GridImageSize::GridImageSize(const INT_T wimage, const INT_T himage) {
  this->_wimage=wimage;
  this->_himage=himage;
};

INT_T GridImageSize::wimage() const {
  return this->_wimage;
}

INT_T GridImageSize::himage() const {
  return this->_himage;
}

GridCoordinateSize::GridCoordinateSize() {
  this->_wgrid=NAN;
  this->_hgrid=NAN;
}

GridCoordinateSize::GridCoordinateSize(const GridCoordinateSize &grid_coordinate_size) {
  this->_wgrid=grid_coordinate_size._wgrid;
  this->_hgrid=grid_coordinate_size._hgrid;
}

GridCoordinateSize& GridCoordinateSize::operator=(const GridCoordinateSize &grid_coordinate_size) {
  this->_wgrid=grid_coordinate_size._wgrid;
  this->_hgrid=grid_coordinate_size._hgrid;
  return *this;
}

GridCoordinateSize::GridCoordinateSize(FLOAT_T wgrid, FLOAT_T hgrid) {
  this->_wgrid=wgrid;
  this->_hgrid=hgrid;
}

FLOAT_T GridCoordinateSize::wgrid() const {
  return this->_wgrid;
}

FLOAT_T GridCoordinateSize::hgrid() const {
  return this->_hgrid;
}

GridCoordinate::GridCoordinate() {
  this->_xgrid=NAN;
  this->_ygrid=NAN;
}

GridCoordinate::GridCoordinate(const GridCoordinate &grid_coordinate) {
  this->_xgrid=grid_coordinate._xgrid;
  this->_ygrid=grid_coordinate._ygrid;
}

GridCoordinate& GridCoordinate::operator=(const GridCoordinate &grid_coordinate) {
  this->_xgrid=grid_coordinate._xgrid;
  this->_ygrid=grid_coordinate._ygrid;
  return *this;
}

GridCoordinate::GridCoordinate(FLOAT_T xgrid, FLOAT_T ygrid) {
  this->_xgrid=xgrid;
  this->_ygrid=ygrid;
};

FLOAT_T GridCoordinate::xgrid() const {
  return this->_xgrid;
}

FLOAT_T GridCoordinate::ygrid() const {
  return this->_ygrid;
}

bool GridCoordinate::invalid() const {
  return (std::isnan(this->_xgrid) || std::isnan(this->_ygrid));
}

GridIndex::GridIndex() {
  this->_igrid=INVALID_PIXEL_VALUE;
  this->_jgrid=INVALID_PIXEL_VALUE;
}

GridIndex::GridIndex(const INT_T igrid, INT_T jgrid) {
  this->_igrid=igrid;
  this->_jgrid=jgrid;
}

GridIndex::GridIndex(const GridIndex &grid_index) {
  this->_igrid=grid_index._igrid;
  this->_jgrid=grid_index._jgrid;
}

GridIndex& GridIndex::operator=(const GridIndex &grid_index) {
  this->_igrid=grid_index._igrid;
  this->_jgrid=grid_index._jgrid;
  return *this;
}

INT_T GridIndex::i_grid() const {
  return this->_igrid;
}

INT_T GridIndex::j_grid() const {
  return this->_jgrid;
}

SubGridIndex::SubGridIndex() {
  this->_i_subgrid=INVALID_PIXEL_VALUE;
  this->_j_subgrid=INVALID_PIXEL_VALUE;
}

SubGridIndex::SubGridIndex(const INT_T igrid, INT_T jgrid) {
  this->_i_subgrid=igrid;
  this->_j_subgrid=jgrid;
}

SubGridIndex::SubGridIndex(const SubGridIndex &grid_index) {
  this->_i_subgrid=grid_index._i_subgrid;
  this->_j_subgrid=grid_index._j_subgrid;
}

SubGridIndex& SubGridIndex::operator=(const SubGridIndex &subgrid_index) {
  this->_i_subgrid=subgrid_index._i_subgrid;
  this->_j_subgrid=subgrid_index._j_subgrid;
  return *this;
}

INT_T SubGridIndex::i_subgrid() const {
  return this->_i_subgrid;
}

INT_T SubGridIndex::j_subgrid() const {
  return this->_j_subgrid;
}

GridPixelSize::GridPixelSize() {
  this->_wpixel=INVALID_PIXEL_VALUE;
  this->_hpixel=INVALID_PIXEL_VALUE;
}

GridPixelSize::GridPixelSize(const GridPixelSize &grid_pixel_size) {
  this->_wpixel=grid_pixel_size._wpixel;
  this->_hpixel=grid_pixel_size._hpixel;
}

GridPixelSize& GridPixelSize::operator=(const GridPixelSize &grid_pixel_size) {
  this->_wpixel=grid_pixel_size._wpixel;
  this->_hpixel=grid_pixel_size._hpixel;
  return *this;
}

GridPixelSize::GridPixelSize(INT_T wpixel, INT_T hpixel) {
  this->_wpixel=wpixel;
  this->_hpixel=hpixel;
};

INT_T GridPixelSize::wpixel() const {
  return this->_wpixel;
}
INT_T GridPixelSize::hpixel() const {
  return this->_hpixel;
}

ViewportPixelCoordinate::ViewportPixelCoordinate() {
  this->_xpixel=INVALID_PIXEL_VALUE;
  this->_ypixel=INVALID_PIXEL_VALUE;
}

ViewportPixelCoordinate::ViewportPixelCoordinate(const ViewportPixelCoordinate &viewport_pixel_coordinate) {
  this->_xpixel=viewport_pixel_coordinate._xpixel;
  this->_ypixel=viewport_pixel_coordinate._ypixel;
}

ViewportPixelCoordinate& ViewportPixelCoordinate::operator=(const ViewportPixelCoordinate &viewport_pixel_coordinate) {
  this->_xpixel=viewport_pixel_coordinate._xpixel;
  this->_ypixel=viewport_pixel_coordinate._ypixel;
  return *this;
}

ViewportPixelCoordinate:: ViewportPixelCoordinate(INT_T xpixel, INT_T ypixel) {
  this->_xpixel=xpixel;
  this->_ypixel=ypixel;
}

ViewportPixelCoordinate::ViewportPixelCoordinate(GridCoordinate &grid_coordinate, FLOAT_T zoom, GridCoordinate &grid_coordinate_pixel_0, const ViewportPixelSize &viewport_pixel_size) {
  this->_xpixel=(INT_T)round((grid_coordinate.xgrid() - grid_coordinate_pixel_0.xgrid())*viewport_pixel_size.wpixel()*zoom);
  this->_ypixel=(INT_T)round((grid_coordinate.ygrid() - grid_coordinate_pixel_0.ygrid())*viewport_pixel_size.hpixel()*zoom);
}

INT_T ViewportPixelCoordinate::xpixel() const {
  return this->_xpixel;
}

INT_T ViewportPixelCoordinate::ypixel() const {
  return this->_ypixel;
}

ViewportPixelSize::ViewportPixelSize() {
  this->_wpixel=INVALID_PIXEL_VALUE;
  this->_hpixel=INVALID_PIXEL_VALUE;
}

ViewportPixelSize::ViewportPixelSize(const ViewportPixelSize &viewport_pixel_size) {
  this->_wpixel=viewport_pixel_size._wpixel;
  this->_hpixel=viewport_pixel_size._hpixel;
}

ViewportPixelSize& ViewportPixelSize::operator=(const ViewportPixelSize &viewport_pixel_size) {
  this->_wpixel=viewport_pixel_size._wpixel;
  this->_hpixel=viewport_pixel_size._hpixel;
  return *this;
}

ViewportPixelSize::ViewportPixelSize(INT_T wpixel, INT_T hpixel) {
  this->_wpixel=wpixel;
  this->_hpixel=hpixel;
}

INT_T ViewportPixelSize::wpixel() const {
  return this->_wpixel;
}

INT_T ViewportPixelSize::hpixel() const {
  return this->_hpixel;
}
