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
  auto rendererFlags=SDL_RENDERER_ACCELERATED;
  int windowFlags=0;
  this->format=SDL_AllocFormat(SDL_PIXELFORMAT_RGB24);
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
    ERROR("Couldn't initialize SDL:" << SDL_GetError());
    this->_successful=false;
  } else {
    this->window=SDL_CreateWindow("Image Grid Viewer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, windowFlags);
    if (!this->window) {
      ERROR("Failed to open %d " << SCREEN_WIDTH << "x" << SCREEN_HEIGHT << " window: " << SDL_GetError());
      this->_successful=false;
    } else {
      this->screen_surface=SDL_GetWindowSurface(window);
      if (SDL_NumJoysticks() < 1) {
        this->joystick_enabled=false;
      } else {
        this->joystick_enabled=true;
        this->game_controller=SDL_JoystickOpen(0);
      }
    }
  }
}

SDLApp::~SDLApp() {
  SDL_FreeSurface(screen_surface);
  SDL_DestroyWindow(window);
  window=nullptr;
  SDL_Quit();
}

bool SDLApp::do_input(FLOAT_T &current_speed_x, FLOAT_T &current_speed_y, FLOAT_T &current_speed_zoom, FLOAT_T &zoom, FLOAT_T &zoom_speed, const GridPixelSize &image_max_size, FLOAT_T &xgrid, FLOAT_T &ygrid) {
  SDL_Event e;
  auto keep_going=true;
  while(SDL_PollEvent(&e)) {
    auto pixel_size=(100.0/image_max_size.wpixel()/zoom);
    if (e.type == SDL_QUIT) {
      return false;
    } else if (e.type == SDL_JOYAXISMOTION) {
      // TODO: deal with dead zones
      if (e.jaxis.which == 0) {
        DEBUGIO("jaxis 0: " << e.jaxis.axis << " ++ " << e.jaxis.value);
        if (e.jaxis.axis == 0) {
          current_speed_x=(e.jaxis.value/JOY_MAX)*(JOY_BASE_MOVE*pixel_size);
        } else if (e.jaxis.axis == 1) {
          current_speed_y=(e.jaxis.value/JOY_MAX)*(JOY_BASE_MOVE*pixel_size);
        } else if (e.jaxis.axis == 4) {
          current_speed_zoom=JOY_BASE_ZOOM*(e.jaxis.value/JOY_MAX);
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

void SDLApp::blank_viewport(const ViewportPixelSize &viewport_pixel_size) {
  SDL_Rect screen_rect;
  screen_rect.x=0;
  screen_rect.y=0;
  screen_rect.w=viewport_pixel_size.wpixel();
  screen_rect.h=viewport_pixel_size.hpixel();
  SDL_FillRect(this->screen_surface, &screen_rect, SDL_MapRGB(this->format,0,0,0));
}

void SDLApp::delay() {
  // SDL_Delay(SDL_DELAY);
  SDL_Delay(64);
}

bool SDLApp::successful() {
  return this->_successful;
}
