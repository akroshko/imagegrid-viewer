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
// C++ headers
#include <mutex>
// C headers
#include <cmath>

class ViewPortCurrentState {
public:
  ViewPortCurrentState(GridCoordinate current_grid_coordinate,
                       GridPixelSize max_image_size,
                       FLOAT_T zoom,
                       ViewportPixelSize screen_size,
                       bool valid_update);
  // these are enabled because this class is used for data transfer
  // and created and returned from a function
  ViewPortCurrentState(ViewPortCurrentState&)=default;
  ViewPortCurrentState(ViewPortCurrentState&&)=default;
  ViewPortCurrentState& operator=(ViewPortCurrentState&)=delete;
  ViewPortCurrentState& operator=(ViewPortCurrentState&&)=delete;
  GridCoordinate current_grid_coordinate() const;
  /** @return The maximum image size. */
  GridPixelSize max_image_size() const;
  /** @return The current zoom. */
  FLOAT_T zoom() const;
  /** @return The current screen size. */
  ViewportPixelSize screen_size() const;
  /** @return Is this a valid update of the viewport. */
  bool valid_update() const;
private:
  friend class ViewPortTransferState;
  GridCoordinate _current_grid_coordinate;
  GridPixelSize _max_image_size;
  FLOAT_T _zoom;
  ViewportPixelSize _screen_size;
  bool _valid_update;
};

/**
 * Class for transfering the current state of the viewport between
 * threads. Updated every single time the viewport changes.  Should be
 * a singleton class that is only meant to be produced in one place
 * and consumed in another.
 */
class ViewPortTransferState {
// TODO: make sure this class is not moveable or copyable
public:
  ViewPortTransferState();
  ViewPortTransferState(const ViewPortTransferState&)=delete;
  ViewPortTransferState(const ViewPortTransferState&&)=delete;
  ViewPortTransferState& operator=(const ViewPortTransferState&)=delete;
  ViewPortTransferState& operator=(const ViewPortTransferState&&)=delete;
  void UpdateGridValues(FLOAT_T zoom,
                        const GridCoordinate &grid,
                        const GridPixelSize &max_image_size,
                        const ViewportPixelSize &screen_size);
  ViewPortCurrentState GetGridValues();
  /**
   * Find zoom index from a zoom value.
   *
   * @param zoom The zoom value to convert to an index.
   * @param min_zoom_index The minimum zoom value to return.
   * @param max_zoom_index The maximum zoom value to return.
   * @return The bounded zoom index corresponding to the zoom value.
   */
  static INT_T find_zoom_index_bounded(FLOAT_T zoom,
                                       INT_T min_zoom_index,
                                       INT_T max_zoom_index);
  /**
   * Find the maximum upper bound a zoom_index corresponds to.
   *
   * @param zoom_index An index into an array of zoomed textures or images.
   * @param the corresponding upper bound on the zoom value.
   * @return The upper bound of the zoom.
   */
  static FLOAT_T find_zoom_upper(INT_T zoom_index);
  /**
   * Find the leftmost visible coordinate.  This is fairly
   * conservative because it assumes max screen size and max zoom
   * based on zoom<-->zoom_index conversion.
   *
   * @param viewport_current_state The current state of the viewport.
   * @return The leftmost visible coordinate.
   */
  static FLOAT_T find_leftmost_visible(const ViewPortCurrentState& viewport_current_state);
  /**
   * Find the rightmost visible coordinate.  This is fairly
   * conservative because it assumes max screen size and max zoom
   * based on zoom<-->zoom_index conversion.
   *
   * @param viewport_current_state The current state of the viewport.
   * @return The rightmost visible coordinate.
   */
  static FLOAT_T find_rightmost_visible(const ViewPortCurrentState& viewport_current_state);
  /**
   * Find the topmost visible coordinate.  This is fairly conservative
   * because it assumes max screen size and max zoom based on
   * zoom<-->zoom_index conversion.
   *
   * @param viewport_current_state The current state of the viewport.
   * @return The topmost visible coordinate.
   */
  static FLOAT_T find_topmost_visible(const ViewPortCurrentState& viewport_current_state);
  /**
   * Find the bottommost visible coordinate.  This is fairly conservative
   * because it assumes max screen size and max zoom based on
   * zoom<-->zoom_index conversion.
   *
   * @param viewport_current_state The current state of the viewport.
   * @return The bottommost visible coordinate.
   */
  static FLOAT_T find_bottommost_visible(const ViewPortCurrentState& viewport_current_state);
  /**
   * Find whether an index is visible on the grid. This is fairly
   * conservative because it assumes max screen size and max zoom
   * based on zoom<-->zoom_index conversion.
   *
   * @param i the index along the width of the grid.
   * @param j the index along the height of the grid.
   * @param viewport_current_state The current state of the viewport.
   * @return Whether the index is visible.
   */
  static bool grid_index_visible(INT_T i, INT_T j,
                                 const ViewPortCurrentState& viewport_current_state);
private:
  static FLOAT_T _find_max_zoom(FLOAT_T zoom_index);
  FLOAT_T _zoom=NAN;
  GridCoordinate _grid;
  GridPixelSize _max_image_size;
  FLOAT_T _zoom_last=NAN;
  GridCoordinate _grid_last;
  bool _been_updated=false;
  ViewportPixelSize _screen_size;
  std::mutex _using_mutex;
};

#endif
