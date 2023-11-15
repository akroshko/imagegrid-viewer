/**
 * Implementaton of classes that encapsulate SDL functions.
 */
// local headers
#include "../common.hpp"
#include "../coordinates.hpp"
#include "sdl2.hpp"
// C++ headers
#include <iostream>
// C headers
#include <cstddef>
// C library headers
#include <SDL2/SDL.h>

SDLApp::SDLApp() {
  // auto rendererFlags=SDL_RENDERER_ACCELERATED;
  int windowFlags=SDL_WINDOW_RESIZABLE;
  this->_format=SDL_AllocFormat(SDL_PIXELFORMAT_RGBA32);
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
    ERROR("Couldn't initialize SDL:" << SDL_GetError());
    this->_successful=false;
  } else {
    this->_window=SDL_CreateWindow("Image Grid Viewer",
                                   SDL_WINDOWPOS_UNDEFINED,
                                   SDL_WINDOWPOS_UNDEFINED,
                                   INITIAL_SCREEN_WIDTH,
                                   INITIAL_SCREEN_HEIGHT,
                                   windowFlags);
    if (!this->_window) {
      ERROR("Failed to open %d " << INITIAL_SCREEN_WIDTH
                                 << "x" << INITIAL_SCREEN_HEIGHT
                                 << " window: " << SDL_GetError());
      this->_successful=false;
    } else {
      if (SDL_NumJoysticks() < 1) {
        this->_joystick_enabled=false;
      } else {
        this->_joystick_enabled=true;
        this->_game_controller=SDL_JoystickOpen(0);
      }
    }
  }
}

SDLApp::~SDLApp() {
  if (this->_window) {
    SDL_DestroyWindow(this->_window);
    this->_window=nullptr;
  }
  SDL_Quit();
}

Sint16 SDLApp::get_jaxis(Sint16 jaxis_original) {
  if (jaxis_original > JOY_DEADZONE) {
    return jaxis_original-JOY_DEADZONE;
  } else if (jaxis_original < -JOY_DEADZONE) {
    return jaxis_original+JOY_DEADZONE;
  } else {
    return 0;
  }
}

bool SDLApp::do_input(FLOAT64& current_speed_x, FLOAT64& current_speed_y,
                      FLOAT64& current_speed_zoom,
                      FLOAT64& zoom, const FLOAT64& zoom_speed,
                      const GridPixelSize& image_max_size,
                      FLOAT64& xgrid, FLOAT64& ygrid,
                      INT64& mouse_x, INT64& mouse_y,
                      INT64& window_w, INT64& window_h) {
  SDL_Event e;
  auto keep_going=true;
  while(SDL_PollEvent(&e)) {
    auto pixel_size=(100.0/image_max_size.wpixel()/zoom);
    if (e.type == SDL_QUIT) {
      return false;
    } else if (e.type == SDL_WINDOWEVENT) {
      switch (e.window.event) {
      case SDL_WINDOWEVENT_RESIZED:
        window_w=e.window.data1;
        window_h=e.window.data2;
        break;
      case SDL_WINDOWEVENT_SIZE_CHANGED:
        window_w=e.window.data1;
        window_h=e.window.data2;
        break;
      case SDL_WINDOWEVENT_MAXIMIZED:
        int window_w_get;
        int window_h_get;
        SDL_GetWindowSize(this->_window, &window_w_get, &window_h_get);
        window_w=window_w_get;
        window_h=window_h_get;
        break;
      default:
        break;
      }
    } else if (e.type == SDL_JOYAXISMOTION) {
      // TODO: deal with dead zones
      if (e.jaxis.which == 0) {
        auto jaxis_adjusted=this->get_jaxis(e.jaxis.value);
        DEBUGIO("jaxis 0: " << e.jaxis.axis << " ++ " << e.jaxis.value << " ++ " << jaxis_adjusted);
        if (e.jaxis.axis == 0) {
          current_speed_x=(jaxis_adjusted/JOY_MAX)*(JOY_BASE_MOVE*pixel_size);
        } else if (e.jaxis.axis == 1) {
          current_speed_y=(jaxis_adjusted/JOY_MAX)*(JOY_BASE_MOVE*pixel_size);
        } else if (e.jaxis.axis == 4) {
          current_speed_zoom=JOY_BASE_ZOOM*(jaxis_adjusted/JOY_MAX);
        }
      }
    } else if (e.type == SDL_MOUSEMOTION) {
      // TODO: probably need an e.which for multiple mice
      mouse_x=e.motion.x;
      mouse_y=e.motion.y;
    } else if (e.type == SDL_KEYDOWN) {
      switch(e.key.keysym.sym) {
      case SDLK_LEFT:
        // move 5% of a screen left
        xgrid-=(KEY_PRESS_MOVE*pixel_size);
        break;
      case SDLK_RIGHT:
        // move 5% of a screen right
        xgrid+=(KEY_PRESS_MOVE*pixel_size);
        break;
      case SDLK_UP:
        // move 5% of a screen up
        ygrid-=(KEY_PRESS_MOVE*pixel_size);
        break;
      case SDLK_DOWN:
        // move 5% of a screen down
        ygrid+=(KEY_PRESS_MOVE*pixel_size);
        break;
      case SDLK_EQUALS:
      case SDLK_PLUS:
        zoom*=(1.2*zoom_speed);
        break;
      case SDLK_MINUS:
        zoom/=(1.2*zoom_speed);
        break;
      case SDLK_q:
        keep_going=false;
        break;
      default:
        break;
      }
    }
  }
  // speed
  xgrid+=(current_speed_x*JOY_BASE_MOVE);
  ygrid+=(current_speed_y*JOY_BASE_MOVE);
  zoom=(1.0-current_speed_zoom)*zoom;
  return keep_going;
}

