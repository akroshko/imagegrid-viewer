#ifndef ITERATORS_HPP
#define ITERATORS_HPP

#include "debug.hpp"
#include "error.hpp"
#include "types.hpp"
#include "defaults.hpp"
// C++ headers
#include <array>
#include <queue>

/**
 *  Base class for iterating the load order of imageson the image grid.
 */
class ImageGridIterator {
protected:
  ImageGridIterator()=default;
public:
  ~ImageGridIterator()=default;
  /**
   * Get the next index of i,j.
   *
   * @param i The next index along a row.
   *
   * @param j The next index along a column.
   */
  virtual bool get_next(INT_T &i, INT_T &j);
protected:
  INT_T _w;
  INT_T _h;
  INT_T _x_current;
  INT_T _y_current;
  std::queue<std::array<INT_T,2>> _index_values;
};

/**
 * Iterate over an ImageGrid.  Generally in an optimal pattern
 */
class ImageGridIteratorFull : public ImageGridIterator {
public:
  ImageGridIteratorFull()=delete;
  /**
   * Iterate over the full grid starting from the center of
   * visibility.  Finds closest coorindate to center of visibility if
   * center of visibility is not on the grid.
   *
   * @param w_image_grid width of the image grid in images
   *
   * @param h_image_grid height of the image grid in images
   *
   * @param max_wpixel max pixel width of images
   *
   * @param max_hpixel max pixel height of images
   *
   * @param current_grid_x x coordinate of center of viewport
   *
   * @param current_grid_y y coordinate of center of viewport
   *
   * @param zoom current zoom
   */
  ImageGridIteratorFull(INT_T w_image_grid, INT_T h_image_grid,
                        INT_T max_wpixel, INT_T max_hpixel,
                        FLOAT_T current_grid_x, FLOAT_T current_grid_y,
                        FLOAT_T zoom);
  ~ImageGridIteratorFull()=default;
};

class ImageGridIteratorVisible : public ImageGridIterator {
public:
  ImageGridIteratorVisible()=delete;
  /**
   * Iterate over visible image grid squares. Starting at center of
   * visibility.
   *
   * @param w_image_grid width of the image grid in images
   *
   * @param h_image_grid height of the image grid in images
   *
   * @param max_wpixel max pixel width of images
   *
   * @param max_hpixel max pixel height of images
   *
   * @param current_grid_x x coordinate of center of viewport
   *
   * @param current_grid_y y coordinate of center of viewport
   *
   * @param zoom current zoom
   */
  ImageGridIteratorVisible(INT_T w_image_grid, INT_T h_image_grid,
                           INT_T max_wpixel, INT_T max_hpixel,
                           FLOAT_T current_grid_x, FLOAT_T current_grid_y,
                           FLOAT_T zoom);
  ~ImageGridIteratorVisible()=default;
};

#endif
