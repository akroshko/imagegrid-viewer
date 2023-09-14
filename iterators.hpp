#ifndef ITERATORS_HPP
#define ITERATORS_HPP

#include "common.hpp"
#include "viewport_current_state.hpp"
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
   * @param j The next index along a column.
   * @return Whether to continue.
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
   * @param w_image_grid Width of the image grid in images.
   * @param h_image_grid Height of the image grid in images.
   * @param viewport_current_state The current state of the viewport.
   */
  ImageGridIteratorFull(INT_T w_image_grid, INT_T h_image_grid,
                        const ViewPortCurrentState& viewport_current_state);
  ~ImageGridIteratorFull()=default;
};

class ImageGridIteratorVisible : public ImageGridIterator {
public:
  ImageGridIteratorVisible()=delete;
  /**
   * Iterate over visible image grid squares. Starting at center of
   * visibility.
   *
   * @param w_image_grid Width of the image grid in images.
   * @param h_image_grid Height of the image grid in images.
   * @param viewport_current_state The current state of the viewport.
   */
  ImageGridIteratorVisible(INT_T w_image_grid, INT_T h_image_grid,
                           const ViewPortCurrentState& viewport_current_state);
  ~ImageGridIteratorVisible()=default;
};

#endif
