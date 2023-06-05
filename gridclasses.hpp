#ifndef GRIDCLASSES_HPP

#define GRIDCLASSES_HPP

#include "config.hpp"
#include "debug.hpp"
#include "error.hpp"
// C++ headers
#include <vector>
#include <iostream>
#include <string>
#include <mutex>

// library headers
#include <SDL2/SDL.h>

class ImageGridSquareZoomLevel {
public:
  ImageGridSquareZoomLevel();
  ~ImageGridSquareZoomLevel();
  std::mutex load_mutex;
  size_t rgb_wpixel;
  size_t rgb_hpixel;
  // the actual RGB data
  unsigned char* rgb_data;
};


class ImageGridSquare {
  /* An individual square on the grid. */
public:
  ImageGridSquare();
  ~ImageGridSquare();
  // // the width in pixels of the image in this square
  // size_t rgb_wpixel;
  // // the height in pixels of the image in this square
  // size_t rgb_hpixel;
  // // the actual RGB data
  // unsigned char* rgb_data;
  // load a file into this square
  ImageGridSquareZoomLevel **image_array;
  void load_file(std::string filename);
};

class ImageGrid {
  /* The grid of images.  In the future these will be lazily loaded from disk/cache. */
public:
  ImageGrid(size_t width, size_t height);
  ~ImageGrid();
  // the width of this grid in images
  size_t images_wgrid;
  // the height of this grid in images
  size_t images_hgrid;
  // the maximum width of an image square in pixels
  size_t images_max_wpixel;
  // the maximum height of an image square in pixels
  size_t images_max_hpixel;
  // the indidivual squares
  ImageGridSquare** squares;
  // load a set of images from a path
  //   filenames: a list of files to load, order in grid is left to right, top to bottom
  bool load_images(std::vector<std::string> filenames);
  //   pathname: either a preset name to load or the path
  bool load_images(char *pathname);
private:
  bool load_grid(std::vector<std::string> file_list);
};

class TextureGridSquareZoomLevel {
  /* An individual square at a particular zoom level in the texture grid. */
public:
  TextureGridSquareZoomLevel();
  ~TextureGridSquareZoomLevel();
  // lock when the display_area is being worked on
  std::mutex display_mutex;
  SDL_Surface* display_texture;
};

class TextureGridSquare {
  /* An individual square in the texture grid. */
public:
  TextureGridSquare();
  ~TextureGridSquare();
  // the width of the texture with no zoom
  int texture_wpixel;
  // the height of the texture with no zoom
  int texture_hpixel;
  // an array of textures
  // the first element of the array is the full-size texture
  // the subsequent elements are zoomed textures each reduced by a factor of 2
  TextureGridSquareZoomLevel** texture_array;
  // does not work for now because elements of texture_array contain a mutex
  // std::array<TextureGridSquareZoomLevel*, 10> texture_array;
};

class TextureGrid {
  /* The grid as textures.  These are generally loaded lazily. */
public:
  TextureGrid(size_t width, size_t height);
  ~TextureGrid();
  // the width of this grid in textures
  size_t textures_wgrid;
  // the height of this grid in textures
  size_t textures_hgrid;
  // the maximum zoom (maximum number of reductions by a factor of 2)
  int textures_max_zoom;
  // maximum width of the individual textures in pixels
  int textures_max_wpixel;
  // maximum height of the individual textures in pixels
  int textures_max_hpixel;
  // the indidivual squares
  TextureGridSquare** squares;
  // initialize the maximum size of each texture and the maximum zoom
  // generally has to be done after all imagesare loaded
  void init_max_size_zoom(ImageGrid *grid);
  // load a texture
  //   source_square: the square containing the RGB data
  //   dest_square: the destination square to load the texture into
  bool load_texture(TextureGridSquare &dest_square, ImageGridSquare &source_square, int z);
  // update the textures based on the current coordinates and zoom level
  //   grid: the ImageGrid
  //   xgrid: the x coordinate on the grid
  //   ygrid: the y coordinate on the grid
  //   loadall: load all textures at this zoom level, otherwise a 3x3 is loaded
  void update_textures (ImageGrid *grid, float xgrid, float ygrid,int zoom_level, bool load_all);
};

#endif
