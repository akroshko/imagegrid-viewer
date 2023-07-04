/**
 * Test code to blit a 640x480 texture to an SDL window.
 */
#include "debug.hpp"
#include "error.hpp"
#include "defaults.hpp"
#include "utility.hpp"
#include "sdl.hpp"
#include "gridclasses.hpp"

int main(int argc, char *argv[]) {
  auto texture_wpixel=512ul;
  auto texture_hpixel=512ul;
  // create the SDL app
  SDLApp *sdl_app=new SDLApp();
  // create an imagegridzoom level
  auto square=new ImageGridSquareZoomLevel();
  auto file_to_load = std::string{"/opt/imagegrid-viewer/082e01_02.tif"};
  // auto file_to_load = std::string{"./test_vert.tif"};
  // auto file_to_load = std::string{"./gradient.tif"};
  // auto file_to_load = std::string{"./color_grid.tif"};
  // auto file_to_load = std::string{"test_map_4.tif"};
  square->zoom_level=2;
  // load in a file
  square->load_file(file_to_load);
  // make a cool texture
  // now transfer the RGB data to texture and blit it
  SDL_Surface* display_texture=nullptr;
  display_texture = SDL_CreateRGBSurfaceWithFormat(0,texture_wpixel,texture_hpixel,24,SDL_PIXELFORMAT_RGB24);
  SDL_LockSurface(display_texture);
  for (auto i = 0ul; i < square->rgb_wpixel; i++) {
      for (auto j = 0ul; j < square->rgb_wpixel; j++) {
        auto rgb_index=(i*square->rgb_wpixel+j)*3;
        auto texture_index=(i*texture_hpixel+j)*3;
        if (i < texture_wpixel && j < texture_hpixel) {
          ((unsigned char *)display_texture->pixels)[texture_index]=square->rgb_data[rgb_index];
          ((unsigned char *)display_texture->pixels)[texture_index+1]=square->rgb_data[rgb_index+1];
          ((unsigned char *)display_texture->pixels)[texture_index+2]=square->rgb_data[rgb_index+2];
        }
      }
  }
  SDL_UnlockSurface(display_texture);
  // now blit this
  SDL_Rect scaled_rect;
  scaled_rect.x=0;
  scaled_rect.y=0;
  scaled_rect.w=texture_wpixel;
  scaled_rect.h=texture_hpixel;
  SDL_BlitScaled(display_texture, NULL, sdl_app->screen_surface, &scaled_rect);
  SDL_UpdateWindowSurface(sdl_app->window);
  SDL_Delay(8192);
  DELETE_IF_NOT_NULLPTR(square);
  DELETE_IF_NOT_NULLPTR(sdl_app);
}