void SDLApp::delay() const {
  SDL_Delay(SDL_DELAY);
}

bool SDLApp::successful() const {
  return this->_successful;
}

SDL_Window* SDLApp::window() const {
  return this->_window;
}

SDL_PixelFormat* SDLApp::format() const {
  return this->_format;
}

SDLDrawableSurface::SDLDrawableSurface(SDLApp* const sdl_app,
                                       const ViewportPixelSize& viewport_pixel_size) {
  this->_sdl_app=sdl_app;
  if ((this->_screen_surface=SDL_GetWindowSurface(sdl_app->window()))) {
    SDL_Rect screen_rect;
    screen_rect.x=0;
    screen_rect.y=0;
    screen_rect.w=viewport_pixel_size.wpixel();
    screen_rect.h=viewport_pixel_size.hpixel();
    if (SDL_FillRect(this->_screen_surface,
                     &screen_rect,
                     SDL_MapRGBA(sdl_app->format(),0,0,0,0)) < 0) {
      PRINT_SDL_ERROR;
    }
  } else {
    PRINT_SDL_ERROR;
  }
}

SDLDrawableSurface::~SDLDrawableSurface() {
  if (SDL_UpdateWindowSurface(this->_sdl_app->window()) != 0) {
    PRINT_SDL_ERROR;
  }
  SDL_FreeSurface(this->_screen_surface);
  this->_screen_surface=nullptr;
}

SDL_Surface* SDLDrawableSurface::screen_surface() {
  return this->_screen_surface;
}

bool SDLDisplayTextureWrapper::is_valid () const {
  return (this->_display_texture);
}

SDLDisplayTextureWrapper::~SDLDisplayTextureWrapper () {
  SDL_FreeSurface(this->_display_texture);
  this->_display_texture=nullptr;
}

void SDLDisplayTextureWrapper::create_surface(INT64 wpixel, INT64 hpixel) {
  if (this->_display_texture) {
    this->unlock_surface();
  }
  auto wpixel_aligned=wpixel + (TEXTURE_ALIGNMENT - (wpixel % TEXTURE_ALIGNMENT));
  auto hpixel_aligned=hpixel;
  if (!(this->_display_texture=SDL_CreateRGBSurfaceWithFormat(0,wpixel_aligned,hpixel_aligned,32,SDL_PIXELFORMAT_RGBA32))) {
    PRINT_SDL_ERROR;
  } else {
    this->_wpixel_visible=wpixel;
    this->_hpixel_visible=hpixel;
  }
}

void SDLDisplayTextureWrapper::unload_surface() {
  SDL_FreeSurface(this->_display_texture);
  this->_display_texture=nullptr;
}

