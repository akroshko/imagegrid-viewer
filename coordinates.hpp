/**
 * Header to define the classes that represent different coordinates.
 */
#ifndef COORDINATES_HPP
#define COORDINATES_HPP

#include "common.hpp"

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
 * Represents the size of the grid in integer units of images.
 *
 * For example, representing that a grid or part of a grid that is 20
 * images wide and 15 images high is a 20x15 grid of images.
 */
class GridImageSize {
public:
  GridImageSize();
  GridImageSize(const GridImageSize& grid_image_size);
  /**
   * @param wimage
   * @param himage
   */
  GridImageSize(const INT64 wimage, const INT64 himage);
  GridImageSize& operator=(const GridImageSize& grid_image_size);
  /** @return The width. */
  INT64 wimage() const;
  /** @return The height. */
  INT64 himage() const;
private:
  INT64 _wimage;
  INT64 _himage;
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
  GridCoordinateSize(const GridCoordinateSize& grid_coordinate_size);
  GridCoordinateSize(const FLOAT64 wgrid, const FLOAT64 hgrid);
  GridCoordinateSize& operator=(const GridCoordinateSize& grid_coordinate_size);
  /** @return The width. */
  FLOAT64 wgrid() const;
  /** @return The height. */
  FLOAT64 hgrid() const;
private:
  FLOAT64 _wgrid;
  FLOAT64 _hgrid;
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
  GridCoordinate(const GridCoordinate& grid_coordinate);
  GridCoordinate(const FLOAT64 xgrid, FLOAT64 ygrid);
  GridCoordinate(ViewportPixelCoordinate& viewport_pixel_coordinate,
                 FLOAT64 zoom,
                 ViewportPixelSize& viewport_pixel_size,
                 GridCoordinate& viewport_grid_coordinate,
                 GridPixelSize& max_image_pixel_size);
  GridCoordinate& operator=(const GridCoordinate& grid_coordinate);
  /** @return The x coordinate. */
  FLOAT64 xgrid() const;
  /** @return The y coordinate. */
  FLOAT64 ygrid() const;
  /** @return If values are valid. */
  bool invalid() const;
private:
  FLOAT64 _xgrid;
  FLOAT64 _ygrid;
};

/**
 * Represents indices a grid of images with the origin in the top-left
 * corner.
 */
class GridIndex {
public:
  GridIndex();
  GridIndex(const GridIndex& grid_index);
  GridIndex(const INT64 igrid, INT64 jgrid);
  GridIndex(ViewportPixelCoordinate& viewport_pixel_coordinate);
  GridIndex& operator=(const GridIndex& grid_index);
  /** @return The i coordinate. */
  INT64 i_grid() const;
  /** @return The j coordinate. */
  INT64 j_grid() const;
  // /** @return If values are valid. */
  // bool invalid() const;
private:
  INT64 _igrid;
  INT64 _jgrid;
};

/**
 * Represents coordinates with a grid, i.e., subgrid with the origin
 * at the top-right corner.
 *
 * Subgrids are generally used when the logical grid squares need
 * partitioning either because of the nature of the data (see
 * (E)ast/(W)est maps in the 50K Canadian NTS system) or for
 * performance reasons (grid squares larger than 16384x16384 that
 * makes sense for textures).
 *
 */
class SubGridIndex {
public:
  SubGridIndex();
  SubGridIndex(const SubGridIndex& subgrid_index);
  SubGridIndex(const INT64 igrid, INT64 jgrid);
  SubGridIndex& operator=(const SubGridIndex& subgrid_index);
  /** @return The i coordinate. */
  INT64 i_subgrid() const;
  /** @return The j coordinate. */
  INT64 j_subgrid() const;
  /** @return If values are valid. */
  bool invalid() const;
private:
  INT64 _i_subgrid;
  INT64 _j_subgrid;
};

/**
 * Represents a size on the image grid in pixels.  For instance if
 * each image was 4000x2500 pixels, a rectangle represented by
 * GridCoordinateSize of 3.2x2.7 would be 12800x6750.
 */
class GridPixelSize {
public:
  GridPixelSize();
  GridPixelSize(const GridPixelSize& grid_pixel_size);
  GridPixelSize(INT64 wpixel, INT64 hpixel);
  GridPixelSize& operator=(const GridPixelSize& grid_pixel_size);
  /** @return The width. */
  INT64 wpixel() const;
  /** @return The height. */
  INT64 hpixel() const;
private:
  INT64 _wpixel;
  INT64 _hpixel;
};

/**
 * Represents a coorindate on an image in pixels.  For instance
 * if each image on the grid was 2000x1500 pixels.  The coordinate
 * represented by GridCoordinate of (2.4,1.6) would be 800x900.
 */
class ImagePixelCoordinate {
public:
  ImagePixelCoordinate();
  ImagePixelCoordinate(const ImagePixelCoordinate& grid_pixel_coordinate);
  ImagePixelCoordinate(INT64 xpixel, INT64 ypixel);
  /**
   * Initialize a pixel coordinate on an image with an image grid coordinate.
   *
   * @param grid_coordinate The coordinate on the grid.
   * @param grid_pixel_size The pixel size of the images on the grid.
   */
  ImagePixelCoordinate(GridCoordinate& grid_coordinate,
                       GridPixelSize& grid_pixel_size);
  ImagePixelCoordinate& operator=(const ImagePixelCoordinate& grid_pixel_coordinate);
  /** @return The x coordinate. */
  INT64 xpixel() const;
  /** @return The y coordinate. */
  INT64 ypixel() const;
private:
  INT64 _xpixel;
  INT64 _ypixel;
};

/**
 * Represents a size on the viewport in pixels.
 */
class ViewportPixelSize {
public:
  ViewportPixelSize();
  ViewportPixelSize(const ViewportPixelSize& viewport_pixel_size);
  ViewportPixelSize(INT64 wpixel, INT64 xpixel);
  ViewportPixelSize& operator= (const ViewportPixelSize& viewport_pixel_size);
  /** @return The width. */
  INT64 wpixel() const;
  /** @return The height. */
  INT64 hpixel() const;
private:
  INT64 _wpixel;
  INT64 _hpixel;
};

/**
 * Represents a coorindate on the viewport in pixels using the upper
 * left corner as the origin.
 */
class ViewportPixelCoordinate {
public:
  ViewportPixelCoordinate();
  ViewportPixelCoordinate(const ViewportPixelCoordinate& viewport_pixel_coordinate);
  ViewportPixelCoordinate(INT64 xpixel, INT64 ypixel);
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
  ViewportPixelCoordinate(GridCoordinate& grid_coordinate,
                          FLOAT64 zoom,
                          GridCoordinate& grid_coordinate_pixel_0,
                          ViewportPixelSize& viewport_pixel_size);
  ViewportPixelCoordinate& operator=(const ViewportPixelCoordinate& viewport_pixel_coordinate);
  /** @return The x coordinate. */
  INT64 xpixel() const;
  /** @return The y coordinate. */
  INT64 ypixel() const;
private:
  INT64 _xpixel;
  INT64 _ypixel;
};


#endif
