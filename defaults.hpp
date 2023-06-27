/**
 * Contains some default values.  Many of these may eventually be
 * configurable.
 */
#ifndef DEFAULTS_HPP
#define DEFAULTS_HPP
#include "types.hpp"
#include <chrono>

// screen width
// TODO: make dynamic and configurable
const INT_T SCREEN_WIDTH=1280;
const INT_T SCREEN_HEIGHT=720;

// a max screen height
// used to decide how big of images should be loaded
// TODO: make dynamic and configurable
const INT_T MAX_SCREEN_WIDTH=1920;
const INT_T MAX_SCREEN_HEIGHT=1080;

// some constants for input/ouput configuration
const FLOAT_T KEY_PRESS_MOVE = 0.5;
const FLOAT_T JOY_BASE_MOVE = 2.0;
const FLOAT_T JOY_MAX = 32768;

// internal configuration settings
const int SDL_DELAY = 16;
const INT_T TEXTURE_ALIGNMENT = 4;

const double INITIAL_ZOOM = 1.0;

const double INITIAL_ZOOM_SPEED = 1.0;
const double INITIAL_CURRENT_ZOOM_SPEED=0.0;

const double INITIAL_X_Y_SPEED = 0.0;

const double INITIAL_X = 1.0;
const double INITIAL_Y = 1.0;

// this is set higher than any images I'm working with
// when data structure improve it could be dynamic
const INT_T MAX_ZOOM_LEVELS=10;

// a placeholder index that will be changed in the future
const INT_T IMAGE_GRID_INDEX=0;

const auto THREAD_SLEEP = std::chrono::milliseconds(50);

#endif
