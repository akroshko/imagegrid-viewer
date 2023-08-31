/**
 * Implementaton of classes that encapsulate SDL functions.
 */
// local headers
#include "../debug.hpp"
#include "../error.hpp"
#include "../defaults.hpp"
#include "../coordinates.hpp"
#include "sdl.hpp"
// C library headers
#include <SDL2/SDL.h>

SDLApp::SDLApp() {
  // auto rendererFlags=SDL_RENDERER_ACCELERATED;
  int windowFlags=SDL_WINDOW_RESIZABLE;
  this->format=SDL_AllocFormat(SDL_PIXELFORMAT_RGB24);
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
    ERROR("Couldn't initialize SDL:" << SDL_GetError());
    this->_successful=false;
  } else {
    this->window=SDL_CreateWindow("Image Grid Viewer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, INITIAL_SCREEN_WIDTH, INITIAL_SCREEN_HEIGHT, windowFlags);
    if (!this->window) {
      ERROR("Failed to open %d " << INITIAL_SCREEN_WIDTH << "x" << INITIAL_SCREEN_HEIGHT << " window: " << SDL_GetError());
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
  SDL_FreeSurface(this->screen_surface);
  SDL_DestroyWindow(this->window);
  window=nullptr;
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

bool SDLApp::do_input(FLOAT_T &current_speed_x, FLOAT_T &current_speed_y,
                      FLOAT_T &current_speed_zoom,
                      FLOAT_T &zoom, FLOAT_T &zoom_speed,
                      const GridPixelSize &image_max_size,
                      FLOAT_T &xgrid, FLOAT_T &ygrid,
                      INT_T &window_w, INT_T &window_h) {
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
        SDL_GetWindowSize(this->window, &window_w_get, &window_h_get);
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

void SDLApp::delay() {
  // SDL_Delay(SDL_DELAY);
  SDL_Delay(64);
}

bool SDLApp::successful() const {
  return this->_successful;
}

SDLDrawableSurface::SDLDrawableSurface(SDLApp* const sdl_app,
                                       const ViewportPixelSize &viewport_pixel_size) {
  this->_sdl_app=sdl_app;
  this->_screen_surface=SDL_GetWindowSurface(sdl_app->window);
  SDL_Rect screen_rect;
  screen_rect.x=0;
  screen_rect.y=0;
  screen_rect.w=viewport_pixel_size.wpixel();
  screen_rect.h=viewport_pixel_size.hpixel();
  SDL_FillRect(this->_screen_surface, &screen_rect, SDL_MapRGB(sdl_app->format,0,0,0));
}

SDLDrawableSurface::~SDLDrawableSurface() {
  SDL_UpdateWindowSurface(this->_sdl_app->window);
  SDL_FreeSurface(this->_screen_surface);
}

SDL_Surface* SDLDrawableSurface::screen_surface() {
  return this->_screen_surface;
}

bool SDLDisplayTextureWrapper::is_valid () {
  return (this->_display_texture != nullptr);
}

SDLDisplayTextureWrapper::~SDLDisplayTextureWrapper () {
  if (this->_display_texture != nullptr) {
    SDL_FreeSurface(this->_display_texture);
    this->_display_texture=nullptr;
  }
}

void SDLDisplayTextureWrapper::create_surface(INT_T wpixel, INT_T hpixel) {
  if (this->_display_texture != nullptr) {
    this->unlock_surface();
  }
  this->_display_texture=SDL_CreateRGBSurfaceWithFormat(0,wpixel,hpixel,24,SDL_PIXELFORMAT_RGB24);
}

void SDLDisplayTextureWrapper::unload_surface() {
  SDL_FreeSurface(this->_display_texture);
  this->_display_texture=nullptr;
}

void* SDLDisplayTextureWrapper::pixels () {
  return this->_display_texture->pixels;
}

bool SDLDisplayTextureWrapper::lock_surface () {
  return SDL_LockSurface(this->_display_texture);
}

void SDLDisplayTextureWrapper::unlock_surface () {
  if (this->_display_texture != nullptr) {
    SDL_UnlockSurface(this->_display_texture);
  }
}

void SDLDisplayTextureWrapper::blit_texture(SDL_Surface* screen_surface,
                                            ViewportPixelCoordinate &viewport_pixel_coordinate,
                                            ViewportPixelSize &image_pixel_size_viewport) {
  SDL_Rect scaled_rect;
  scaled_rect.x=viewport_pixel_coordinate.xpixel();
  scaled_rect.y=viewport_pixel_coordinate.ypixel();
  scaled_rect.w=image_pixel_size_viewport.wpixel();
  scaled_rect.h=image_pixel_size_viewport.hpixel();
  SDL_BlitScaled(this->_display_texture, NULL, screen_surface, &scaled_rect);
}
