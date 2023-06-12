/**
 * Header to define the classes that represent different coordinates.
 */
#ifndef COORDINATES_HPP
#define COORDINATES_HPP

#include "debug.hpp"
#include "error.hpp"
#include "types.hpp"

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
  /**
   * Copy constructor.
   */
  GridImageSize(GridImageSize *grid_image_size);
  /**
   * @param wimage
   * @param himage
   */
  GridImageSize(INT_T wimage, INT_T himage);
  /** Accessor for the width */
  INT_T wimage();
  /** Accessor for the height */
  INT_T himage();
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
  GridCoordinateSize(GridCoordinateSize *grid_coordinate_size);
  GridCoordinateSize(FLOAT_T wgrid, FLOAT_T hgrid);
  /** Accessor for the width */
  FLOAT_T wgrid();
  /** Accessor for the height */
  FLOAT_T hgrid();
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
  GridCoordinate(GridCoordinate *grid_coordinate_size);
  GridCoordinate(FLOAT_T xgrid, FLOAT_T ygrid);
  /** Accessor for the x coordinate */
  FLOAT_T xgrid();
  /** Accessor for the y coordinate */
  FLOAT_T ygrid();
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
  GridPixelSize(GridPixelSize *grid_pixel_size);
  GridPixelSize(INT_T wpixel, INT_T hpixel);
  /** Accessor for the width */
  INT_T wpixel();
  /** Accessor for the height */
  INT_T hpixel();
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
  GridPixelCoordinate(GridPixelCoordinate *grid_pixel_coordinate);
  GridPixelCoordinate(INT_T xpixel, INT_T ypixel);
  /** Accessor for the x coordinate */
  INT_T xpixel();
  /** Accessor for the y coordinate */
  INT_T ypixel();
private:
  INT_T _xpixel;
  INT_T _ypixel;
};


/**
 * Represents a size on the viewport in pixels.
 */
class ViewportPixelSize {
public:
  ViewportPixelSize(ViewportPixelSize *viewport_pixel_size);
  ViewportPixelSize(INT_T wpixel, INT_T xpixel);
  /** Accessor for the width */
  INT_T wpixel();
  /** Accessor for the height */
  INT_T hpixel();
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
  ViewportPixelCoordinate(ViewportPixelCoordinate *viewport_pixel_coordinate);
  ViewportPixelCoordinate(INT_T xpixel, INT_T ypixel);
  ViewportPixelCoordinate(GridPixelCoordinate grid_pixel_coordinate);
  /**
   * Convert a grid coordinate to a viewport pixel coordinate.
   *
   * @param grid_coordinate The GridCoordinate to convert.
   *
   * @param zoom The zoom level, used to convert between pixels on the
   *             image grid and viewport.
   *
   * @param grid_coordinate_pixel_0 The grid coordinate of the origin
   *                                of the viewport.
   *
   * @param viewport_pixel_size The size of the viewport in pixels.
   */
  ViewportPixelCoordinate(GridCoordinate *grid_coordinate, FLOAT_T zoom, GridCoordinate *grid_coordinate_pixel_0, ViewportPixelSize *viewport_pixel_size);

  /** Accessor for the x coordinate */
  INT_T xpixel();
  /** Accessor for the y coordinate */
  INT_T ypixel();
private:
  INT_T _xpixel;
  INT_T _ypixel;
};


#endif