void* SDLDisplayTextureWrapper::pixels () {
  return this->_display_texture->pixels;
}

void SDLDisplayTextureWrapper::clear () {
  if (this->_display_texture) {
    if (SDL_FillRect(this->_display_texture,NULL,0) < 0) {
      PRINT_SDL_ERROR;
    }
  }
}

bool SDLDisplayTextureWrapper::lock_surface () {
  if (this->_display_texture) {
    if (SDL_LockSurface(this->_display_texture) < 0) {
      PRINT_SDL_ERROR;
      return false;
    } else {
      return true;
    }
  } else {
    return false;
  }
}

void SDLDisplayTextureWrapper::unlock_surface () {
  if (this->_display_texture) {
    SDL_UnlockSurface(this->_display_texture);
  }
}

INT64 SDLDisplayTextureWrapper::texture_wpixel_aligned () const {
  return this->_display_texture->w;
}

INT64 SDLDisplayTextureWrapper::texture_hpixel_aligned () const {
  return this->_display_texture->h;
}

INT64 SDLDisplayTextureWrapper::texture_wpixel_visible() const {
  return this->_wpixel_visible;
}

INT64 SDLDisplayTextureWrapper::texture_hpixel_visible() const {
  return this->_hpixel_visible;
}

void SDLDisplayTextureWrapper::blit_texture(SDLDrawableSurface* drawable_surface,
                                            INT64 texture_wpixel,
                                            INT64 texture_hpixel,
                                            const ViewportPixelCoordinate& viewport_pixel_coordinate,
                                            const ViewportPixelSize& image_pixel_size_viewport) {
  SDL_Rect texture_rect;
  texture_rect.x=0;
  texture_rect.y=0;
  texture_rect.w=texture_wpixel;
  texture_rect.h=texture_hpixel;
  SDL_Rect scaled_rect;
  scaled_rect.x=viewport_pixel_coordinate.xpixel();
  scaled_rect.y=viewport_pixel_coordinate.ypixel();
  scaled_rect.w=image_pixel_size_viewport.wpixel();
  scaled_rect.h=image_pixel_size_viewport.hpixel();
  if (SDL_BlitScaled(this->_display_texture,
                     &texture_rect,
                     drawable_surface->screen_surface(),
                     &scaled_rect) < 0) {
    PRINT_SDL_ERROR;
  }
}

SDLFontTextureWrapper::SDLFontTextureWrapper () {
  if (TTF_Init() < 0) {
    PRINT_SDL_TTF_ERROR;
  }
  if (!(this->_sdl_current_font = TTF_OpenFont(OVERLAY_FONT_PATH, 24))) {
    // does not appear to use TTF_ERROR
    ERROR("TTF_OpenFont");
  }
  this->_sdl_font_color = {255, 255, 255, 0};
}

SDLFontTextureWrapper::~SDLFontTextureWrapper () {
  SDL_FreeSurface(this->_overlay_message_surface);
  this->_overlay_message_surface=nullptr;
  TTF_CloseFont(this->_sdl_current_font);
  this->_sdl_current_font=nullptr;
  TTF_Quit();
}

void SDLFontTextureWrapper::update_text(const std::string& text) {
  SDL_FreeSurface(this->_overlay_message_surface);
  this->_overlay_message_surface=nullptr;
  this->_overlay_message_surface=TTF_RenderText_Solid(this->_sdl_current_font,
                                                      text.c_str(),
                                                      this->_sdl_font_color);
  if (!this->_overlay_message_surface) {
    // does not appear to use TTF_ERROR
    ERROR("TTF_RenderText_Solid");
  }
}

void SDLFontTextureWrapper::draw_text(SDLDrawableSurface* drawable_surface,
                                      INT64 xpixel, INT64 ypixel) {
  if (this->_overlay_message_surface) {
    SDL_Rect message_rect;
    message_rect.x = xpixel;
    message_rect.y = ypixel;
    message_rect.w =this->_overlay_message_surface->w;
    message_rect.h =this->_overlay_message_surface->h;
    SDL_BlitScaled(this->_overlay_message_surface,
                   NULL,
                   drawable_surface->screen_surface(),
                   &message_rect);
  }
}

SDL_Surface* SDLFontTextureWrapper::get_surface() {
  return this->_overlay_message_surface;
}
