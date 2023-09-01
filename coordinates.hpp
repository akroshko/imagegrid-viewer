/**
 * Header to define the classes that represent different coordinates.
 */
#ifndef COORDINATES_HPP
#define COORDINATES_HPP

#include "debug.hpp"
#include "error.hpp"
#include "types.hpp"
#include <memory>

class GridCoordinate;
class GridPixelCoordinate;
class ViewportPixelCoordinate;
class ViewportPixelSize;

/**
 * Represents the size of the grid in integer units of images.
 *
 * For example, representing that a grid or part of a grid that is 20
 * images wide and 15 images high is a 20x15 grid of images.
 */
class GridImageSize {
public:
  GridImageSize();
  GridImageSize(const GridImageSize &grid_image_size);
  /**
   * @param wimage
   * @param himage
   */
  GridImageSize(const INT_T wimage, const INT_T himage);
  /** @return The width. */
  INT_T wimage() const;
  /** @return The height. */
  INT_T himage() const;
private:
  INT_T _wimage;
  INT_T _himage;
};

/**
 * Represents a size on the image grid in floating point units of
 * images.
 *
 * For example, a rectangle that is 3.2 image widths wide and 2.7
 * image heights high would be represented by 3.2x2.7.
 */
class GridCoordinateSize {
public:
  GridCoordinateSize();
  GridCoordinateSize(const GridCoordinateSize &grid_coordinate_size);
  GridCoordinateSize(const FLOAT_T wgrid, const FLOAT_T hgrid);
  /** @return The width. */
  FLOAT_T wgrid() const;
  /** @return The height. */
  FLOAT_T hgrid() const;
private:
  FLOAT_T _wgrid;
  FLOAT_T _hgrid;
};

/**
 * Represents coordinates on a grid of images with the origin in the
 * top-left corner.
 *
 * For example, a point 2.4 image widths right from the origin and 1.6
 * image height down would be represented by the coordinate (2.4,1.6).
 */
class GridCoordinate {
public:
  GridCoordinate();
  GridCoordinate(const GridCoordinate &grid_coordinate_size);
  GridCoordinate(const FLOAT_T xgrid, FLOAT_T ygrid);
  /** @return The x coordinate. */
  FLOAT_T xgrid() const;
  /** @return The y coordinate. */
  FLOAT_T ygrid() const;
  /** @return If values are valid. */
  bool invalid() const;
private:
  FLOAT_T _xgrid;
  FLOAT_T _ygrid;
};

/**
 * Represents a size on the image grid in pixels.  For instance if
 * each image was 4000x2500 pixels, a rectangle represented by
 * GridCoordinateSize of 3.2x2.7 would be 12800x6750.
 */
class GridPixelSize {
public:
  GridPixelSize();
  GridPixelSize(const GridPixelSize &grid_pixel_size);
  GridPixelSize(INT_T wpixel, INT_T hpixel);
  /** @return The width. */
  INT_T wpixel() const;
  /** @return The height. */
  INT_T hpixel() const;
private:
  INT_T _wpixel;
  INT_T _hpixel;
};

/**
 * Represents a coorindate on the image grid in pixels.  For instance
 * if each image on the grid was 2000x1500 pixels.  The coordinate
 * represented by GridCoordinate of (2.4,1.6) would be 4800x2400.
 */
class GridPixelCoordinate {
public:
  GridPixelCoordinate(const GridPixelCoordinate &grid_pixel_coordinate);
  GridPixelCoordinate(INT_T xpixel, INT_T ypixel);
  /** @return The x coordinate. */
  INT_T xpixel() const;
  /** @return The y coordinate. */
  INT_T ypixel() const;
private:
  INT_T _xpixel;
  INT_T _ypixel;
};

/**
 * Represents a size on the viewport in pixels.
 */
class ViewportPixelSize {
public:
  ViewportPixelSize();
  ViewportPixelSize(const ViewportPixelSize &viewport_pixel_size);
  ViewportPixelSize(INT_T wpixel, INT_T xpixel);
  /** @return The width. */
  INT_T wpixel() const;
  /** @return The height. */
  INT_T hpixel() const;
private:
  INT_T _wpixel;
  INT_T _hpixel;
};

/**
 * Represents a coorindate on the viewport in pixels using the upper
 * left corner as the origin.
 */
class ViewportPixelCoordinate {
public:
  ViewportPixelCoordinate();
  ViewportPixelCoordinate(const ViewportPixelCoordinate &viewport_pixel_coordinate);
  ViewportPixelCoordinate(INT_T xpixel, INT_T ypixel);
  ViewportPixelCoordinate(const GridPixelCoordinate &grid_pixel_coordinate);
  /**
   * Convert a grid coordinate to a viewport pixel coordinate.
   *
   * @param grid_coordinate The GridCoordinate to convert.
   * @param zoom The zoom level, used to convert between pixels on the
   *             image grid and viewport.
   * @param grid_coordinate_pixel_0 The grid coordinate of the origin
   *                                of the viewport.
   * @param viewport_pixel_size The size of the viewport in pixels.
   */
  ViewportPixelCoordinate(GridCoordinate &grid_coordinate, FLOAT_T zoom, GridCoordinate &grid_coordinate_pixel_0, const ViewportPixelSize &viewport_pixel_size);

  /** @return The x coordinate. */
  INT_T xpixel() const;
  /** @return The y coordinate. */
  INT_T ypixel() const;
private:
  INT_T _xpixel;
  INT_T _ypixel;
};


#endif
