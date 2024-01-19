/**
 * Headers representing classes that encapsulating SDL functions.
 */
#ifndef SDL2_HPP
#define SDL2_HPP

#include "../common.hpp"
#include "../datatypes/coordinates.hpp"
// library headers
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

// this is defined as a macro so I can get easily get file/line information
#define PRINT_SDL_ERROR ERROR_LOCAL("SDL " << SDL_GetError())
#define PRINT_SDL_TTF_ERROR ERROR_LOCAL("SDL TTF " << TTF_GetError())

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
  void delay() const;
  /**
   * Update the various parameters ViewPort needs based on SDL inputs.
   * TODO: add docs when I update terminology
   *
   * @param current_speed_x The current x speed for the joystick,
   *                        generally a multiplier for an internal
   *                        constant.
   * @param current_speed_y The current y speed for the joystick,
   *                        generally a multiplier for an internal
   *                        constant.
   * @param current_speed_zoom Current speed things are being zoomed
   *                           for the joystick, generally a
   *                           multiplier for an interal constant.
   # @param zoom The current zoom.
   * @param zoom_speed The speed things are being zoomed when keyboard is pressed.
   * @param image_max_size The maximum pixel size of each image.
   * @param xgrid The x coordinate on the image grid.
   * @param ygrid The y coordinate on the image grid.
   * @param window_w The width of the window.
   * @param window_h The height of the window.
   * @return Whether to keep going or to quit.
   */
  bool do_input(FLOAT64& current_speed_x,
                FLOAT64& current_speed_y,
                FLOAT64& current_speed_zoom,
                FLOAT64& zoom,
                const FLOAT64& zoom_speed,
                const GridPixelSize& image_max_size,
                FLOAT64& xgrid,
                FLOAT64& ygrid,
                INT64& mouse_x,
                INT64& mouse_y,
                INT64& window_w,
                INT64& window_h);
  /**
   * Process raw joystick values.
   *
   * @param jaxis_original The raw joystick axis value.
   * @return the processed value, generally with deadzones for now.
   */
  Sint16 get_jaxis(Sint16 jaxis_original);
  /** @return Whether the app was successfully initialized. */
  bool successful() const;
  /** @return The SDL_Window for the app. */
  SDL_Window* window() const;
  /** @return The SDL_PixelFormat for the app. */
  SDL_PixelFormat* format() const;
private:
  SDL_Window* _window;
  SDL_PixelFormat* _format;
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
                     const BufferPixelSize& viewport_pixel_size);
  ~SDLDrawableSurface();
  // TODO: this should be made a friend and private
  /** @return The drawable screen surface. */
  SDL_Surface* screen_surface();
  /**
    * @param viewport_pixel_coordinate The coorindate to draw the rectangle at.
    * @param rect_pixel_size The size ofthe rectangle.
    * @param color The color of the rectangle.
    * @return If successful.
    */
  bool draw_rect(const BufferPixelCoordinate& viewport_pixel_coordinate,
                 const BufferPixelSize& rect_pixel_size,
                 PIXEL_RGBA color);
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
  /** @return Is the wrapped texture currently valid? */
  bool is_valid() const;
  /**
   * Create a new surface.
   *
   * Unlocks surface if locked.
   *
   * @param wpixel The width of the surface.
   * @param hpixel The height of the surface.
   * @return The new surface.
   */
  void create_surface(INT64 wpixel, INT64 hpixel);
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
   * Clears the surface.
   *
   * Assumes surface is locked.
   */
  void clear();
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
  /** @return The size in pixels of the stored texture. */
  BufferPixelSize texture_size_aligned () const;
  /** @return The size in pixels of the texture before alignment. */
  BufferPixelSize texture_size_visible() const;
  /**
   * Blit a texture to the surface.
   *
   * TODO: texture_wpixel and texture_hpixel will get objects specific to texture coordinates.
   *
   * @param drawable_surface A pointer to the the SDLDrawableSurface to blit to.
   * @param texture_size The visible size of the texture to blit.
   * @param viewport_pixel_coordinate The coorindate to blit to on the viewport.
   * @param image_pixel_size_viewport The size of the texture on the viewport.
   */
  void blit_texture(SDLDrawableSurface* drawable_surface,
                    const BufferPixelSize& texture_size,
                    const BufferPixelCoordinate& viewport_pixel_coordinate,
                    const BufferPixelSize& image_pixel_size_viewport);
private:
  SDL_Surface* _display_texture=nullptr;
  INT64 _wpixel_visible;
  INT64 _hpixel_visible;
};

/**
 * An object to hold an SDL texture with text.
 */
class SDLFontTextureWrapper {
public:
  SDLFontTextureWrapper();
  ~SDLFontTextureWrapper();
  /** Update the text. */
  void update_text(const std::string& text);
  /**
   * Draw the text.
   *
   * @param drawable_surface The screen surface to draw on.
   * @param The x pixel coordinate of the upper left corner.
   * @param The y pixel coordinate of the upper left corner.
   */
  void draw_text(SDLDrawableSurface* drawable_surface, INT64 xpixel, INT64 ypixel);
  /** Get the surface that displays the text. */
  SDL_Surface* surface();
private:
  SDL_Surface* _overlay_message_surface=nullptr;
  TTF_Font* _sdl_current_font=nullptr;
  SDL_Color _sdl_font_color;
};


# endif
