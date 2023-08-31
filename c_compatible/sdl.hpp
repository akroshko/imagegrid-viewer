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
   * Process raw joystick values.
   *
   * @param jaxis_original the raw joystick axis value
   *
   * @return the processed value, generally with deadzones for now.
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
 * An object that wraps a drawable surface for the lifetime of the
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

/**
 * An object that wraps SDL_Surface as a texture.
 */
class SDLDisplayTextureWrapper {
public:
  SDLDisplayTextureWrapper()=default;
  ~SDLDisplayTextureWrapper();
  /**
   * Is the texture currently valid?
   *
   * @return Is the texture valid?
   */
  bool is_valid();
  /**
   * Create a new surface.
   *
   * Unlocks surface if locked.
   *
   * @param wpixel The width of the surface.
   *
   * @param hpixel The height of the surface.
   *
   * @return The new surface.
   */
  void create_surface(INT_T wpixel, INT_T hpixel);
  /**
   * Unload the surface if it is loaded.
   */
  void unload_surface();
  /**
   * Get the raw pixels array of the surface.
   *
   * @return A pointer to the raw pixel array.
   */
  void* pixels();
  /**
   * Lock the surface.
   *
   * @return If the surface was successfully locked.
   */
  bool lock_surface();
  /**
   * Unlock the surface.
   */
  void unlock_surface();
  /**
   * Blit a texture to the surface.
   *
   * @param screen_surface The screen surface to blit to.
   *
   * @param viewport_pixel_coordinate The coorindate to blit to on the viewport.
   *
   * @param image_pixel_size_viewport The size of the texture on the viewport.
   */
  void blit_texture(SDL_Surface* screen_surface,
                    ViewportPixelCoordinate &viewport_pixel_coordinate,
                    ViewportPixelSize &image_pixel_size_viewport);
private:
  SDL_Surface* _display_texture=nullptr;
};

# endif
