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

GridImageSize::GridImageSize(const GridImageSize& grid_image_size) {
  this->_wimage=grid_image_size._wimage;
  this->_himage=grid_image_size._himage;
};

GridImageSize& GridImageSize::operator=(const GridImageSize& grid_image_size) {
  this->_wimage=grid_image_size._wimage;
  this->_himage=grid_image_size._himage;
  return *this;
};


GridImageSize::GridImageSize(const INT64 wimage, const INT64 himage) {
  this->_wimage=wimage;
  this->_himage=himage;
};

INT64 GridImageSize::wimage() const {
  return this->_wimage;
}

INT64 GridImageSize::himage() const {
  return this->_himage;
}

GridCoordinateSize::GridCoordinateSize() {
  this->_wgrid=NAN;
  this->_hgrid=NAN;
}

GridCoordinateSize::GridCoordinateSize(const GridCoordinateSize& grid_coordinate_size) {
  this->_wgrid=grid_coordinate_size._wgrid;
  this->_hgrid=grid_coordinate_size._hgrid;
}

GridCoordinateSize& GridCoordinateSize::operator=(const GridCoordinateSize& grid_coordinate_size) {
  this->_wgrid=grid_coordinate_size._wgrid;
  this->_hgrid=grid_coordinate_size._hgrid;
  return *this;
}

GridCoordinateSize::GridCoordinateSize(FLOAT64 wgrid, FLOAT64 hgrid) {
  this->_wgrid=wgrid;
  this->_hgrid=hgrid;
}

FLOAT64 GridCoordinateSize::wgrid() const {
  return this->_wgrid;
}

FLOAT64 GridCoordinateSize::hgrid() const {
  return this->_hgrid;
}

GridCoordinate::GridCoordinate() {
  this->_xgrid=NAN;
  this->_ygrid=NAN;
}

GridCoordinate::GridCoordinate(const GridCoordinate& grid_coordinate) {
  this->_xgrid=grid_coordinate._xgrid;
  this->_ygrid=grid_coordinate._ygrid;
}

GridCoordinate& GridCoordinate::operator=(const GridCoordinate& grid_coordinate) {
  this->_xgrid=grid_coordinate._xgrid;
  this->_ygrid=grid_coordinate._ygrid;
  return *this;
}

GridCoordinate::GridCoordinate(FLOAT64 xgrid, FLOAT64 ygrid) {
  this->_xgrid=xgrid;
  this->_ygrid=ygrid;
}

GridCoordinate::GridCoordinate(ViewportPixelCoordinate& viewport_pixel_coordinate,
                               FLOAT64 zoom,
                               ViewportPixelSize& viewport_pixel_size,
                               GridCoordinate& viewport_grid_coordinate,
                               GridPixelSize& max_image_pixel_size) {
  auto viewport_x=viewport_grid_coordinate.xgrid();
  auto viewport_y=viewport_grid_coordinate.ygrid();
  auto max_image_wpixel=max_image_pixel_size.wpixel();
  auto max_image_hpixel=max_image_pixel_size.hpixel();
  auto half_width=viewport_pixel_size.wpixel()/2.0;
  auto half_height=viewport_pixel_size.hpixel()/2.0;
  auto viewport_horizontal_distance_grid=(half_width/(FLOAT64)max_image_wpixel/zoom);
  auto viewport_vertical_distance_grid=(half_height/(FLOAT64)max_image_hpixel/zoom);
  auto viewport_left_grid=viewport_x-viewport_horizontal_distance_grid;
  auto viewport_top_grid=viewport_y-viewport_vertical_distance_grid;
  this->_xgrid=(viewport_left_grid+((FLOAT64)viewport_pixel_coordinate.xpixel()/(FLOAT64)max_image_wpixel/zoom));
  this->_ygrid=(viewport_top_grid+((FLOAT64)viewport_pixel_coordinate.ypixel()/(FLOAT64)max_image_hpixel/zoom));
}


