#include "debug.hpp"
#include "error.hpp"
#include "types.hpp"
#include "defaults.hpp"
#include "iterators.hpp"
#include "viewport_current_state.hpp"

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


// this is a pretty bad implementation, but encapsulates this looping code at least
ImageGridIteratorFull::ImageGridIteratorFull(INT_T w_image_grid, INT_T h_image_grid,
                                             const ViewPortCurrentState& viewport_current_state) {
  this->_w=w_image_grid;
  this->_h=h_image_grid;
  auto current_grid_x=viewport_current_state.current_grid_coordinate().xgrid();
  auto current_grid_y=viewport_current_state.current_grid_coordinate().ygrid();
  // TODO need a good iterator class for this type of work
  // load the one we are looking at
  // do the center
  // pretend we are in the corner of the grid if we are outside the grid
  auto adjusted_grid_x=(INT_T)floor(current_grid_x);
  auto adjusted_grid_y=(INT_T)floor(current_grid_y);
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
  for (INT_T r=1l; r <= (std::max(w_image_grid,h_image_grid)); r++) {
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

// TODO: how I will build up this iterator
//       - make sure rest of top layer is loaded
//       - finally iterate out from immediate underneath
// TODO: so the new and improved will need
//       - current zoom level and index
//       - current location
//       - calculate bounds (should this calculation be switched from texture_update to viewport)
// TODO: will need a default to do if viewport unavailable
ImageGridIteratorVisible::ImageGridIteratorVisible(INT_T w_image_grid, INT_T h_image_grid,
                                                   const ViewPortCurrentState& viewport_current_state) {
  this->_w=w_image_grid;
  this->_h=h_image_grid;
  auto current_grid_x=viewport_current_state.current_grid_coordinate().xgrid();
  auto current_grid_y=viewport_current_state.current_grid_coordinate().ygrid();
  auto leftmost_visible=ViewPortTransferState::find_leftmost_visible(viewport_current_state);
  auto rightmost_visible=ViewPortTransferState::find_rightmost_visible(viewport_current_state);
  auto topmost_visible=ViewPortTransferState::find_topmost_visible(viewport_current_state);
  auto bottommost_visible=ViewPortTransferState::find_bottommost_visible(viewport_current_state);
  auto visible_imin=floor(leftmost_visible);
  auto visible_imax=floor(rightmost_visible);
  auto visible_jmin=floor(topmost_visible);
  auto visible_jmax=floor(bottommost_visible);
  auto center_i=(INT_T)floor(current_grid_x);
  auto center_j=(INT_T)floor(current_grid_y);
  auto r_i=std::max(std::abs(center_i-visible_imin),
                    std::abs(visible_imax-center_i));
  auto r_j=std::max(std::abs(center_j-visible_jmin),
                    std::abs(visible_jmax-center_j));
  auto r_visible=std::max(std::abs(r_i),std::abs(r_j));
  // check visible layer first
  for (INT_T r=0l; r <= r_visible; r++) {
    for (INT_T i=center_i-r; i <= center_i+r; i++) {
      auto j=center_j-r;
      if (i >= 0 && i < w_image_grid && j >= 0 && j < h_image_grid) {
        this->_index_values.push({i,j});
      }
    }
    for (INT_T j=center_j-r; j <= center_j+r; j++) {
      auto i=center_i+r;
      if (i >= 0 && i < w_image_grid && j >= 0 && j < h_image_grid) {
        this->_index_values.push({i,j});
      }
    }
    for (INT_T i=center_i+r; i >= center_i-r; i--) {
      auto j=center_j+r;
      if (i >= 0 && i < w_image_grid && j >= 0 && j < h_image_grid) {
        this->_index_values.push({i,j});
      }
    }
    for (INT_T j=center_j+r; j >= center_j-r; j--) {
      auto i=center_i-r;
      if (i >= 0 && i < w_image_grid && j >= 0 && j < h_image_grid) {
        this->_index_values.push({i,j});
      }
    }
  }
}
