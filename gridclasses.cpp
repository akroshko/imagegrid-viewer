// local headers
#include "config.hpp"
#include "debug.hpp"
#include "error.hpp"
#include "fileload.hpp"
#include "gridclasses.hpp"
// C headers
#include <math.h>
// C++ headers
#include <iostream>
#include <string>
using namespace std;

ImageGridSquare::ImageGridSquare() {
}

void ImageGridSquare::load_file (string filename) {
  if (check_tiff(filename)) {
    // TODO: check success
    load_tiff_as_rgb(filename,rgb_wpixel,rgb_hpixel,&rgb_data);
  } else if (check_png(filename)) {
    DEBUG("Constructing PNG: " << filename);
    // TODO: check success
    load_png_as_rgb(filename,rgb_wpixel,rgb_hpixel,&rgb_data);
    DEBUG("Done PNG: " << filename);
  } else {
    ERROR("ImageGridSquare::load_file can't load: " << filename);
  }
}

ImageGridSquare::~ImageGridSquare() {
  delete[] rgb_data;
}

ImageGrid::ImageGrid(int width, int height) {
  images_wgrid=width;
  images_hgrid=height;
  squares = new ImageGridSquare*[width];
  for (size_t i = 0; i < width; i++) {
    squares[i] = new ImageGridSquare[height];
  }
}

ImageGrid::~ImageGrid() {
  for (size_t i = 0; i < images_wgrid; i++) {
    delete[] squares[i];
    squares[i]=nullptr;
  }
  delete[] squares;
  squares=nullptr;
}

bool ImageGrid::load_images(vector<string> filenames) {
  return load_grid(filenames);
}

bool ImageGrid::load_images(char *pathname) {
  vector<string> filenames;
  filenames=load_numbered_images(string(pathname));
  return load_grid(filenames);
}

bool ImageGrid::load_grid(vector<string> filenames) {
  bool successful=true;
  // new zoom stuff
  for (size_t i = 0; i < images_wgrid; i++) {
    if (!successful) {
      squares[i]=nullptr;
      continue;
    }
    for (size_t j = 0; j < images_hgrid; j++) {
      if (!successful) {
        continue;
      }
      int ij=j*images_wgrid+i;
      MSG("Loading: " << filenames[ij]);
      squares[i][j].load_file(filenames[ij]);
      // set the RGB of the surface
      if (squares[i][j].rgb_wpixel > images_max_wpixel) {
        images_max_wpixel=squares[i][j].rgb_wpixel+(TEXTURE_ALIGNMENT - (squares[i][j].rgb_wpixel % TEXTURE_ALIGNMENT));
      }
      if (squares[i][j].rgb_hpixel > images_max_hpixel) {
        images_max_hpixel=squares[i][j].rgb_hpixel;
      }
    }
  }
  return successful;
}

TextureGridSquareZoomLevel::TextureGridSquareZoomLevel () {
  display_texture=nullptr;
}

TextureGridSquareZoomLevel::~TextureGridSquareZoomLevel () {
  if (display_texture) {
    SDL_FreeSurface(display_texture);
  }
}

TextureGridSquare::TextureGridSquare () {
  DEBUG("TextureGridSquare Constructor");
  image_array=new TextureGridSquareZoomLevel*[10]();
  for (size_t i = 0; i < 10; i++) {
    image_array[i]=new TextureGridSquareZoomLevel();
  }
}

TextureGridSquare::~TextureGridSquare () {
  for (size_t i = 0; i < 10; i++) {
    delete image_array[i];
    image_array[i]=nullptr;
  }
  delete[] image_array;
  image_array=nullptr;
}

TextureGrid::TextureGrid (int width, int height) {
  textures_wgrid=width;
  textures_hgrid=height;
  squares = new TextureGridSquare*[width];
  for (size_t i = 0; i < width; i++) {
    squares[i] = new TextureGridSquare[height];
  }
}

void TextureGrid::init_max_size_zoom(ImageGrid *grid) {
  int zoom_length = 0;
  float current_zoom=1.0;
  // swap out and reallocate
  textures_max_wpixel=grid->images_max_wpixel;
  textures_max_hpixel=grid->images_max_hpixel;
  zoom_length += 1;
  current_zoom /= 2.0;
  // TODO: inefficient, do without a while loop
  while (!(((textures_wgrid*textures_max_wpixel*current_zoom) < MAX_SCREEN_WIDTH) && ((textures_hgrid*textures_max_hpixel*current_zoom) < MAX_SCREEN_HEIGHT))) {
    zoom_length += 1;
    current_zoom /= 2.0;
  }
  textures_max_zoom=zoom_length-1;
}

