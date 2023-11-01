/**
 * Test code to blit a 640x480 texture to an SDL window.
 */
#include "common.hpp"
#include "utility.hpp"
#include "imagegrid/imagegrid.hpp"
// C compatible headers
#include "c_sdl/sdl.hpp"

int main(int argc, char* argv[]) {
  auto texture_wpixel=512ul;
  auto texture_hpixel=512ul;
  // create the SDL app
  auto sdl_app=std::make_unique<SDLApp>();
  // create an imagegridzoom level
  auto square=std::make_unique<ImageGridSquareZoomLevel>(2);
  auto file_to_load=std::string{"/opt/imagegrid-viewer/082e01_02.tif"};
  // auto file_to_load=std::string{"./test_vert.tif"};
  // auto file_to_load=std::string{"./gradient.tif"};
  // auto file_to_load=std::string{"./color_grid.tif"};
  // auto file_to_load=std::string{"test_map_4.tif"};
  // load in a file
  std::vector<ImageGridSquareZoomLevel*> squares;
  squares.emplace_back(square.get());
  ImageGridSquareZoomLevel::load_square(file_to_load,
                                      // TODO: replace with cached file
                                      file_to_load,
                                      squares);
  // make a cool texture
  // now transfer the RGB data to texture and blit it
  SDL_Surface* display_texture=nullptr;
  display_texture=SDL_CreateRGBSurfaceWithFormat(0,texture_wpixel,texture_hpixel,32,SDL_PIXELFORMAT_RGBA32);
  SDL_LockSurface(display_texture);
  INT64 index=0;
  for (auto i=0ul; i < square->rgb_wpixel(index,index); i++) {
    for (auto j=0ul; j < square->rgb_wpixel(index,index); j++) {
      auto rgb_index=(i*square->rgb_wpixel(index,index)+j)*3;
      auto texture_index=(i*texture_hpixel+j)*3;
      if (i < texture_wpixel && j < texture_hpixel) {
        ((unsigned char *)display_texture->pixels)[texture_index]=square->get_rgb_data(index,index)[rgb_index];
        ((unsigned char *)display_texture->pixels)[texture_index+1]=square->get_rgb_data(index,index)[rgb_index+1];
        ((unsigned char *)display_texture->pixels)[texture_index+2]=square->get_rgb_data(index,index)[rgb_index+2];
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
  SDL_BlitScaled(display_texture, NULL, sdl_app->_screen_surface, &scaled_rect);
  SDL_UpdateWindowSurface(sdl_app->window());
  SDL_Delay(8192);
}
