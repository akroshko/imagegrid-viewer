/**
 * Header to define the classes that represent different coordinates.
 */
#ifndef COORDINATES_HPP
#define COORDINATES_HPP

#include "common.hpp"
// C headers
#include <cmath>

// forward declaring everything
class GridImageSize;
class GridCoordinateSize;
class GridCoordinate;
class GridIndex;
class SubGridIndex;
class GridPixelSize;
class ImagePixelCoordinate;
class ViewportPixelSize;
class ViewportPixelCoordinate;

/**
 * A coordinate consisting of a pair of components.
 **/
template <typename T>
class CoordinatePair {
public:
  CoordinatePair()=default;
  /**
   * @param x1 The first component of the coordinate pair, typically x or width.
   * @param x2 The second component of the coordinate pair, typically y or height.
   */
  CoordinatePair (T x1, T x2);
  CoordinatePair(const CoordinatePair& coordinate_pair);
  CoordinatePair& operator=(const CoordinatePair& coordinate_pair);
  // TODO: not implemented yet
  CoordinatePair& operator+(const CoordinatePair& coordinate_pair);
  CoordinatePair& operator-(const CoordinatePair& coordinate_pair);
  CoordinatePair& operator*(const CoordinatePair& coordinate_pair);
  CoordinatePair& operator/(const CoordinatePair& coordinate_pair);
protected:
  T _x1;
  T _x2;
};

template <typename T>
CoordinatePair<T>::CoordinatePair (const CoordinatePair& coordinate_pair) {
  this->_x1=coordinate_pair._x1;
  this->_x2=coordinate_pair._x2;
};

template <typename T>
CoordinatePair<T>::CoordinatePair (T x1, T x2) {
  this->_x1=x1;
  this->_x2=x2;
};

template <typename T>
CoordinatePair<T>& CoordinatePair<T>::operator=(const CoordinatePair& coordinate_pair) {
  if (this == &coordinate_pair) {
    return *this;
  }
  this->_x1=coordinate_pair._x1;
  this->_x2=coordinate_pair._x2;
  return *this;
};

template <typename T>
class CoordinatePairINT : public CoordinatePair<T> {
public:
  CoordinatePairINT();
  CoordinatePairINT(T x1, T x2) : CoordinatePair<T>(x1, x2) {};
};

template <typename T>
CoordinatePairINT<T>::CoordinatePairINT () {
  this->_x1=INVALID_PIXEL_VALUE;
  this->_x2=INVALID_PIXEL_VALUE;
};

template <typename T>
class CoordinatePairFLOAT : public CoordinatePair<FLOAT64> {
public:
  CoordinatePairFLOAT();
  CoordinatePairFLOAT(T x1, T x2) : CoordinatePair<T>(x1, x2) {};
};

template <typename T>
CoordinatePairFLOAT<T>::CoordinatePairFLOAT () {
  this->_x1=NAN;
  this->_x2=NAN;
};

/**
 * Represents the size of the grid in integer units of images.
 *
 * For example, representing that a grid or part of a grid that is 20
 * images wide and 15 images high is a 20x15 grid of images.
 */
class GridImageSize : public CoordinatePairINT<INT64> {
public:
  GridImageSize()=default;
  /**
   * @param w The width of the grid in images.
   * @param h The height of the grid in images.
   */
  GridImageSize(const INT64 w, const INT64 h) : CoordinatePairINT<INT64>(w, h) {};
  /** @return The width. */
  INT64 w() const;
  /** @return The height. */
  INT64 h() const;
};

/**
 * Represents a size on the image grid in floating point units of
 * images.
 *
 * For example, a rectangle that is 3.2 image widths wide and 2.7
 * image heights high would be represented by 3.2x2.7.
 */
class GridCoordinateSize : public CoordinatePairFLOAT<FLOAT64> {
public:
  GridCoordinateSize()=default;
  /**
   * @param w The width on the grid.
   * @param h The height on the grid.
   */
  GridCoordinateSize(const FLOAT64 w, const FLOAT64 h) : CoordinatePairFLOAT<FLOAT64>(w, h) {};
  /** @return The width. */
  FLOAT64 w() const;
  /** @return The height. */
  FLOAT64 h() const;
};

/**
 * Represents coordinates on a grid of images with the origin in the
 * top-left corner.
 *
 * For example, a point 2.4 image widths right from the origin and 1.6
 * image height down would be represented by the coordinate (2.4,1.6).
 */
class GridCoordinate : public CoordinatePairFLOAT<FLOAT64> {
public:
  GridCoordinate()=default;
  /**
   * @param x The x coordinate on the grid.
   * @param y The y coordinate on the grid.
   */
  GridCoordinate(const FLOAT64 x, FLOAT64 y) : CoordinatePairFLOAT<FLOAT64>(x, y) {};
  /**
   * Convert from viewport pixel coordinates to grid coordinates.
   *
   * @param viewport_pixel_coordinate
   * @param zoom
   * @param viewport_pixel_size
   * @param viewport_grid_coordinate
   * @param max_image_pixel_size
   */
  GridCoordinate(ViewportPixelCoordinate& viewport_pixel_coordinate,
                 FLOAT64 zoom,
                 ViewportPixelSize& viewport_pixel_size,
                 GridCoordinate& viewport_grid_coordinate,
                 GridPixelSize& max_image_pixel_size);
  /** @return The x coordinate. */
  FLOAT64 x() const;
  /** @return The y coordinate. */
  FLOAT64 y() const;
  /** @return If values are valid. */
  bool invalid() const;
};

