// C++ headers
#include <chrono>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
// C headers
#include <climits>
#include <cstdint>

////////////////////////////////////////////////////////////////////////////////
// Contains definitions to help with debugging.
////////////////////////////////////////////////////////////////////////////////
#ifndef COMMON_HPP
#define COMMON_HPP

// turn on/off debug messages
// these show progress of the application
#ifdef DEBUG_MESSAGES
#define DEBUG(x) do { std::cerr << "DEBUG: " << x << "\n" << std::flush; } while (0)
#else
#define DEBUG(x)
#endif

// turn on/off debug I/O messages
// show I/O, this tends to be much higher volume than standard DEBUG messages
#ifdef DEBUG_IO
#define DEBUGIO(x) do { std::cerr << "DEBUG IO: " << x << "\n" << std::flush; } while (0)
#else
#define DEBUGIO(x)
#endif

////////////////////////////////////////////////////////////////////////////////
// Give errors and other messages for the user
////////////////////////////////////////////////////////////////////////////////
// show an error
#define ERROR(x) do { std::cerr << "ERROR: " << x << std::endl; } while (0)
// show a non-fatal warning
#define WARN(x) do { std::cerr << "WARN: " << x << std::endl; } while (0)
// show a standard message
#define MSG(x) do { std::cerr << "MSG: " << x << std::endl; } while (0)
// produce a message but with no newline
#define MSGNONEWLINE(x) do { std::cerr << x; } while (0)

////////////////////////////////////////////////////////////////////////////////
// Types for this program.  I prefer to use these types of definitions
// so I can easily changed and so I'm not mixing
// long/short/signed/unsigned.
////////////////////////////////////////////////////////////////////////////////

typedef double FLOAT_T;
typedef uint64_t UINT_T;
typedef int64_t INT_T;


////////////////////////////////////////////////////////////////////////////////
// more complex types

// derived from https://en.cppreference.com/w/cpp/utility/hash
// can this be improved
struct hash_pair {
  template <class T1, class T2>
  std::size_t operator()(const std::pair<T1, T2>& p) const {
    auto h1=std::hash<T1>{}(p.first);
    auto h2=std::hash<T2>{}(p.second);

    if (h1 != h2) {
      return h1^h2;
    }
    return h1;
  }
};

typedef std::unordered_map<std::pair<INT_T,INT_T>,
                           std::unordered_map<std::pair<INT_T,INT_T>,
                                              std::string,
                                              hash_pair>,
                           hash_pair> FILE_DATA_T;

////////////////////////////////////////////////////////////////////////////////
// Contains some default values.  Many of these are placeholders for
// values that may eventually be configurable.
////////////////////////////////////////////////////////////////////////////////

// screen width
// TODO: make dynamic and configurable
const INT_T INITIAL_SCREEN_WIDTH=1280;
const INT_T INITIAL_SCREEN_HEIGHT=720;

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

// maximum minimum size of scaled images
const INT_T MAX_MIN_SCALED_IMAGE_SIZE=32;

const auto THREAD_SLEEP=std::chrono::milliseconds(1);

// some placeholder values
// rather than zero, this should make sure that any use of initialized class members becomes obvious
// TODO: will replace with assert or an exception
const INT_T INVALID_PIXEL_VALUE=INT_MIN;

// these are things that should probably be done more smartly later
const INT_T LOAD_FILES_BATCH=1;
const INT_T LOAD_TEXTURES_BATCH=8;

// the filler color
const unsigned char FILLER_LEVEL=64;

// max cache pixel size
const INT_T CACHE_MAX_PIXEL_SIZE=512;

// zoom step
// I don't see myself moving away from powers of two, but it's nice to
// have it visible where I make this assumption
const INT_T ZOOM_STEP=2;

// placeholder in command line arguments for empty file
const std::string EMPTY_FILE_PLACEHOLDER="[[EMPTY]]";

// size of joystick deadzone
// SDL assumes joystick is -32768 to 32768
// TODO: range may be off by one
const INT_T JOY_DEADZONE=2048;

#endif