FLOAT64 GridCoordinate::xgrid() const {
  return this->_xgrid;
}

FLOAT64 GridCoordinate::ygrid() const {
  return this->_ygrid;
}

bool GridCoordinate::invalid() const {
  return (std::isnan(this->_xgrid) || std::isnan(this->_ygrid));
}

GridIndex::GridIndex() {
  this->_igrid=INVALID_PIXEL_VALUE;
  this->_jgrid=INVALID_PIXEL_VALUE;
}

GridIndex::GridIndex(const INT64 igrid, INT64 jgrid) {
  this->_igrid=igrid;
  this->_jgrid=jgrid;
}

GridIndex::GridIndex(const GridIndex& grid_index) {
  this->_igrid=grid_index._igrid;
  this->_jgrid=grid_index._jgrid;
}

GridIndex& GridIndex::operator=(const GridIndex& grid_index) {
  this->_igrid=grid_index._igrid;
  this->_jgrid=grid_index._jgrid;
  return *this;
}

INT64 GridIndex::i_grid() const {
  return this->_igrid;
}

INT64 GridIndex::j_grid() const {
  return this->_jgrid;
}

bool GridIndex::invalid() const {
  return (this->_igrid == INVALID_PIXEL_VALUE || this->_jgrid == INVALID_PIXEL_VALUE);
}

SubGridIndex::SubGridIndex() {
  this->_i_subgrid=INVALID_PIXEL_VALUE;
  this->_j_subgrid=INVALID_PIXEL_VALUE;
}

SubGridIndex::SubGridIndex(const INT64 igrid, INT64 jgrid) {
  this->_i_subgrid=igrid;
  this->_j_subgrid=jgrid;
}

SubGridIndex::SubGridIndex(const SubGridIndex& grid_index) {
  this->_i_subgrid=grid_index._i_subgrid;
  this->_j_subgrid=grid_index._j_subgrid;
}

SubGridIndex& SubGridIndex::operator=(const SubGridIndex& sub_index) {
  this->_i_subgrid=sub_index._i_subgrid;
  this->_j_subgrid=sub_index._j_subgrid;
  return *this;
}

INT64 SubGridIndex::subgrid_i() const {
  return this->_i_subgrid;
}

INT64 SubGridIndex::subgrid_j() const {
  return this->_j_subgrid;
}

GridPixelSize::GridPixelSize() {
  this->_wpixel=INVALID_PIXEL_VALUE;
  this->_hpixel=INVALID_PIXEL_VALUE;
}

GridPixelSize::GridPixelSize(const GridPixelSize& grid_pixel_size) {
  this->_wpixel=grid_pixel_size._wpixel;
  this->_hpixel=grid_pixel_size._hpixel;
}

GridPixelSize& GridPixelSize::operator=(const GridPixelSize& grid_pixel_size) {
  this->_wpixel=grid_pixel_size._wpixel;
  this->_hpixel=grid_pixel_size._hpixel;
  return *this;
}

GridPixelSize::GridPixelSize(INT64 wpixel, INT64 hpixel) {
  this->_wpixel=wpixel;
  this->_hpixel=hpixel;
};

INT64 GridPixelSize::wpixel() const {
  return this->_wpixel;
}
INT64 GridPixelSize::hpixel() const {
  return this->_hpixel;
}

ImagePixelCoordinate::ImagePixelCoordinate() {
  this->_xpixel=INVALID_PIXEL_VALUE;
  this->_ypixel=INVALID_PIXEL_VALUE;
}

ImagePixelCoordinate::ImagePixelCoordinate(const ImagePixelCoordinate& grid_pixel_coordinate) {
  this->_xpixel=grid_pixel_coordinate.xpixel();
  this->_ypixel=grid_pixel_coordinate.ypixel();
}

ImagePixelCoordinate::ImagePixelCoordinate(INT64 xpixel, INT64 ypixel) {
  this->_xpixel=xpixel;
  this->_ypixel=ypixel;
}

