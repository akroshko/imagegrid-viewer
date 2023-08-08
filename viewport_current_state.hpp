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
  /*
   * Find zoom index from a zoom value.
   *
   * @param zoom The zoom value to convert to an index.
   *
   * @param min_zoom_index The minimum zoom value to return.
   *
   * @param max_zoom_index The maximum zoom value to return.
   *
   * @return The bounded zoom index corresponding to the zoom value.
   */
  static INT_T find_zoom_index_bounded(FLOAT_T zoom,
                                       INT_T min_zoom_index,
                                       INT_T max_zoom_index);
  /*
   * Find the maximum upper bound a zoom_index corresponds to.
   *
   * @param zoom_index An index into an array of zoomed textures or images.
   *
   * @param the corresponding upper bound on the zoom value.
   */
  static FLOAT_T find_zoom_upper(INT_T zoom_index);
  /**
   * Find the leftmost visible coordinate.  This is fairly
   * conservative because it assumes max screen size and max zoom
   * based on zoom<-->zoom_index conversion.
   *
   * @param xgrid The x coordinate on the grid.
   *
   * @param ygrid The y coordinate on the grid.
   *
   * @param max_wpixel The maximum height of the images.
   *
   * @param max_hpixel The maximum width of the images.
   *
   * @param zoom The current zoom.
   *
   * @return The leftmost visible coordinate.
   */
  static FLOAT_T find_leftmost_visible(FLOAT_T xgrid, FLOAT_T ygrid,
                                       INT_T max_wpixel, INT_T max_hpixel,
                                       FLOAT_T zoom);
  /**
   * Find the rightmost visible coordinate.  This is fairly
   * conservative because it assumes max screen size and max zoom
   * based on zoom<-->zoom_index conversion.
   *
   * @param xgrid The x coordinate on the grid.
   *
   * @param ygrid The y coordinate on the grid.
   *
   * @param max_wpixel The maximum height of the images.
   *
   * @param max_hpixel The maximum width of the images.
   *
   * @param zoom The current zoom.
   *
   * @return The rightmost visible coordinate.
   */
  static FLOAT_T find_rightmost_visible(FLOAT_T xgrid, FLOAT_T ygrid,
                                        INT_T max_wpixel, INT_T max_hpixel,
                                        FLOAT_T zoom);
  /**
   * Find the topmost visible coordinate.  This is fairly conservative
   * because it assumes max screen size and max zoom based on
   * zoom<-->zoom_index conversion.
   *
   * @param xgrid The x coordinate on the grid.
   *
   * @param ygrid The y coordinate on the grid.
   *
   * @param max_wpixel The maximum height of the images.
   *
   * @param max_hpixel The maximum width of the images.
   *
   * @param zoom The current zoom.
   *
   * @return The topmost visible coordinate.
   */
  static FLOAT_T find_topmost_visible(FLOAT_T xgrid, FLOAT_T ygrid,
                                      INT_T max_wpixel, INT_T max_hpixel,
                                      FLOAT_T zoom);
  /**
   * Find the bottommost visible coordinate.  This is fairly conservative
   * because it assumes max screen size and max zoom based on
   * zoom<-->zoom_index conversion.
   *
   * @param xgrid The x coordinate on the grid.
   *
   * @param ygrid The y coordinate on the grid.
   *
   * @param max_wpixel The maximum height of the images.
   *
   * @param max_hpixel The maximum width of the images.
   *
   * @param zoom The current zoom.
   *
   * @return The bottommost visible coordinate.
   */
  static FLOAT_T find_bottommost_visible(FLOAT_T xgrid, FLOAT_T ygrid,
                                         INT_T max_wpixel, INT_T max_hpixel,
                                         FLOAT_T zoom);
  /**
   * Find whether an index is visible on the grid. This is fairly
   * conservative because it assumes max screen size and max zoom
   * based on zoom<-->zoom_index conversion.
   *
   * @param i the index along the width of the grid.
   *
   * @param j the index along the height of the grid.
   *
   * @param xgrid The x coordinate on the grid.
   *
   * @param ygrid The y coordinate on the grid.
   *
   * @param max_wpixel The maximum height of the images.
   *
   * @param max_hpixel The maximum width of the images.
   *
   * @param zoom The current zoom.
   *
   * @return Whether the index is visible
   */
  static bool grid_index_visible(INT_T i, INT_T j,
                                 FLOAT_T xgrid, FLOAT_T ygrid,
                                 INT_T max_wpixel, INT_T max_hpixel,
                                 FLOAT_T zoom);
private:
  static FLOAT_T _find_max_zoom(FLOAT_T zoom_index);
  FLOAT_T zoom=NAN;
  GridCoordinate grid;
  FLOAT_T zoom_last=NAN;
  GridCoordinate grid_last;
  bool been_updated=false;
  std::mutex using_mutex;
};

#endif
