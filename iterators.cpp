#include "debug.hpp"
#include "error.hpp"
#include "types.hpp"
#include "defaults.hpp"
#include "iterators.hpp"

// this is a pretty bad implementation, but encapsulates this looping code at least
ImageGridIterator::ImageGridIterator(INT_T w, INT_T h, INT_T current_grid_x, INT_T current_grid_y) {
  this->_w=w;
  this->_h=h;
  // TODO need a good iterator class for this type of work
  // load the one we are looking at
  // do the center
  // pretend we are in the corner of the grid if we are outside the grid
  auto adjusted_grid_x=current_grid_x;
  auto adjusted_grid_y=current_grid_y;
  if (current_grid_x > this->_w) {
    adjusted_grid_x=this->_w;
  } else if (current_grid_x < 0) {
    adjusted_grid_x=0;
  }
  if (current_grid_y > this->_h) {
    adjusted_grid_y=this->_h;
  } else if (current_grid_y < 0) {
    adjusted_grid_y=0;
  }
  auto i=adjusted_grid_x;
  auto j=adjusted_grid_y;
  this->_index_values.push({i,j});
  // load near the viewport one first then work way out
  for (INT_T r=1l; r <= (std::max(w,h)); r++) {
    // start at top left corner, go to top right corner
    for (INT_T i=adjusted_grid_x-r; i <= adjusted_grid_x+r; i++) {
      auto j=adjusted_grid_y-r;
      this->_index_values.push({i,j});
    }
    // go to bottom right corner
    for (INT_T j=adjusted_grid_y-r; j <= adjusted_grid_y+r; j++) {
      auto i=adjusted_grid_x+r;
      this->_index_values.push({i,j});
    }
    // go to bottom left corner
    for (INT_T i=adjusted_grid_x+r; i >= adjusted_grid_x-r; i--) {
      auto j=adjusted_grid_y+r;
      this->_index_values.push({i,j});
    }
    // go to top right corner
    for (INT_T j=adjusted_grid_y+r; j >= adjusted_grid_y-r; j--) {
      auto i=adjusted_grid_x-r;
      this->_index_values.push({i,j});
    }
  }
}

bool ImageGridIterator::get_next(INT_T &i, INT_T &j) {
  if (this->_index_values.size() > 0) {
    auto popped_array=this->_index_values.front();
    this->_index_values.pop();
    i=popped_array[0];
    j=popped_array[1];
    return true;
  } else {
    return false;
  }
}
