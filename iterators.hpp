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
 * Iterate over an ImageGrid.  Generally in an optimal pattern
 */
class ImageGridIterator {
public:
  ImageGridIterator()=delete;
  ImageGridIterator(INT_T w, INT_T h, INT_T current_grid_x, INT_T current_grid_y);
  ~ImageGridIterator()=default;
  bool get_next(INT_T &i, INT_T &j);
private:
  INT_T _w;
  INT_T _h;
  INT_T _x_current;
  INT_T _y_current;
  std::queue<std::array<INT_T,2>> _index_values;
};

#endif
