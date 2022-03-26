// local headers
#include "config.hpp"
#include "debug.hpp"
#include "error.hpp"
#include "defaults.hpp"
#include "imagegrid-viewer.hpp"
// C++ headers
#include <vector>
#include <iostream>
#include <string>
// C headers
#include <math.h>
#include <getopt.h>
using namespace std;

// library headers
#include <SDL2/SDL.h>

SDLApp::SDLApp() {
  int rendererFlags = SDL_RENDERER_ACCELERATED;
  int windowFlags = 0;

  format = SDL_AllocFormat(SDL_PIXELFORMAT_RGB24);

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
    ERROR("Couldn't initialize SDL:" << SDL_GetError());
    successful=false;
  } else {
    window = SDL_CreateWindow("Image Grid Viewer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, windowFlags);
    if (!window) {
      ERROR("Failed to open %d " << SCREEN_WIDTH << "x" << SCREEN_HEIGHT << " window: " << SDL_GetError());
      successful=false;
    } else {
      screen_surface = SDL_GetWindowSurface(window);

      if (SDL_NumJoysticks() < 1) {
        joystick_enabled=false;
      } else {
        joystick_enabled=true;
        game_controller = SDL_JoystickOpen(0);
      }
    }
  }
}

SDLApp::~SDLApp() {
  SDL_FreeSurface(screen_surface);
  SDL_DestroyWindow(window);
  window = nullptr;

  SDL_Quit();
}

void cleanup(ImageGrid* &grid) {
  if (grid) {
    delete grid;
    grid=nullptr;
  }
}

int main(int argc, char *argv[]) {
  bool continue_flag = true;
  bool load_images_successful = false;
  int loop_count=0;
  int width=0;
  int height=0;

  char path_value[256] = { 0 };
  vector<string> filenames;

  int opt;

  // get options
  while ((opt = getopt(argc ,argv, "w:h:p:")) != -1) {
    switch (opt) {
    case 'w':
      width=atoi(optarg);
      break;
    case 'h':
      height=atoi(optarg);
      break;
    case 'p':
      strcpy(path_value,optarg);
      break;
    case '?':
      if (optopt == 'w' || optopt == 'h' || optopt == 'p') {
        cerr << "Option " << optopt << " requires an argument.";
      } else {
        cerr << "Unknown option: " << (char)optopt << endl;
      }
      return 1;
    default:
      return 1;
    }
  }

  // get any files on the end
  if (optind != argc) {
    if (path_value[0] != 0) {
      ERROR("Cannot specify both a path and individual files.");
    } else {
      for (int i = optind; i < argc; i++) {
        filenames.push_back(string(argv[i]));
      }
    }
  }

  // Singletons for this application
  SDLApp sdl_app;
  ImageGrid* grid;
  TextureGrid* texture_grid;
  ViewPort viewport;
  vector<BlitItem> blititems;

  if (!sdl_app.successful) {
    ERROR("Failed to initialize properly");
  } else {
    grid=new ImageGrid(width,height);
    if (path_value[0] != 0) {
      DEBUG("load_image_grid() with w: " << width << " h: " << height << " " << path_value);
      load_images_successful = grid->load_images(path_value);
    } else {
      load_images_successful = grid->load_images(filenames);
    }
    if (!load_images_successful) {
      ERROR("Failed to load images!");
    } else {
      texture_grid=new TextureGrid(width,height);
      // TODO: need info above from down here
      texture_grid->init_max_size_zoom(grid);
      // initialize initial position
      if (width == 1) {
        viewport.viewport_xgrid = 0.5;
      }
      if (height == 1) {
        viewport.viewport_ygrid = 0.5;
      }
      while (continue_flag) {
        DEBUG("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
        DEBUG("++ Main loop begin");
        // stuff
        DEBUG("input()");
        continue_flag=viewport.do_input();
        DEBUG("input() done");
        // updates global variables
        DEBUG("find_viewport_grid()");
        // TODO: texture grid loading here
        viewport.find_viewport(grid,texture_grid,blititems);
        DEBUG("find_viewport_grid() done");
        // blit blitables
        viewport.blank_viewport(sdl_app.screen_surface, sdl_app.format);
        DEBUG("Loop count: " << loop_count);
        for (size_t i = 0; i < blititems.size(); i++) {
          DEBUG("blit loop:" << i);
          blititems[i].blit_this(sdl_app.screen_surface);
        }
        SDL_UpdateWindowSurface(sdl_app.window);
        SDL_Delay(SDL_DELAY);
        loop_count++;
      }
    }
  }
  cleanup(grid);
  return 0;
}
