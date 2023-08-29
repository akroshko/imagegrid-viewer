/**
 * Headers representing classes that encapsulating SDL functions.
 */
#ifndef SDL_HPP
#define SDL_HPP

#include "../debug.hpp"
#include "../error.hpp"
#include "../defaults.hpp"
#include "../coordinates.hpp"
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
   * @param window_w
   *
   * @param window_h
   *
   * @return Whether to keep going or to quit.
   */
  bool do_input(FLOAT_T &current_speed_x, FLOAT_T &current_speed_y,
                FLOAT_T &current_speed_zoom, FLOAT_T &zoom_speed, FLOAT_T &zoom,
                const GridPixelSize &image_max_size, FLOAT_T &xgrid, FLOAT_T &ygrid,
                INT_T &window_w, INT_T &window_h);
  /**
   *
   */
  Sint16 get_jaxis(Sint16 jaxis_original);
  /**
   * Check whether app was successful
   */
  bool successful() const;
  SDL_Window* window;
  SDL_PixelFormat* format;
  SDL_Surface* screen_surface;
private:
  bool _successful=true;
  bool _joystick_enabled;
  SDL_Renderer* _renderer;
  SDL_Joystick* _game_controller;
};

/**
 * An object that gives a drawable surface for the lifetime of the
 * object.
 */
class SDLDrawableSurface {
public:
  SDLDrawableSurface(SDLApp* const sdl_app,
                     const ViewportPixelSize &viewport_pixel_size);
  ~SDLDrawableSurface();
  /** @return The drawable screen surface. */
  SDL_Surface* screen_surface();
private:
  SDL_Surface* _screen_surface;
  SDLApp* _sdl_app;
};

# endif
