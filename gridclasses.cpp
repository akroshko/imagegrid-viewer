// local headers
#include "config.hpp"
#include "debug.hpp"
#include "error.hpp"
#include "utility.hpp"
#include "fileload.hpp"
#include "gridclasses.hpp"
// C headers
#include <math.h>
// C++ headers
#include <iostream>
#include <string>

ImageGridSquareZoomLevel::ImageGridSquareZoomLevel() {
}

ImageGridSquareZoomLevel::~ImageGridSquareZoomLevel() {
  delete[] this->rgb_data;
}

ImageGridSquare::ImageGridSquare() {
  this->image_array=new ImageGridSquareZoomLevel*[IMAGE_GRID_INDEX+1];
  for (size_t i = 0; i < IMAGE_GRID_INDEX+1; i++) {
    this->image_array[i]=new ImageGridSquareZoomLevel();
  }
}

void ImageGridSquare::load_file (std::string filename) {
  if (check_tiff(filename)) {
    // TODO: check success
    load_tiff_as_rgb(filename,
                     this->image_array[IMAGE_GRID_INDEX]->rgb_wpixel,
                     this->image_array[IMAGE_GRID_INDEX]->rgb_hpixel,
                     &this->image_array[IMAGE_GRID_INDEX]->rgb_data);
  } else if (check_png(filename)) {
    DEBUG("Constructing PNG: " << filename);
    // TODO: check success
    load_png_as_rgb(filename,
                    this->image_array[IMAGE_GRID_INDEX]->rgb_wpixel,
                    this->image_array[IMAGE_GRID_INDEX]->rgb_hpixel,
                    &this->image_array[IMAGE_GRID_INDEX]->rgb_data);
    DEBUG("Done PNG: " << filename);
  } else {
    ERROR("ImageGridSquare::load_file can't load: " << filename);
  }
}

ImageGridSquare::~ImageGridSquare() {
  for (size_t i = 0; i < IMAGE_GRID_INDEX+1; i++) {
    delete[] this->image_array[i];
    this->image_array[i]=nullptr;
  }
  delete[] this->image_array;
  this->image_array=nullptr;
}

ImageGrid::ImageGrid(size_t width, size_t height) {
  this->coordinate_info.images_wgrid=width;
  this->coordinate_info.images_hgrid=height;
  this->squares = new ImageGridSquare*[width];
  for (size_t i = 0; i < width; i++) {
    this->squares[i] = new ImageGridSquare[height];
  }
}

ImageGrid::~ImageGrid() {
  for (size_t i = 0; i < this->coordinate_info.images_wgrid; i++) {
    delete[] this->squares[i];
    this->squares[i]=nullptr;
  }
  delete[] this->squares;
  this->squares=nullptr;
}

bool ImageGrid::load_images(std::vector<std::string> filenames) {
  return load_grid(filenames);
}

bool ImageGrid::load_images(char *pathname) {
  std::vector<std::string> filenames;
  filenames=load_numbered_images(std::string(pathname));
  return load_grid(filenames);
}

