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
#include "gridsetup.hpp"
#include "gridclasses.hpp"
#include "coordinates.hpp"
// C compatible headers
#include "c_compatible/sdl.hpp"
// C++ headers
#include <iostream>
#include <mutex>
#include <string>
#include <vector>

 /** Class that stores an item that ready to be blit to the screen. */
class BlitItem {
public:
  BlitItem()=delete;
  BlitItem(TextureGridSquareZoomLevel* square, INT_T count, const ViewportPixelCoordinate &viewport_pixel_coordinate, const ViewportPixelSize &grid_image_size_zoomed);
  ~BlitItem();
  // BlitItem(const BlitItem&)=delete;
  BlitItem(const BlitItem&)=default;
  BlitItem(const BlitItem&&)=delete;
  BlitItem& operator=(const BlitItem&)=delete;
  BlitItem& operator=(const BlitItem&&)=delete;
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
  ViewportPixelCoordinate viewport_pixel_coordinate;
  /**
   * The width on the viewport of this texture.
   */
  ViewportPixelSize image_pixel_size_viewport;
private:
  /**
   * Just counting the number of items to be blit, mostly for
   * debugging.
   */
  int blit_index;
};

/**
 * Class that represents a viewport.
 */
class ViewPort {
public:
  ViewPort()=delete;
  ViewPort(std::shared_ptr<ViewPortCurrentState> viewport_current_state_texturegrid_update,
           std::shared_ptr<ViewPortCurrentState> viewport_current_state_imagegrid_update);
  ViewPort(const ViewPort&)=delete;
  ViewPort(const ViewPort&&)=delete;
  ViewPort& operator=(const ViewPort&)=delete;
  ViewPort& operator=(const ViewPort&&)=delete;
  void find_viewport_blit(TextureGrid* texture_grid,  SDLApp* sdl_app);
  /** update the values in this class with current keyboard/joystick/etc. input */
  bool do_input(SDLApp* sdl_app);
  /**
   * Update the information and observors for the current state of the viewport.
   */
  void update_viewport_info(FLOAT_T xgrid, FLOAT_T ygrid);
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
  void set_image_max_size(const GridPixelSize &image_max_size);
private:
  GridPixelSize _image_max_size;
  /** The current size of the window in pixels. */
  ViewportPixelSize viewport_pixel_size;
  /** the grid coordinates of the center of the viewport */
  GridCoordinate viewport_grid;
  /** the zoom level, 1.0 indicates all pixels are 1:1, 0.5 indicates zoomed out by a factor of 2 */
  FLOAT_T zoom=INITIAL_ZOOM;
  /** a zoom speed per SDL frame */
  FLOAT_T zoom_speed=INITIAL_ZOOM_SPEED;
  /** the current x speed of movement per SDL frame */
  FLOAT_T current_speed_x=INITIAL_X_Y_SPEED;
  /** the current y speed of movement per SDL frame */
  FLOAT_T current_speed_y=INITIAL_X_Y_SPEED;
  /** the current speed of zoom per SDL frame */
  FLOAT_T current_speed_zoom=INITIAL_CURRENT_ZOOM_SPEED;
  /** object for transfering the state of the viewport in a threadsafe manner */
  std::shared_ptr<ViewPortCurrentState> viewport_current_state_texturegrid_update;
  std::shared_ptr<ViewPortCurrentState> viewport_current_state_imagegrid_update;
};

#endif
