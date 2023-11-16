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
// TODO: eventually only show internals in non-release builds
#define ERROR(x) do { std::cerr << "ERROR: " << __FILE__ << ":" << __LINE__ << " " << x << std::endl; } while (0)
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

// types for generic operations
typedef double FLOAT64;
typedef uint64_t UINT64;
typedef int64_t INT64;
// types for pixels and other data
typedef uint32_t PIXEL_RGBA;

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

////////////////////////////////////////////////////////////////////////////////
// special data structure to help setup grid
// TODO: put here to avoid circular import, restructure to avoid this
struct GridSetupFile {
  INT64 grid_i;
  INT64 grid_j;
  INT64 subgrid_i;
  INT64 subgrid_j;
  std::string filename;
};

////////////////////////////////////////////////////////////////////////////////
// Contains some default values.  Many of these are placeholders for
// values that may eventually be configurable.
////////////////////////////////////////////////////////////////////////////////

// screen width
// TODO: make dynamic and configurable
const INT64 INITIAL_SCREEN_WIDTH=1280;
const INT64 INITIAL_SCREEN_HEIGHT=720;

// a max screen height
// used to decide how big of images should be loaded
// TODO: make dynamic and configurable
const INT64 MAX_SCREEN_WIDTH=1920;
const INT64 MAX_SCREEN_HEIGHT=1080;

const size_t PATH_BUFFER_SIZE=4096;

// some constants for input/ouput configuration
const FLOAT64 KEY_PRESS_MOVE=0.5;
const FLOAT64 JOY_BASE_MOVE=2.0;
// used to be 0.05
const FLOAT64 JOY_BASE_ZOOM=0.25;
const FLOAT64 JOY_MAX=32768;

// internal configuration settings

// TODO: changing SDL_DELAY from 64 to 16 slows down loading of a
// particular set of maps from 1'05 to 1'17, investigate later
const int SDL_DELAY=16;
const INT64 TEXTURE_ALIGNMENT=4;
// experimental
// const INT64 SMALL_TEXTURE_ALIGNMENT=3;
// const INT64 LARGE_TEXTURE_ALIGNMENT=16;

const double INITIAL_ZOOM=0.0125;

const double INITIAL_ZOOM_SPEED=1.0;
const double INITIAL_CURRENT_ZOOM_SPEED=0.0;

const double INITIAL_X_Y_SPEED=0.0;

const double INITIAL_X=1.5;
const double INITIAL_Y=1.5;
// const double INITIAL_X=5.0;
// const double INITIAL_Y=5.0;

// maximum minimum size of scaled images
const INT64 MAX_MIN_SCALED_IMAGE_SIZE=32;

// size of the individual textures
// const INT64 TILE_PIXEL_BASE_SIZE=2048;
// for debugging
// const INT64 TILE_PIXEL_BASE_SIZE=8192;
const INT64 TILE_PIXEL_BASE_SIZE=16384;

// some placeholder values
// rather than zero, this should make sure that any use of initialized class members becomes obvious
// TODO: will replace with assert or an exception
const INT64 INVALID_PIXEL_VALUE=INT_MIN;

// these are things that should probably be done more smartly later
const INT64 LOAD_FILES_BATCH=1;
const INT64 LOAD_TEXTURES_BATCH=8;

// the filler color
const PIXEL_RGBA FILLER_LEVEL=0xFF404040;

// max cache pixel size
const INT64 CACHE_MAX_PIXEL_SIZE=512;

// where to put the overlay
const INT64 OVERLAY_X=10;
const INT64 OVERLAY_Y=10;

// overlay font path
const char OVERLAY_FONT_PATH[]="/usr/share/fonts/truetype/freefont/FreeSans.ttf";

// zoom step
// I don't see myself moving away from powers of two, but it's nice to
// have it visible where I make this assumption
const INT64 ZOOM_STEP=2;

// placeholder in command line arguments for empty file
const std::string EMPTY_FILE_PLACEHOLDER="[[EMPTY]]";

// stuff that needs to be crossplatformed later
const char TEMP_TEMPLATE_TIF[]="/tmp/imagegrid_temp_XXXXXX.tif";

// NTS file internals
const char NTS_TIF_INTERNAL_EXTENSION[]="tif";

// size of joystick deadzone
// SDL assumes joystick is -32768 to 32768
// TODO: range may be off by one
const INT64 JOY_DEADZONE=2048;

////////////////////////////////////////////////////////////////////////////////
// Strings for user interaction

const std::string HELP_STRING=
  "Usage: imagegrid-viewer [-c|-d] -w WIDTH -h HEIGHT IMAGES...\n"
  "       imagegrid-viewer [-c|-d] -f TEXT_FILE\n"
  "\n"
  "  -c        create cache\n"
  "  -d        use cache\n"
  "\n"
  "  -w        width of grid in images\n"
  "  -h        height of grid in images\n"
  "\n"
  "  -f        text file with each line in the format:\n"
  "            X_INDEX Y_INDEX X_SUBGRID_INDEX Y_SUBGRID_INDEX FILENAME\n";

#endif