ImagePixelCoordinate::ImagePixelCoordinate(GridCoordinate& grid_coordinate,
                                           GridPixelSize& grid_pixel_size) {
  this->_xpixel=round((grid_coordinate.xgrid()-floor(grid_coordinate.xgrid()))*grid_pixel_size.wpixel());
  this->_ypixel=round((grid_coordinate.ygrid()-floor(grid_coordinate.ygrid()))*grid_pixel_size.hpixel());
}

ImagePixelCoordinate& ImagePixelCoordinate::operator=(const ImagePixelCoordinate& grid_pixel_coordinate) {
  this->_xpixel=grid_pixel_coordinate.xpixel();
  this->_ypixel=grid_pixel_coordinate.ypixel();
  return *this;
}

INT64 ImagePixelCoordinate::xpixel() const {
  return this->_xpixel;
}

INT64 ImagePixelCoordinate::ypixel() const {
  return this->_ypixel;
}

ViewportPixelCoordinate::ViewportPixelCoordinate() {
  this->_xpixel=INVALID_PIXEL_VALUE;
  this->_ypixel=INVALID_PIXEL_VALUE;
}

ViewportPixelCoordinate::ViewportPixelCoordinate(const ViewportPixelCoordinate& viewport_pixel_coordinate) {
  this->_xpixel=viewport_pixel_coordinate._xpixel;
  this->_ypixel=viewport_pixel_coordinate._ypixel;
}

ViewportPixelCoordinate& ViewportPixelCoordinate::operator=(const ViewportPixelCoordinate& viewport_pixel_coordinate) {
  this->_xpixel=viewport_pixel_coordinate._xpixel;
  this->_ypixel=viewport_pixel_coordinate._ypixel;
  return *this;
}

ViewportPixelCoordinate:: ViewportPixelCoordinate(INT64 xpixel, INT64 ypixel) {
  this->_xpixel=xpixel;
  this->_ypixel=ypixel;
}

ViewportPixelCoordinate::ViewportPixelCoordinate(GridCoordinate& grid_coordinate, FLOAT64 zoom,
                                                 GridCoordinate& grid_coordinate_pixel_0,
                                                 ViewportPixelSize& viewport_pixel_size) {
  this->_xpixel=(INT64)round((grid_coordinate.xgrid() - grid_coordinate_pixel_0.xgrid())*viewport_pixel_size.wpixel()*zoom);
  this->_ypixel=(INT64)round((grid_coordinate.ygrid() - grid_coordinate_pixel_0.ygrid())*viewport_pixel_size.hpixel()*zoom);
}

INT64 ViewportPixelCoordinate::xpixel() const {
  return this->_xpixel;
}

INT64 ViewportPixelCoordinate::ypixel() const {
  return this->_ypixel;
}

ViewportPixelSize::ViewportPixelSize() {
  this->_wpixel=INVALID_PIXEL_VALUE;
  this->_hpixel=INVALID_PIXEL_VALUE;
}

ViewportPixelSize::ViewportPixelSize(const ViewportPixelSize& viewport_pixel_size) {
  this->_wpixel=viewport_pixel_size._wpixel;
  this->_hpixel=viewport_pixel_size._hpixel;
}

ViewportPixelSize& ViewportPixelSize::operator=(const ViewportPixelSize& viewport_pixel_size) {
  this->_wpixel=viewport_pixel_size._wpixel;
  this->_hpixel=viewport_pixel_size._hpixel;
  return *this;
}

ViewportPixelSize::ViewportPixelSize(INT64 wpixel, INT64 hpixel) {
  this->_wpixel=wpixel;
  this->_hpixel=hpixel;
}

INT64 ViewportPixelSize::wpixel() const {
  return this->_wpixel;
}

INT64 ViewportPixelSize::hpixel() const {
  return this->_hpixel;
}
