/**
 * Header representing an observer pattern for finding current state of the viewport.
 */
#ifndef VIEWPORT_CURRENT_STATE_HPP
#define VIEWPORT_CURRENT_STATE_HPP

// local headers
#include "common.hpp"
#include "coordinates.hpp"
// C++ headers
#include <mutex>
// C headers
#include <cmath>

class ViewPortCurrentState {
public:
  ViewPortCurrentState(GridCoordinate current_grid_coordinate,
                       GridPixelSize image_max_size,
                       FLOAT64 zoom,
                       ViewportPixelSize screen_size,
                       ViewportPixelCoordinate pointer_pixel_coordinate,
                       ViewportPixelCoordinate center_pixel_coordinate);
  // these are enabled because this class is used for data transfer
  // and created and returned from a function
  ViewPortCurrentState(ViewPortCurrentState&)=default;
  ViewPortCurrentState(ViewPortCurrentState&&)=default;
  ViewPortCurrentState& operator=(ViewPortCurrentState&)=delete;
  ViewPortCurrentState& operator=(ViewPortCurrentState&&)=delete;
  GridCoordinate current_grid_coordinate() const;
  /** @return The maximum image size. */
  GridPixelSize image_max_size() const;
  /** @return The current zoom. */
  FLOAT64 zoom() const;
  /** @return The current screen size. */
  ViewportPixelSize screen_size() const;
  ViewportPixelCoordinate pointer() const;
  ViewportPixelCoordinate center() const;
private:
  friend class ViewPortTransferState;
  GridCoordinate _current_grid_coordinate;
  GridPixelSize _image_max_size;
  FLOAT64 _zoom;
  ViewportPixelSize _screen_size;
  ViewportPixelCoordinate _pointer_pixel_coordinate;
  ViewportPixelCoordinate _center_pixel_coordinate;
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
  void UpdateGridValues(FLOAT64 zoom,
                        const GridCoordinate& grid,
                        const GridPixelSize& max_image_size,
                        const ViewportPixelSize& screen_size,
                        const ViewportPixelCoordinate& pointer_pixel_coordinate,
                        const ViewportPixelCoordinate& center_pixel_coordinate);
  ViewPortCurrentState GetGridValues();
  /**
   * Find zoom index from a zoom value.
   *
   * @param zoom The zoom value to convert to an index.
   * @param min_zoom_index The minimum zoom value to return.
   * @param max_zoom_index The maximum zoom value to return.
   * @return The bounded zoom index corresponding to the zoom value.
   */
  static INT64 find_zoom_index_bounded(FLOAT64 zoom,
                                       INT64 min_zoom_index,
                                       INT64 max_zoom_index);
  /**
   * Find the maximum upper bound a zoom_index corresponds to.
   *
   * @param zoom_index An index into an array of zoomed textures or images.
   * @param the corresponding upper bound on the zoom value.
   * @return The upper bound of the zoom.
   */
  static FLOAT64 find_zoom_upper(INT64 zoom_index);
  /**
   * Find the leftmost visible coordinate.  This is fairly
   * conservative because it assumes max screen size and max zoom
   * based on zoom<-->zoom_index conversion.
   *
   * @param viewport_current_state The current state of the viewport.
   * @return The leftmost visible coordinate.
   */
  static FLOAT64 find_leftmost_visible(const ViewPortCurrentState& viewport_current_state);
  /**
   * Find the rightmost visible coordinate.  This is fairly
   * conservative because it assumes max screen size and max zoom
   * based on zoom<-->zoom_index conversion.
   *
   * @param viewport_current_state The current state of the viewport.
   * @return The rightmost visible coordinate.
   */
  static FLOAT64 find_rightmost_visible(const ViewPortCurrentState& viewport_current_state);
  /**
   * Find the topmost visible coordinate.  This is fairly conservative
   * because it assumes max screen size and max zoom based on
   * zoom<-->zoom_index conversion.
   *
   * @param viewport_current_state The current state of the viewport.
   * @return The topmost visible coordinate.
   */
  static FLOAT64 find_topmost_visible(const ViewPortCurrentState& viewport_current_state);
  /**
   * Find the bottommost visible coordinate.  This is fairly conservative
   * because it assumes max screen size and max zoom based on
   * zoom<-->zoom_index conversion.
   *
   * @param viewport_current_state The current state of the viewport.
   * @return The bottommost visible coordinate.
   */
  static FLOAT64 find_bottommost_visible(const ViewPortCurrentState& viewport_current_state);
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
  static bool grid_index_visible(INT64 i, INT64 j,
                                 const ViewPortCurrentState& viewport_current_state);
private:
  static FLOAT64 _find_max_zoom(FLOAT64 zoom_index);
  FLOAT64 _zoom=NAN;
  GridCoordinate _grid;
  GridPixelSize _image_max_size;
  GridCoordinate _grid_last;
  ViewportPixelSize _screen_size;
  ViewportPixelCoordinate _pointer_pixel_coordinate;
  ViewportPixelCoordinate _center_pixel_coordinate;
  std::mutex _using_mutex;
};

#endif
