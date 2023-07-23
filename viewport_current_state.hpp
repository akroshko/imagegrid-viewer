/**
 * Header representing an observer pattern for finding current state of the viewport.
 */
#ifndef VIEWPORT_CURRENT_STATE_HPP
#define VIEWPORT_CURRENT_STATE_HPP

// local headers
#include "debug.hpp"
#include "error.hpp"
#include "types.hpp"
#include "defaults.hpp"
#include "coordinates.hpp"
#include <cmath>
#include <mutex>

/**
 * Class for transfering the current state of the viewport between
 * threads. Updated every single time the viewport changes.  Should be
 * a singleton class that is only meant to be produced in one place
 * and consumed in another.
 */
class ViewPortCurrentState {
// TODO: make sure this class is not moveable or copyable
public:
  ViewPortCurrentState();
  ViewPortCurrentState(const ViewPortCurrentState&)=delete;
  ViewPortCurrentState(const ViewPortCurrentState&&)=delete;
  ViewPortCurrentState& operator=(const ViewPortCurrentState&)=delete;
  ViewPortCurrentState& operator=(const ViewPortCurrentState&&)=delete;
  void UpdateGridValues(FLOAT_T zoom, const GridCoordinate &grid);
  bool GetGridValues(FLOAT_T &zoom, GridCoordinate &grid);
  // TODO: some utility functions to implement
  static INT_T find_zoom_index(FLOAT_T zoom);
  static FLOAT_T find_zoom(INT_T zoom_index);
  // I have both INT_T zoom_index and FLOAT_T zoom because I want to
  // avoid unnecessary int->float->int conversins
  static FLOAT_T find_leftmost_visible(FLOAT_T xgrid, FLOAT_T ygrid,
                                       INT_T max_wpixel, INT_T max_hpixel,
                                       INT_T zoom_index);
  static FLOAT_T find_leftmost_visible(FLOAT_T xgrid, FLOAT_T ygrid,
                                       INT_T max_wpixel, INT_T max_hpixel,
                                       FLOAT_T zoom);
  static FLOAT_T find_rightmost_visible(FLOAT_T xgrid, FLOAT_T ygrid,
                                        INT_T max_wpixel, INT_T max_hpixel,
                                        INT_T zoom_index);
  static FLOAT_T find_rightmost_visible(FLOAT_T xgrid, FLOAT_T ygrid,
                                        INT_T max_wpixel, INT_T max_hpixel,
                                        FLOAT_T zoom);
  static FLOAT_T find_topmost_visible(FLOAT_T xgrid, FLOAT_T ygrid,
                                      INT_T max_wpixel, INT_T max_hpixel,
                                      INT_T zoom_index);
  static FLOAT_T find_topmost_visible(FLOAT_T xgrid, FLOAT_T ygrid,
                                      INT_T max_wpixel, INT_T max_hpixel,
                                      FLOAT_T zoom);
  static FLOAT_T find_bottommost_visible(FLOAT_T xgrid, FLOAT_T ygrid,
                                         INT_T max_wpixel, INT_T max_hpixel,
                                         INT_T zoom_index);
  static FLOAT_T find_bottommost_visible(FLOAT_T xgrid, FLOAT_T ygrid,
                                         INT_T max_wpixel, INT_T max_hpixel,
                                         FLOAT_T zoom);
private:
  static FLOAT_T _find_max_zoom(INT_T zoom_index);
  static FLOAT_T _find_max_zoom(FLOAT_T zoom_index);
  FLOAT_T zoom=NAN;
  GridCoordinate grid;
  FLOAT_T zoom_last=NAN;
  GridCoordinate grid_last;
  bool been_updated=false;
  std::mutex using_mutex;
};

#endif
