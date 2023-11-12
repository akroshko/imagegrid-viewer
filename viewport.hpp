/**
 * Header representing the viewport.
 */
#ifndef VIEWPORT_HPP
#define VIEWPORT_HPP

// local headers
#include "common.hpp"
#include "imagegrid/gridsetup.hpp"
#include "texture_overlay.hpp"
#include "texturegrid.hpp"
#include "coordinates.hpp"
#include  "viewport_current_state.hpp"
// C compatible headers
#include "c_sdl/sdl.hpp"
// C++ headers
#include <memory>

 /** Class that stores an item that ready to be blit to the screen. */
class BlitItem {
public:
  BlitItem()=delete;
  BlitItem(TextureGridSquareZoomLevel* square, INT64 count,
           const ViewportPixelCoordinate& viewport_pixel_coordinate,
           const ViewportPixelSize& grid_image_size_zoomed);
  ~BlitItem()=default;
  BlitItem(const BlitItem&)=default;
  BlitItem(const BlitItem&&)=delete;
  BlitItem& operator=(const BlitItem&)=delete;
  BlitItem& operator=(const BlitItem&&)=delete;
  /**
   * Method called to do the actual blitting.
   *
   * @param screen_surface The screen surface to blit to.
   */
  void blit_this(SDLDrawableSurface* screen_surface);
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
  INT64 _blit_index;
};

/**
 * Class that represents a viewport.
 */
class ViewPort {
public:
  ViewPort()=delete;
  ViewPort(std::shared_ptr<ViewPortTransferState> viewport_current_state_texturegrid_update,
           std::shared_ptr<ViewPortTransferState> viewport_current_state_imagegrid_update);
  ViewPort(const ViewPort&)=delete;
  ViewPort(const ViewPort&&)=delete;
  ViewPort& operator=(const ViewPort&)=delete;
  ViewPort& operator=(const ViewPort&&)=delete;
  void find_viewport_blit(TextureGrid* texture_grid,
                          TextureOverlay* const texture_overlay,
                          SDLApp* sdl_app);
  /**
   * Update the values in this class with current keyboard/joystick/etc. input.
   *
   * @param sdl_app The SDL app object.
   * @return
   */
  bool do_input(SDLApp* sdl_app);
  /**
   * Update the information and observors for the current state of the viewport.
   *
   * @param xgrid The x grid coordinate to update to.
   * @param ygrid The y grid coordinate to update to.
   */
  void update_viewport_info(FLOAT64 xgrid, FLOAT64 ygrid);
  /**
   * Adjust initial location.
   *
   * @param grid_setup The object holding the data on the images in
   *                   the grid, including the filenames and grid
   *                   size.
   */
  void adjust_initial_location(const GridSetup* grid_setup);
  /**
   * Set max size of images.
   *
   * @param image_max_size The maximum size of images.
   */
  void set_image_max_size(const GridPixelSize& image_max_size);
private:
  GridPixelSize _image_max_size;
  /** The current size of the window in pixels. */
  ViewportPixelSize _viewport_pixel_size;
  /** the grid coordinates of the center of the viewport */
  GridCoordinate _viewport_grid;
  /** the zoom out value, 1.0 indicates all pixels are 1:1, 0.5 indicates zoomed out by a factor of 2 */
  FLOAT64 _zoom=INITIAL_ZOOM;
  /** a zoom speed per SDL frame */
  FLOAT64 _zoom_speed=INITIAL_ZOOM_SPEED;
  /** the current x speed of movement per SDL frame */
  FLOAT64 _current_speed_x=INITIAL_X_Y_SPEED;
  /** the current y speed of movement per SDL frame */
  FLOAT64 _current_speed_y=INITIAL_X_Y_SPEED;
  /** the current speed of zoom per SDL frame */
  FLOAT64 _current_speed_zoom=INITIAL_CURRENT_ZOOM_SPEED;
  /** The window width in pixels */
  INT64 _current_window_w=INITIAL_SCREEN_WIDTH;
  /** The window height in pixels */
  INT64 _current_window_h=INITIAL_SCREEN_HEIGHT;
  /** The current mouse cursor x coordinate */
  INT64 _current_mouse_xpixel=0;
  /** The current mouse cursor y coordinate */
  INT64 _current_mouse_ypixel=0;
  /** object for transfering the state of the viewport to the
   * texturegrid_update thread in a threadsafe manner */
  std::shared_ptr<ViewPortTransferState> _viewport_current_state_texturegrid_update;
  /** object for transfering the state of the viewport to the
   * imagegrid_updte thead in a threadsafe manner */
  std::shared_ptr<ViewPortTransferState> _viewport_current_state_imagegrid_update;
};

#endif
