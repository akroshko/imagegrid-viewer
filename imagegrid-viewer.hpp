#ifndef IMAGEGRIDVIEWER_HPP

#define IMAGEGRIDVIEWER_HPP

#include "config.hpp"
#include "debug.hpp"
#include "error.hpp"
#include "defaults.hpp"
#include "gridclasses.hpp"
#include "viewport.hpp"
#include "fileload.hpp"

// C++ headers
#include <vector>
#include <iostream>
using namespace std;

// library headers
#include <SDL2/SDL.h>

// a class to contain SDL boilerplate
class SDLApp {
public:
  SDLApp();
  ~SDLApp();
  SDL_Renderer *renderer;
  SDL_Window *window;
  bool joystick_enabled;
  SDL_Joystick* game_controller;
  SDL_PixelFormat *format;
  SDL_Surface* screen_surface;
  bool successful=true;
};

#endif
