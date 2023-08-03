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

const size_t PATH_BUFFER_SIZE=4096;

// some constants for input/ouput configuration
const FLOAT_T KEY_PRESS_MOVE=0.5;
const FLOAT_T JOY_BASE_MOVE=2.0;
// used to be 0.05
const FLOAT_T JOY_BASE_ZOOM=0.25;
const FLOAT_T JOY_MAX=32768;

// internal configuration settings
const int SDL_DELAY=16;
const INT_T TEXTURE_ALIGNMENT=4;

const double INITIAL_ZOOM=1.0;

const double INITIAL_ZOOM_SPEED=1.0;
const double INITIAL_CURRENT_ZOOM_SPEED=0.0;

const double INITIAL_X_Y_SPEED=0.0;

const double INITIAL_X=1.0;
const double INITIAL_Y=1.0;
// const double INITIAL_X=5.0;
// const double INITIAL_Y=5.0;

// this is set higher than any images I'm working with
// when data structure improve it could be dynamic
// TODO: this is bad, VERY VERY BAD!
const INT_T MAX_TEXTURE_ZOOM_INDEX=10;

// maximum minimum size of scaled images
const INT_T MAX_MIN_SCALED_IMAGE_SIZE=32;

const auto THREAD_SLEEP=std::chrono::milliseconds(1);

// some placeholder values
// rather than zero, this should make sure that any use of initialized class members becomes obvious
// TODO: will replace with assert or an exception
const INT_T INVALID_PIXEL_VALUE=INT_MIN;

// these are things that should probably be done more smartly later
const INT_T LOAD_FILES_BATCH=1;
const INT_T LOAD_TEXTURES_BATCH=4;

#endif