bool ImageGrid::load_grid(std::vector<std::string> filenames) {
  auto successful=true;
  // TODO: sort out this when refactoring file loading
  for (size_t i = 0; i < this->coordinate_info.images_wgrid; i++) {
    if (!successful) {
      squares[i]=nullptr;
      continue;
    }
    for (size_t j = 0; j < this->coordinate_info.images_hgrid; j++) {
      if (!successful) {
        continue;
      }
      int ij=j*this->coordinate_info.images_wgrid+i;
      MSG("Loading: " << filenames[ij]);
      squares[i][j].load_file(filenames[ij]);
      // set the RGB of the surface
      // TODO: encapsulate calculation of max pixels
      auto rgb_wpixel=squares[i][j].image_array[IMAGE_GRID_INDEX]->rgb_wpixel;
      auto rgb_hpixel=squares[i][j].image_array[IMAGE_GRID_INDEX]->rgb_hpixel;
      auto max_wpixel=this->coordinate_info.images_max_wpixel;
      auto max_hpixel=this->coordinate_info.images_max_hpixel;
      if (rgb_wpixel > max_wpixel) {
        this->coordinate_info.images_max_wpixel=rgb_wpixel+(TEXTURE_ALIGNMENT - (rgb_wpixel % TEXTURE_ALIGNMENT));
      }
      if (rgb_hpixel > max_hpixel) {
        this->coordinate_info.images_max_hpixel=rgb_hpixel;
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
  this->texture_array=new TextureGridSquareZoomLevel*[MAX_ZOOM_LEVELS]();
  for (size_t i = 0; i < MAX_ZOOM_LEVELS; i++) {
    this->texture_array[i]=new TextureGridSquareZoomLevel();
  }
}

TextureGridSquare::~TextureGridSquare () {
  for (size_t i = 0; i < MAX_ZOOM_LEVELS; i++) {
    delete this->texture_array[i];
    this->texture_array[i]=nullptr;
  }
  delete[] this->texture_array;
  this->texture_array=nullptr;
}

TextureGrid::TextureGrid (size_t width, size_t height) {
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
  textures_max_wpixel=grid->coordinate_info.images_max_wpixel;
  textures_max_hpixel=grid->coordinate_info.images_max_hpixel;
  zoom_length += 1;
  current_zoom /= 2.0;
  // TODO: inefficient, do without a while loop
  while (!(((textures_wgrid*textures_max_wpixel*current_zoom) < MAX_SCREEN_WIDTH) && ((textures_hgrid*textures_max_hpixel*current_zoom) < MAX_SCREEN_HEIGHT))) {
    zoom_length += 1;
    current_zoom /= 2.0;
  }
  textures_max_zoom=zoom_length-1;
}

bool TextureGrid::load_texture (TextureGridSquare &dest_square,
                                ImageGridSquare &source_square,
                                int zoom) {
  auto successful=true;
  auto source_wpixel=source_square.image_array[IMAGE_GRID_INDEX]->rgb_wpixel;
  auto source_hpixel=source_square.image_array[IMAGE_GRID_INDEX]->rgb_hpixel;
  auto zoom_reduction=((int)pow(2,zoom));
  auto dest_wpixel=(dest_square.texture_wpixel)/zoom_reduction;
  auto dest_hpixel=(dest_square.texture_hpixel)/zoom_reduction;
  dest_wpixel=dest_wpixel + (TEXTURE_ALIGNMENT - (dest_wpixel % TEXTURE_ALIGNMENT));
  dest_square.texture_array[zoom]->display_texture = SDL_CreateRGBSurfaceWithFormat(0,dest_wpixel,dest_hpixel,24,SDL_PIXELFORMAT_RGB24);
  SDL_LockSurface(dest_square.texture_array[zoom]->display_texture);
  auto skip = zoom_reduction;
  if (zoom == 0) {
    // use memcpy to hopefully take advantage of standard library when zoom index is zero
    for (size_t l = 0; l < source_hpixel; l+=skip) {
      auto dest_index = (l*dest_wpixel)*3;
      auto source_index = (l*source_wpixel)*3;
      memcpy(((unsigned char *)dest_square.texture_array[zoom]->display_texture->pixels)+dest_index,
             ((unsigned char *)source_square.image_array[IMAGE_GRID_INDEX]->rgb_data)+source_index,
             sizeof(unsigned char)*source_wpixel*3);
    }
  } else {
    int ld=0;
    for (size_t l = 0; l < source_hpixel; l+=skip) {
      int kd=0;
      for (size_t k = 0; k < source_wpixel; k+=skip) {
        auto dest_index = (ld*dest_wpixel+kd)*3;
        auto source_index = (l*source_wpixel+k)*3;
        auto source_data=source_square.image_array[IMAGE_GRID_INDEX]->rgb_data;
        auto dest_array=dest_square.texture_array[zoom]->display_texture->pixels;
        ((unsigned char *)dest_array)[dest_index]=source_data[source_index];
        ((unsigned char *)dest_array)[dest_index+1]=source_data[source_index+1];
        ((unsigned char *)dest_array)[dest_index+2]=source_data[source_index+2];
        kd++;
      }
      ld++;
    }
  }
  SDL_UnlockSurface(dest_square.texture_array[zoom]->display_texture);
  return successful;
}

void TextureGrid::update_textures (ImageGrid *grid, float xgrid,
                                   float ygrid, int zoom_level,
                                   bool load_all) {
  DEBUG("update_textures() " << zoom_level << " | " << xgrid << " | " << ygrid);
  // j,i is better for plotting a grid
  for (size_t j = 0; j < textures_hgrid; j++) {
    MSGNONEWLINE("| ");
    // TODO: this will eventually be a switch statement to load in different things
    for (size_t i = 0; i < textures_wgrid; i++) {
      std::unique_lock<std::mutex> lock(squares[i][j].texture_array[zoom_level]->display_mutex, std::defer_lock);
      // skip everything if locked
      if (lock.try_lock()) {
        if (!load_all &&                                         \
            ((i < next_smallest(xgrid)) ||                       \
             (i > next_largest(xgrid)) ||                        \
             (j < next_smallest(ygrid)) ||                       \
             (j > next_largest(ygrid)))) {
          MSGNONEWLINE("0 ");
          if (squares[i][j].texture_array[zoom_level]->display_texture != nullptr) {
            SDL_FreeSurface(squares[i][j].texture_array[zoom_level]->display_texture);
            squares[i][j].texture_array[zoom_level]->display_texture = nullptr;
          }
        } else {
          MSGNONEWLINE("L ");
          squares[i][j].texture_wpixel=textures_max_wpixel;
          squares[i][j].texture_hpixel=textures_max_hpixel;
          if (squares[i][j].texture_array[zoom_level]->display_texture == nullptr) {
            load_texture(squares[i][j], grid->squares[i][j], zoom_level);
          }
        }
      }
    }
    MSGNONEWLINE("|" << std::endl);
  }
  DEBUG("update_textures() end");
}
