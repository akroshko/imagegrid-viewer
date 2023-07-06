/**
 * Contains some default values.  Many of these may eventually be
 * configurable.
 */
#ifndef DEFAULTS_HPP
#define DEFAULTS_HPP
#include "types.hpp"
#include <chrono>
#include <climits>

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
const FLOAT_T KEY_PRESS_MOVE=0.5;
const FLOAT_T JOY_BASE_MOVE=2.0;
const FLOAT_T JOY_MAX=32768;

// internal configuration settings
const int SDL_DELAY=16;
const INT_T TEXTURE_ALIGNMENT=4;

const double INITIAL_ZOOM=1.0;

const double INITIAL_ZOOM_SPEED=1.0;
const double INITIAL_CURRENT_ZOOM_SPEED=0.0;

const double INITIAL_X_Y_SPEED=0.0;

// const double INITIAL_X=1.0;
// const double INITIAL_Y=1.0;
const double INITIAL_X=5.0;
const double INITIAL_Y=5.0;

// this is set higher than any images I'm working with
// when data structure improve it could be dynamic
// TODO: this is bad, VERY VERY BAD!
const INT_T MAX_TEXTURE_ZOOM_LEVELS=10;

// a placeholder index that will be changed in the future
const INT_T IMAGE_GRID_BASE_INDEX=0;
const INT_T IMAGE_GRID_LENGTH=3;
// const INT_T IMAGE_GRID_LENGTH=1;

// 60 FPS?
const auto THREAD_SLEEP=std::chrono::milliseconds(4);
const auto MINI_SLEEP=std::chrono::milliseconds(1);

// some placeholder values

// rather than zero, this should make sure that any use of initialized class members becomes obvious
// TODO: will replace with assert or an exception
const INT_T INVALID_PIXEL_VALUE=INT_MIN;

const INT_T LOAD_FILES_BATCH=1;

#endif
