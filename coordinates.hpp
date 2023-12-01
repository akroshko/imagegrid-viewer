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
class BufferPixelSize;
class BufferPixelCoordinate;

template <typename T>
class CoordinatePair;

template <typename T>
CoordinatePair<T> operator+(const CoordinatePair<T>& coordinate_pair, const T& scalar) {
  CoordinatePair<T> result;
  result._x1=coordinate_pair._x1+scalar;
  result._x2=coordinate_pair._x2+scalar;
  return result;
}

template <typename T>
CoordinatePair<T> operator+(const CoordinatePair<T>& coordinate_pair_1, const CoordinatePair<T>& coordinate_pair_2) {
  CoordinatePair<T> result;
  result._x1=coordinate_pair_1._x1+coordinate_pair_2._x1;
  result._x2=coordinate_pair_1._x2+coordinate_pair_2._x2;
  return result;
}

template <typename T>
CoordinatePair<T> operator-(const CoordinatePair<T>& coordinate_pair, const T& scalar) {
  CoordinatePair<T> result;
  result._x1=coordinate_pair._x1-scalar;
  result._x2=coordinate_pair._x2-scalar;
  return result;
}

template <typename T>
CoordinatePair<T> operator-(const CoordinatePair<T>& coordinate_pair_1, const CoordinatePair<T>& coordinate_pair_2) {
  CoordinatePair<T> result;
  result._x1=coordinate_pair_1._x1-coordinate_pair_2._x1;
  result._x2=coordinate_pair_1._x2-coordinate_pair_2._x2;
  return result;
}

template <typename T>
CoordinatePair<T> operator*(const CoordinatePair<T>& coordinate_pair,  const T& scalar) {
  CoordinatePair<T> result;
  result._x1=coordinate_pair._x1*scalar;
  result._x2=coordinate_pair._x2*scalar;
  return result;
}

template <typename T>
CoordinatePair<T> operator*(const CoordinatePair<T>& coordinate_pair_1, const CoordinatePair<T>& coordinate_pair_2) {
  CoordinatePair<T> result;
  result._x1=coordinate_pair_1._x1*coordinate_pair_2._x1;
  result._x2=coordinate_pair_1._x2*coordinate_pair_2._x2;
  return result;
}

template <typename T>
CoordinatePair<T> operator/(const CoordinatePair<T>& coordinate_pair, const T& scalar) {
  CoordinatePair<T> result;
  result._x1=coordinate_pair._x1/scalar;
  result._x2=coordinate_pair._x2/scalar;
  return result;
}

template <typename T>
CoordinatePair<T> operator/(const CoordinatePair<T>& coordinate_pair_1, const CoordinatePair<T>& coordinate_pair_2) {
  CoordinatePair<T> result;
  result._x1=coordinate_pair_1._x1/coordinate_pair_2._x1;
  result._x2=coordinate_pair_1._x2/coordinate_pair_2._x2;
  return result;
}

/**
 * A coordinate consisting of a pair of components.
 **/