/**
 * Represents indices a grid of images with the origin in the top-left
 * corner.
 */
class GridIndex : public CoordinatePairINT<INT64> {
public:
  GridIndex()=default;
  /**
   * @param i The i index on the grid.
   * @param j The j index on the grid.
   */
  GridIndex(const INT64 i, INT64 j) : CoordinatePairINT<INT64>(i, j) {};
  GridIndex(ViewportPixelCoordinate& viewport_pixel_coordinate);
  /** @return The i index. */
  INT64 i() const;
  /** @return The j index. */
  INT64 j() const;
  /** @return If values are valid. */
  bool invalid() const;
};

/**
 * Represents indices with a grid, i.e., subgrid with the origin at
 * the top-right corner.
 *
 * Subgrids are generally used when the logical grid squares need
 * partitioning either because of the nature of the data (see
 * (E)ast/(W)est maps in the 50K Canadian NTS system) or for
 * performance reasons (grid squares larger than 16384x16384 that
 * makes sense for textures).
 */
class SubGridIndex : public CoordinatePairINT<INT64> {
public:
  SubGridIndex()=default;
  /**
   * @param i The i index on the subgrid.
   * @param j The j index on the subgrid.
   */
  SubGridIndex(const INT64 i, INT64 j) : CoordinatePairINT<INT64>(i, j) {};
  /** @return The i index. */
  INT64 i() const;
  /** @return The j index. */
  INT64 j() const;
  /** @return If values are valid. */
  bool invalid() const;
};

/**
 * Represents a size on the image grid in pixels.  For instance if
 * each image was 4000x2500 pixels, a rectangle represented by
 * GridCoordinateSize of 3.2x2.7 would be 12800x6750.
 */
class GridPixelSize : public CoordinatePairINT<INT64> {
public:
  GridPixelSize()=default;
  /**
   * @param w The width in pixels.
   * @param h The height in pixels.
   */
  GridPixelSize(INT64 w, INT64 h) : CoordinatePairINT<INT64>(w, h) {};
  /** @return The width in pixels. */
  INT64 w() const;
  /** @return The height in pixels. */
  INT64 h() const;
};

/**
 * Represents a coorindate on an image in pixels.  For instance
 * if each image on the grid was 2000x1500 pixels.  The coordinate
 * represented by GridCoordinate of (2.4,1.6) would be 800x900.
 */
class ImagePixelCoordinate : public CoordinatePairINT<INT64> {
public:
  ImagePixelCoordinate()=default;
  /**
   * @param x The x coordinate in pixels.
   * @param y The y coorindate in pixels.
   */
  ImagePixelCoordinate(INT64 x, INT64 y) : CoordinatePairINT<INT64>(x, y) {};
  /**
   * Initialize a pixel coordinate on an image with an image grid coordinate.
   *
   * @param grid_coordinate The coordinate on the grid.
   * @param grid_pixel_size The pixel size of the images on the grid.
   */
  ImagePixelCoordinate(GridCoordinate& grid_coordinate,
                       GridPixelSize& grid_pixel_size);
  /** @return The x coordinate. */
  INT64 x() const;
  /** @return The y coordinate. */
  INT64 y() const;
};

/**
 * Represents a size on the viewport in pixels.
 */
class ViewportPixelSize : public CoordinatePairINT<INT64> {
public:
  ViewportPixelSize()=default;
  /**
   * @param w The width in pixels.
   * @param h The height in pixels.
   */
  ViewportPixelSize(INT64 w, INT64 h) : CoordinatePairINT<INT64>(w, h) {};
  /** @return The width in pixels. */
  INT64 w() const;
  /** @return The height in pixels. */
  INT64 h() const;
};

/**
 * Represents a coorindate on the viewport in pixels using the upper
 * left corner as the origin.
 */
class ViewportPixelCoordinate : public CoordinatePairINT<INT64> {
public:
  ViewportPixelCoordinate()=default;
  /**
   * @param x The x coordinate in pixels.
   * @param y The y coorindate in pixels.
   */
  ViewportPixelCoordinate(INT64 x, INT64 y) : CoordinatePairINT<INT64>(x, y) {};
  /**
   * Convert a grid coordinate to a viewport pixel coordinate.
   *
   * @param grid_coordinate The GridCoordinate to convert.
   * @param zoom The zoom out value, used to convert between pixels on the
   *             image grid and viewport.
   * @param grid_coordinate_pixel_0 The grid coordinate of the origin
   *                                of the viewport.
   * @param viewport_pixel_size The size of the viewport in pixels.
   */
  ViewportPixelCoordinate(GridCoordinate& grid_coordinate,
                          FLOAT64 zoom,
                          GridCoordinate& grid_coordinate_pixel_0,
                          ViewportPixelSize& viewport_pixel_size);
  /** @return The x coordinate in pixels. */
  INT64 x() const;
  /** @return The y coordinate in pixels. */
  INT64 y() const;
};

#endif
