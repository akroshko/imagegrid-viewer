/**
 * Headers representing classes that encapsulating SDL functions.
 */
#ifndef SDL_HPP
#define SDL_HPP

#include "debug.hpp"
#include "error.hpp"
#include "defaults.hpp"
#include "coordinates.hpp"
// library headers
#include <SDL2/SDL.h>

/**
 * An object to contain all the SDL boilerplate and implementation.
 */
class SDLApp {
public:
  SDLApp();
  ~SDLApp();
  /**
   * Call an SDL delay function for the end of the main loop.
   */
  void delay();
  /**
   * Update the various parameters ViewPort needs based on SDL inputs.
   *
   * @param current_speed_x
   *
   * @param current_speed_y
   *
   * @param current_speed_zoom
   *
   * @param zoom_speed
   *
   * @param image_max_size
   *
   * @param xgrid
   *
   * @param ygrid
   *
   */
  bool do_input(FLOAT_T &current_speed_x, FLOAT_T &current_speed_y,
                FLOAT_T &current_speed_zoom, FLOAT_T &zoom_speed, FLOAT_T &zoom,
                const GridPixelSize &image_max_size, FLOAT_T &xgrid, FLOAT_T &ygrid);
  /**
   * Blank out any textures on the viewport.
   *
   * @param viewport_pixel_size The size of the viewport.
   */
  void blank_viewport(const ViewportPixelSize &viewport_pixel_size);
  /**
   * Check whether app was successful
   */
  bool successful();
  SDL_Window *window;
  SDL_PixelFormat *format;
  SDL_Surface* screen_surface;
private:
  bool _successful=true;
  bool joystick_enabled;
  SDL_Renderer *renderer;
  SDL_Joystick* game_controller;
};



# endif