bool TextureGrid::load_texture (TextureGridSquare &dest_square, ImageGridSquare &source_square, int z) {
  bool successful=true;
  int source_wpixel=source_square.rgb_wpixel;
  int source_hpixel=source_square.rgb_hpixel;
  int dest_wpixel=(dest_square.texture_wpixel)/((int)pow(2,z));
  int dest_hpixel=(dest_square.texture_hpixel)/((int)pow(2,z));
  dest_wpixel=dest_wpixel + (TEXTURE_ALIGNMENT - (dest_wpixel % TEXTURE_ALIGNMENT));
  dest_square.image_array[z]->display_texture = SDL_CreateRGBSurfaceWithFormat(0,dest_wpixel,dest_hpixel,24,SDL_PIXELFORMAT_RGB24);
  SDL_LockSurface(dest_square.image_array[z]->display_texture);
  int skip = ((int)pow(2,z));
  if (z == 0) {
    // use memcpy to hopefully take advantage of standard library when zoom index is zero
    for (size_t l = 0; l < source_hpixel; l+=skip) {
      int dest_index = (l*dest_wpixel)*3;
      int source_index = (l*source_wpixel)*3;
      memcpy(((unsigned char *)dest_square.image_array[z]->display_texture->pixels)+dest_index,((unsigned char *)source_square.rgb_data)+source_index,sizeof(unsigned char)*source_wpixel*3);
    }
  } else {
    int ld=0;
    for (size_t l = 0; l < source_hpixel; l+=skip) {
      int kd=0;
      for (size_t k = 0; k < source_wpixel; k+=skip) {
        int dest_index = (ld*dest_wpixel+kd)*3;
        int source_index = (l*source_wpixel+k)*3;
        ((unsigned char *)dest_square.image_array[z]->display_texture->pixels)[dest_index]=source_square.rgb_data[source_index];
        ((unsigned char *)dest_square.image_array[z]->display_texture->pixels)[dest_index+1]=source_square.rgb_data[source_index+1];
        ((unsigned char *)dest_square.image_array[z]->display_texture->pixels)[dest_index+2]=source_square.rgb_data[source_index+2];
        kd++;
      }
      ld++;
    }
  }
  SDL_UnlockSurface(dest_square.image_array[z]->display_texture);
  return successful;
}

// these functions are specific to how coordinates that are integers are
float next_smallest(float x) {
  return floor(x) - 1.0;
}

float next_largest(float x) {
  if (x == ceil(x)) {
    return x + 1.0;
  } else {
    return ceil(x);
  }
}

void TextureGrid::update_textures (ImageGrid *grid, float xgrid, float ygrid, int zoom_level, bool load_all) {
  DEBUG("update_textures() " << zoom_level << " | " << xgrid << " | " << ygrid);
  // j,i is better for plotting a grid
  for (size_t j = 0; j < textures_hgrid; j++) {
    MSGNONEWLINE("| ");
    // TODO: this will eventually be a switch statement to load in different things
    for (size_t i = 0; i < textures_wgrid; i++) {
      std::unique_lock<std::mutex> lock(squares[i][j].image_array[zoom_level]->display_mutex, std::defer_lock);
      // skip everything if locked
      if (lock.try_lock()) {
        if (!load_all &&                                         \
            ((i < next_smallest(xgrid)) ||                       \
             (i > next_largest(xgrid)) ||                        \
             (j < next_smallest(ygrid)) ||                       \
             (j > next_largest(ygrid)))) {
          MSGNONEWLINE("0 ");
          if (squares[i][j].image_array[zoom_level]->display_texture != nullptr) {
            SDL_FreeSurface(squares[i][j].image_array[zoom_level]->display_texture);
            squares[i][j].image_array[zoom_level]->display_texture = nullptr;
          }
        } else {
          MSGNONEWLINE("L ");
          squares[i][j].texture_wpixel=textures_max_wpixel;
          squares[i][j].texture_hpixel=textures_max_hpixel;
          if (squares[i][j].image_array[zoom_level]->display_texture == nullptr) {
            load_texture(squares[i][j], grid->squares[i][j], zoom_level);
          }
        }
      }
    }
    MSGNONEWLINE("|" << endl);
  }
  DEBUG("update_textures() end");
}
