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
#include "datatypes/coordinates.hpp"
#include  "viewport_current_state.hpp"
// C compatible headers
#include "c_sdl2/sdl2.hpp"
// C++ headers
#include <memory>

/**
 * Method called to do the actual blitting.
 *
 * @param screen_surface The screen surface to blit to.
 * @param blit_square
 * @param count
 * @param l_viewport_pixel_coordinate
 * @param grid_image_size_zoomed
 */
void blit_this(SDLDrawableSurface* screen_surface,
               TextureGridSquareZoomLevel* const blit_square,
               const BufferPixelCoordinate& l_viewport_pixel_coordinate,
               const BufferPixelSize& grid_image_size_zoomed);

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
   * @param grid_coordinate The grid coordinate to update to.
   */
  void update_viewport_info(const GridCoordinate& grid_coordinate);
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
  BufferPixelSize _viewport_pixel_size;
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
   * imagegrid_update thead in a threadsafe manner */
  std::shared_ptr<ViewPortTransferState> _viewport_current_state_imagegrid_update;
};

#endif