template <typename T>
class CoordinatePair {
public:
  CoordinatePair()=default;
  CoordinatePair(const CoordinatePair& coordinate_pair);
  CoordinatePair& operator=(const CoordinatePair& coordinate_pair);
  /**
   * @param x1 The first component of the coordinate pair, typically x or width.
   * @param x2 The second component of the coordinate pair, typically y or height.
   */
  CoordinatePair (T x1, T x2);
  friend CoordinatePair operator+ <>(const CoordinatePair& coordinate_pair, const T& scalar);
  friend CoordinatePair operator+ <>(const CoordinatePair& coordinate_pair_1, const CoordinatePair& coordinate_pair_2);
  friend CoordinatePair operator- <>(const CoordinatePair& coordinate_pair, const T& scalar);
  friend CoordinatePair operator- <>(const CoordinatePair& coordinate_pair_1, const CoordinatePair& coordinate_pair_2);
  friend CoordinatePair operator* <>(const CoordinatePair& coordinate_pair, const T& scalar);
  friend CoordinatePair operator* <>(const CoordinatePair& coordinate_pair_1, const CoordinatePair& coordinate_pair_2);
  friend CoordinatePair operator/ <>(const CoordinatePair& coordinate_pair, const T& scalar);
  friend CoordinatePair operator/ <>(const CoordinatePair& coordinate_pair_1, const CoordinatePair& coordinate_pair_2);
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
class CoordinatePairINT;

template <typename T>
CoordinatePairINT<T> operator>>(const CoordinatePairINT<T>& coordinate_pair, const INT64 right_shift) {
  CoordinatePairINT<T> result;
  result._x1=coordinate_pair._x1 >> right_shift;
  result._x2=coordinate_pair._x2 >> right_shift;
  return result;
}

template <typename T>
CoordinatePairINT<T> operator%(const CoordinatePairINT<T>& coordinate_pair, const INT64 divisor) {
  CoordinatePairINT<T> result;
  result._x1=coordinate_pair._x1 % divisor;
  result._x2=coordinate_pair._x2 % divisor;
  return result;
}

template <typename T>
CoordinatePairINT<T> operator%(const CoordinatePairINT<T>& coordinate_pair_1, const CoordinatePairINT<T>& coordinate_pair_2) {
  CoordinatePairINT<T> result;
  result._x1=coordinate_pair_1._x1 % coordinate_pair_2._x1;
  result._x2=coordinate_pair_2._x2 % coordinate_pair_2._x2;
  return result;
}

template <typename T>
CoordinatePairINT<T> operator<<(const CoordinatePairINT<T>& coordinate_pair, const INT64 left_shift) {
  CoordinatePairINT<T> result;
  result._x1=coordinate_pair._x1 << left_shift;
  result._x2=coordinate_pair._x2 << left_shift;
  return result;
}

template <typename T>
class CoordinatePairINT : public CoordinatePair<T> {
public:
  CoordinatePairINT();
  CoordinatePairINT(const CoordinatePair<T>& coordinate_pair) : CoordinatePair<T>(coordinate_pair) {};
  CoordinatePairINT(const CoordinatePairINT& coordinate_pair) : CoordinatePair<T>(coordinate_pair) {};
  CoordinatePairINT(T x1, T x2) : CoordinatePair<T>(x1, x2) {};
  using CoordinatePair<T>::operator=;
  CoordinatePairINT& operator=(const CoordinatePairINT& coordinate_size)=default;
  friend CoordinatePairINT operator% <>(const CoordinatePairINT& coordinate_pair, const INT64 divisor);
  friend CoordinatePairINT operator% <>(const CoordinatePairINT& coordinate_pair_1, const CoordinatePairINT<T>& coordinate_pair_2);
  friend CoordinatePairINT operator>> <>(const CoordinatePairINT& coordinate_pair, const INT64 right_shift);
  friend CoordinatePairINT operator<< <>(const CoordinatePairINT& coordinate_pair, const INT64 right_shift);
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
  CoordinatePairFLOAT(const CoordinatePair<T>& coordinate_pair) : CoordinatePair<T>(coordinate_pair) {};
  CoordinatePairFLOAT(const CoordinatePairFLOAT& coordinate_pair) : CoordinatePair<T>(coordinate_pair) {};
  CoordinatePairFLOAT(T x1, T x2) : CoordinatePair<T>(x1, x2) {};
  using CoordinatePair<T>::operator=;
  CoordinatePairFLOAT& operator=(const CoordinatePairFLOAT& coordinate_size)=default;
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
  GridImageSize(const CoordinatePair<INT64>& grid_image_size) : CoordinatePairINT<INT64>(grid_image_size) {};
  GridImageSize(const CoordinatePairINT<INT64>& grid_image_size) : CoordinatePairINT<INT64>(grid_image_size) {};
  GridImageSize(const GridImageSize& grid_pixel_size) : CoordinatePairINT<INT64>(grid_pixel_size) {};
  using CoordinatePairINT<INT64>::operator=;
  GridImageSize& operator=(const GridImageSize& grid_image_size)=default;
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
  GridCoordinateSize(const CoordinatePair<FLOAT64>& grid_coordinate_size) : CoordinatePairFLOAT<FLOAT64>(grid_coordinate_size) {};
  GridCoordinateSize(const CoordinatePairFLOAT<FLOAT64>& grid_coordinate_size) : CoordinatePairFLOAT<FLOAT64>(grid_coordinate_size) {};
  GridCoordinateSize(const GridCoordinateSize& grid_coordinate_size) : CoordinatePairFLOAT<FLOAT64>(grid_coordinate_size) {};
  using CoordinatePairFLOAT<FLOAT64>::operator=;
  GridCoordinateSize& operator=(const GridCoordinateSize& grid_coordinate_size)=default;
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
  GridCoordinate(const CoordinatePair<FLOAT64>& grid_coordinate) : CoordinatePairFLOAT<FLOAT64>(grid_coordinate) {};
  GridCoordinate(const CoordinatePairFLOAT<FLOAT64>& grid_coordinate) : CoordinatePairFLOAT<FLOAT64>(grid_coordinate) {};
  GridCoordinate(const GridCoordinate& grid_coordinate) :  CoordinatePairFLOAT<FLOAT64>(grid_coordinate) {};
  using CoordinatePairFLOAT<FLOAT64>::operator=;
  GridCoordinate& operator=(const GridCoordinate& grid_coordinate)=default;
  /**
   * @param x The x coordinate on the grid.
   * @param y The y coordinate on the grid.
   */
  GridCoordinate(const FLOAT64 x, const FLOAT64 y) : CoordinatePairFLOAT<FLOAT64>(x, y) {};
  /**
   * Convert from buffer pixel coordinates to grid coordinates.
   *
   * @param buffer_pixel_coordinate
   * @param zoom
   * @param buffer_pixel_size
   * @param buffer_grid_coordinate
   * @param max_image_pixel_size
   */
  GridCoordinate(BufferPixelCoordinate& buffer_pixel_coordinate,
                 FLOAT64 zoom,
                 BufferPixelSize& buffer_pixel_size,
                 GridCoordinate& buffer_grid_coordinate,
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
  GridIndex(const CoordinatePair<INT64>& grid_index) : CoordinatePairINT<INT64>(grid_index) {};
  GridIndex(const CoordinatePairINT<INT64>& grid_index) : CoordinatePairINT<INT64>(grid_index) {};
  GridIndex(const GridIndex& grid_index) : CoordinatePairINT<INT64>(grid_index) {};
  GridIndex& operator=(const GridIndex& grid_index)=default;
  /**
   * @param i The i index on the grid.
   * @param j The j index on the grid.
   */
  GridIndex(const INT64 i, const INT64 j) : CoordinatePairINT<INT64>(i, j) {};

  /** @return The i index. */
  INT64 i() const;
  /** @return The j index. */
  INT64 j() const;
  /** @return If values are valid. */
  bool invalid() const;
};

/**
 * Represents the size of a subgrid in integer units of images.
 *
 */
class SubGridImageSize : public CoordinatePairINT<INT64> {
public:
  SubGridImageSize()=default;
  SubGridImageSize(const CoordinatePair<INT64>& grid_image_size) : CoordinatePairINT<INT64>(grid_image_size) {};
  SubGridImageSize(const CoordinatePairINT<INT64>& grid_image_size) : CoordinatePairINT<INT64>(grid_image_size) {};
  SubGridImageSize(const SubGridImageSize& grid_pixel_size) : CoordinatePairINT<INT64>(grid_pixel_size) {};
  using CoordinatePairINT<INT64>::operator=;
  SubGridImageSize& operator=(const SubGridImageSize& grid_image_size)=default;
  /**
   * @param w The width of the subgrid in images.
   * @param h The height of the subgrid in images.
   */
  SubGridImageSize(const INT64 w, const INT64 h) : CoordinatePairINT<INT64>(w, h) {};
  /** @return The width of the subgrid. */
  INT64 w() const;
  /** @return The height of the subgrid. */
  INT64 h() const;
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
  SubGridIndex(const CoordinatePair<INT64>& subgrid_index) : CoordinatePairINT<INT64>(subgrid_index) {};
  SubGridIndex(const CoordinatePairINT<INT64>& subgrid_index) : CoordinatePairINT<INT64>(subgrid_index) {};
  SubGridIndex(const SubGridIndex& subgrid_index) : CoordinatePairINT<INT64>(subgrid_index) {};
  using CoordinatePairINT<INT64>::operator=;
  SubGridIndex& operator=(const SubGridIndex& subgrid_index)=default;
  /**
   * @param i The i index on the subgrid.
   * @param j The j index on the subgrid.
   */
  SubGridIndex(const INT64 i, const INT64 j) : CoordinatePairINT<INT64>(i, j) {};
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
  GridPixelSize(const CoordinatePair<INT64>& grid_pixel_size) : CoordinatePairINT<INT64>(grid_pixel_size) {};
  GridPixelSize(const CoordinatePairINT<INT64>& grid_pixel_size) : CoordinatePairINT<INT64>(grid_pixel_size) {};
  GridPixelSize(const GridPixelSize& grid_pixel_size) : CoordinatePairINT<INT64>(grid_pixel_size) {};
  using CoordinatePairINT<INT64>::operator=;
  GridPixelSize& operator=(const GridPixelSize& grid_pixel_size)=default;
  /**
   * @param w The width in pixels.
   * @param h The height in pixels.
   */
  GridPixelSize(const INT64 w, const INT64 h) : CoordinatePairINT<INT64>(w, h) {};
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
  ImagePixelCoordinate(const CoordinatePair<INT64>& image_pixel_coordinate) : CoordinatePairINT<INT64>(image_pixel_coordinate) {};
  ImagePixelCoordinate(const CoordinatePairINT<INT64>& image_pixel_coordinate) : CoordinatePairINT<INT64>(image_pixel_coordinate) {};
  ImagePixelCoordinate(const ImagePixelCoordinate& image_pixel_coordinate) : CoordinatePairINT<INT64>(image_pixel_coordinate) {};
  using CoordinatePairINT<INT64>::operator=;
  ImagePixelCoordinate& operator=(const ImagePixelCoordinate& image_pixel_coordinate)=default;
  /**
   * @param x The x coordinate in pixels.
   * @param y The y coorindate in pixels.
   */
  ImagePixelCoordinate(const INT64 x, const INT64 y) : CoordinatePairINT<INT64>(x, y) {};
  /**
   * Initialize a pixel coordinate on an image with an image grid coordinate.
   *
   * @param grid_coordinate The coordinate on the grid.
   * @param grid_pixel_size The pixel size of the images on the grid.
   */
  ImagePixelCoordinate(const GridCoordinate& grid_coordinate,
                       const GridPixelSize& grid_pixel_size);
  /** @return The x coordinate. */
  INT64 x() const;
  /** @return The y coordinate. */
  INT64 y() const;
};

/**
 * Represents the number of tiles representing a buffer.
 */
class BufferTileSize : public CoordinatePairINT<INT64> {
public:
  BufferTileSize()=default;
  BufferTileSize(const CoordinatePair<INT64>& buffer_tile_size) : CoordinatePairINT<INT64>( buffer_tile_size) {};
  BufferTileSize(const CoordinatePairINT<INT64>& buffer_tile_size) : CoordinatePairINT<INT64>( buffer_tile_size) {};
  BufferTileSize(const BufferTileSize& buffer_tile_size) : CoordinatePairINT<INT64>(buffer_tile_size) {};
  using CoordinatePairINT<INT64>::operator=;
  BufferTileSize& operator=(const BufferTileSize& buffer_tile_size)=default;
  /**
   * @param w The number of tiles wide.
   * @param h The number of tiles high.
   */
  BufferTileSize(const INT64 w, const INT64 h) : CoordinatePairINT<INT64>(w, h) {};
  /** @return The width in pixels. */
  INT64 w() const;
  /** @return The height in pixels. */
  INT64 h() const;
};

/**
 * Represents the number of tiles representing a buffer.
 */
class BufferTileIndex : public CoordinatePairINT<INT64> {
public:
  BufferTileIndex()=default;
  BufferTileIndex(const CoordinatePair<INT64>& buffer_tile_index) : CoordinatePairINT<INT64>( buffer_tile_index) {};
  BufferTileIndex(const CoordinatePairINT<INT64>& buffer_tile_index) : CoordinatePairINT<INT64>( buffer_tile_index) {};
  BufferTileIndex(const BufferTileIndex& buffer_tile_index) : CoordinatePairINT<INT64>(buffer_tile_index) {};
  using CoordinatePairINT<INT64>::operator=;
  BufferTileIndex& operator=(const BufferTileIndex& buffer_tile_index)=default;
  /**
   * @param i The index in the i direction.
   * @param j The index in the j direction.
   */
  BufferTileIndex(const INT64 i, const INT64 j) : CoordinatePairINT<INT64>(i, j) {};
  /** @return The index in the x direction. */
  INT64 i() const;
  /** @return The index in the y direction. */
  INT64 j() const;
};


/**
 * Represents a size in a buffer.
 */
class BufferPixelSize : public CoordinatePairINT<INT64> {
public:
  BufferPixelSize()=default;
  BufferPixelSize(const CoordinatePair<INT64>& viewport_pixel_size) : CoordinatePairINT<INT64>( viewport_pixel_size) {};
  BufferPixelSize(const CoordinatePairINT<INT64>& viewport_pixel_size) : CoordinatePairINT<INT64>( viewport_pixel_size) {};
  BufferPixelSize(const BufferPixelSize& viewport_pixel_size) : CoordinatePairINT<INT64>(viewport_pixel_size) {};
  using CoordinatePairINT<INT64>::operator=;
  BufferPixelSize& operator=(const BufferPixelSize& viewport_pixel_size)=default;
  /**
   * @param w The width in pixels.
   * @param h The height in pixels.
   */
  BufferPixelSize(const INT64 w, const INT64 h) : CoordinatePairINT<INT64>(w, h) {};
  /** @return The width in pixels. */
  INT64 w() const;
  /** @return The height in pixels. */
  INT64 h() const;
};

/**
 * Represents a coorindate in a buffer in pixels using the upper left
 * corner as the origin.
 */
class BufferPixelCoordinate : public CoordinatePairINT<INT64> {
public:
  BufferPixelCoordinate()=default;
  BufferPixelCoordinate(const CoordinatePair<INT64>& viewport_pixel_coordinate ) : CoordinatePairINT<INT64>(viewport_pixel_coordinate) {};
  BufferPixelCoordinate(const CoordinatePairINT<INT64>& viewport_pixel_coordinate ) : CoordinatePairINT<INT64>(viewport_pixel_coordinate) {};
  BufferPixelCoordinate(const BufferPixelCoordinate& viewport_pixel_coordinate) : CoordinatePairINT<INT64>(viewport_pixel_coordinate) {};
  using CoordinatePairINT<INT64>::operator=;
  BufferPixelCoordinate& operator=(const BufferPixelCoordinate& viewport_pixel_coordinate)=default;
  /**
   * @param x The x coordinate in pixels.
   * @param y The y coorindate in pixels.
   */
  BufferPixelCoordinate(const INT64 x, const INT64 y) : CoordinatePairINT<INT64>(x, y) {};
  /**
   * Convert a grid coordinate to a viewport pixel coordinate.
   *
   * @param grid_coordinate The GridCoordinate to convert.
   * @param zoom The zoom out value, used to convert between pixels on the
   *             image grid and viewport.
   * @param grid_coordinate_pixel_0 The grid coordinate of the origin
   *                                of the viewport.
   * @param buffer_pixel_size The size of the viewport in pixels.
   */
  BufferPixelCoordinate(GridCoordinate& grid_coordinate,
                        FLOAT64 zoom,
                        GridCoordinate& grid_coordinate_pixel_0,
                        BufferPixelSize& buffer_pixel_size);
  /** @return The x coordinate in pixels. */
  INT64 x() const;
  /** @return The y coordinate in pixels. */
  INT64 y() const;
};

#endif
