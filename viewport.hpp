/**
 * Header representing the viewport.
 */
#ifndef VIEWPORT_HPP
#define VIEWPORT_HPP

// local headers
#include "debug.hpp"
#include "error.hpp"
#include "types.hpp"
#include "defaults.hpp"
#include "sdl.hpp"
#include "gridsetup.hpp"
#include "gridclasses.hpp"
#include "coordinates.hpp"
// C++ headers
#include <vector>
#include <iostream>
#include <string>
#include <mutex>

 /** Class that stores an item that ready to be blit to the screen. */
class BlitItem {
public:
  BlitItem(TextureGridSquareZoomLevel* square, INT_T count, ViewportPixelCoordinate viewport_pixel_coordinate, ViewportPixelSize grid_image_size_zoomed);
  ~BlitItem();
  /**
   * Method called to do the actual blitting.
   */
  void blit_this(SDLApp* sdl_app);
  /**
   * The square that will be blit to the screen.
   */
  TextureGridSquareZoomLevel* blit_square=nullptr;
  /**
   * The pixel location on the viewport that this texture is being
   * blit to.
   */
  ViewportPixelCoordinate* viewport_pixel_coordinate=nullptr;
  /**
   * The width on the viewport of this texture.
   */
  ViewportPixelSize* image_pixel_size_viewport=nullptr;
private:
  /**
   * Just counting the number of items to be blit, mostly for
   * debugging.
   */
  int blit_index;
};


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
  ~ViewPortCurrentState();
  ViewPortCurrentState(const ViewPortCurrentState&) = delete;
  ViewPortCurrentState & operator=(const ViewPortCurrentState&) = delete;
  void UpdateGridValues(FLOAT_T zoom, GridCoordinate *grid);
  bool GetGridValues(FLOAT_T &zoom, GridCoordinate *&grid);
private:
  FLOAT_T zoom=NAN;
  GridCoordinate* grid=nullptr;
  FLOAT_T zoom_last=NAN;
  GridCoordinate* grid_last=nullptr;
  bool been_updated=false;
  std::mutex using_mutex;
};

/**
 * Class that represents a viewport.
 */
class ViewPort {
public:
  ViewPort(ViewPortCurrentState *viewport_current_state);
  ~ViewPort();
  void find_viewport_blit(TextureGrid* texture_grid,  SDLApp* sdl_app);
  /** update the values in this class with current keyboard/joystick/etc. input */
  bool do_input(SDLApp* sdl_app);
  /**
   * Clear old textures from the viewport.
   *
   * @param sdl_app
   */
  void blank_viewport(SDLApp* sdl_app);
  /**
   *
   *
   * @param zoom
   */
  INT_T find_zoom_index(FLOAT_T zoom);
  /**
   * Adjust initial location.
   *
   * @param grid_setup
   */
  void adjust_initial_location(GridSetup *grid_setup);
  /**
   * Max size of images.
   */
  void set_image_max_size(GridPixelSize * image_max_size);
private:
  GridPixelSize *_image_max_size=nullptr;
  /** The current size of the window in pixels. */
  ViewportPixelSize *viewport_pixel_size=nullptr;
  /** the grid coordinates of the center of the viewport */
  GridCoordinate *viewport_grid=nullptr;
  /** the zoom level, 1.0 indicates all pixels are 1:1, 0.5 indicates zoomed out by a factor of 2 */
  FLOAT_T zoom=INITIAL_ZOOM;
  /** a zoom speed per SDL frame */
  FLOAT_T zoom_speed=INITIAL_ZOOM_SPEED;
  /** the current x speed of movement per SDL frame */
  FLOAT_T current_speed_x = INITIAL_X_Y_SPEED;
  /** the current y speed of movement per SDL frame */
  FLOAT_T current_speed_y = INITIAL_X_Y_SPEED;
  /** the current speed of zoom per SDL frame */
  FLOAT_T current_speed_zoom = INITIAL_CURRENT_ZOOM_SPEED;
  /** object for transfering the state of the viewport in a threadsafe manner */
  ViewPortCurrentState *viewport_current_state=nullptr;
  /** Stores the next items to be blit to the viewport. */
  std::vector<BlitItem> blititems;
};

#endif
