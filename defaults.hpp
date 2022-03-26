#ifndef DEFAULTS_HPP

#define DEFAULTS_HPP

// screen width
// TODO: make dynamic and configurable
#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT 720

// a max screen height
// used to decide how big of images should be loaded
// TODO: make dynamic and configurable
#define MAX_SCREEN_WIDTH  1920
#define MAX_SCREEN_HEIGHT 1080

// some constants for input/ouput configuration
const float KEY_PRESS_MOVE = 0.5;
const float JOY_BASE_MOVE = 2.0;
const float JOY_MAX = 32768;

// internal configuration settings
const int SDL_DELAY = 16;
const int TEXTURE_ALIGNMENT = 4;

#endif
