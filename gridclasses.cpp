/**
 * Implementation of the main classes representing the grid.  Includes
 * both loaded images and (zoomed) textures.
 */
// local headers
#include "debug.hpp"
#include "error.hpp"
#include "types.hpp"
#include "utility.hpp"
#include "fileload.hpp"
#include "gridsetup.hpp"
#include "gridclasses.hpp"
// C++ headers
#include <atomic>
#include <cmath>
#include <iostream>
#include <string>

ImageGridSquareZoomLevel::~ImageGridSquareZoomLevel() {
  if(this->rgb_data != nullptr) {
    // this is problematic code, hence the debugging statements here
    DEBUG("++++++++++++++++++++ CLEARING PTR: " << (void *)this->rgb_data);
    delete[] this->rgb_data;
    this->rgb_data=nullptr;
    DEBUG("+++++++++ DONE PTR:");
  }
}

ImageGridSquare::ImageGridSquare() {
  this->image_array=new ImageGridSquareZoomLevel*[IMAGE_GRID_LENGTH];
  for(auto i = 0; i < IMAGE_GRID_LENGTH; i++) {
    this->image_array[i]=new ImageGridSquareZoomLevel();
  }
}

ImageGridSquare::~ImageGridSquare() {
  for(auto i = 0; i < IMAGE_GRID_LENGTH; i++) {
    DELETE_IF_NOT_NULLPTR(this->image_array[i]);
  }
  DELETE_ARRAY_IF_NOT_NULLPTR(this->image_array);
}

void ImageGridSquare::read_file(std::string filename) {
  if(check_tiff(filename)) {
    // TODO: check success
    DEBUG("Constructing TIFF: " << filename);
    read_tiff_data(filename,
                   this->image_array[IMAGE_GRID_BASE_INDEX]->rgb_wpixel,
                   this->image_array[IMAGE_GRID_BASE_INDEX]->rgb_hpixel);
    DEBUG("Done constructing TIff: " << filename << " " << this->image_array[IMAGE_GRID_BASE_INDEX]->rgb_wpixel
                                                 << " " << this->image_array[IMAGE_GRID_BASE_INDEX]->rgb_hpixel);
  } else if(check_png(filename)) {
    DEBUG("Constructing PNG: " << filename);
    // TODO: check success
    read_png_data(filename,
                  this->image_array[IMAGE_GRID_BASE_INDEX]->rgb_wpixel,
                  this->image_array[IMAGE_GRID_BASE_INDEX]->rgb_hpixel);
    DEBUG("Done constructing PNG: " << filename << " " << this->image_array[IMAGE_GRID_BASE_INDEX]->rgb_wpixel
                                                << " " << this->image_array[IMAGE_GRID_BASE_INDEX]->rgb_hpixel);
  } else {
    ERROR("ImageGridSquare::load_file can't load: " << filename);
  }
}

void ImageGridSquare::load_file(std::string filename) {
  // block until things load
  // only block things actually being loaded
  std::lock_guard<std::mutex> guard(this->image_array[IMAGE_GRID_BASE_INDEX]->load_mutex);
  std::lock_guard<std::mutex> guard_second(this->image_array[IMAGE_GRID_SECOND_INDEX]->load_mutex);
  std::lock_guard<std::mutex> guard_thumb(this->image_array[IMAGE_GRID_THUMB_INDEX]->load_mutex);
  if(check_tiff(filename)) {
    DEBUG("Loading TIFF: " << filename);
    // TODO: check success
    load_tiff_as_rgb(filename,
                     this->image_array[IMAGE_GRID_BASE_INDEX]->rgb_wpixel,
                     this->image_array[IMAGE_GRID_BASE_INDEX]->rgb_hpixel,
                     &this->image_array[IMAGE_GRID_BASE_INDEX]->rgb_data);
    // printing pointer here
    DEBUG("++++++++++++++++++++ PTR: " << (void *)this->image_array[IMAGE_GRID_BASE_INDEX]->rgb_data);
    DEBUG("Done TIFF: " << filename);
  } else if(check_png(filename)) {
    DEBUG("Loading PNG: " << filename);
    // TODO: check success
    load_png_as_rgb(filename,
                    this->image_array[IMAGE_GRID_BASE_INDEX]->rgb_wpixel,
                    this->image_array[IMAGE_GRID_BASE_INDEX]->rgb_hpixel,
                    &this->image_array[IMAGE_GRID_BASE_INDEX]->rgb_data);
    DEBUG("Done PNG: " << filename);
  } else {
    ERROR("ImageGridSquare::load_file can't load: " << filename);
  }
  this->image_array[IMAGE_GRID_BASE_INDEX]->is_loaded=true;

}

ImageGrid::ImageGrid(GridSetup *grid_setup) {
  this->grid_image_size=GridImageSize(grid_setup->grid_image_size);
  this->squares = new ImageGridSquare*[grid_setup->grid_image_size.wimage()];
  for (INT_T i = 0; i < grid_setup->grid_image_size.wimage(); i++) {
    this->squares[i] = new ImageGridSquare[grid_setup->grid_image_size.himage()];
  }
  this->image_max_size=GridPixelSize(0,0);
}

ImageGrid::~ImageGrid() {
  for (INT_T i = 0; i < this->grid_image_size.wimage(); i++) {
    DELETE_ARRAY_IF_NOT_NULLPTR(this->squares[i]);
  }
  DELETE_ARRAY_IF_NOT_NULLPTR(this->squares);
}

bool ImageGrid::read_grid_info(GridSetup* grid_setup) {
  auto successful=true;
  if (grid_setup->path_value[0] != 0) {
    grid_setup->filenames=load_numbered_images(std::string(grid_setup->path_value));
  }
  for (INT_T i = 0; i < this->grid_image_size.wimage(); i++) {
    if (!successful) {
      squares[i]=nullptr;
      continue;
    }
    for (INT_T j = 0; j < this->grid_image_size.himage(); j++) {
      if (!successful) {
        continue;
      }
      auto ij=j*this->grid_image_size.wimage()+i;
      MSG("Reading: " << grid_setup->filenames[ij]);
      squares[i][j].read_file(grid_setup->filenames[ij]);
      // set the RGB of the surface
      auto rgb_wpixel=squares[i][j].image_array[IMAGE_GRID_BASE_INDEX]->rgb_wpixel;
      auto rgb_hpixel=squares[i][j].image_array[IMAGE_GRID_BASE_INDEX]->rgb_hpixel;
      auto max_wpixel=this->image_max_size.wpixel();
      auto max_hpixel=this->image_max_size.hpixel();
      // TODO: encapsulate calculation of max pixels
      INT_T new_wpixel, new_hpixel;
      if ((INT_T)rgb_wpixel > max_wpixel) {
        new_wpixel=(INT_T)(rgb_wpixel+(TEXTURE_ALIGNMENT - (rgb_wpixel % TEXTURE_ALIGNMENT)));
      } else {
        new_wpixel=max_wpixel;
      }
      if ((INT_T)rgb_hpixel > max_hpixel) {
        new_hpixel=(INT_T)rgb_hpixel;
      } else {
        new_hpixel=max_hpixel;
      }
      this->image_max_size=GridPixelSize(new_wpixel,new_hpixel);;
      // calculate the second and thumbnail images

    }
  }
  return successful;
}

bool ImageGrid::load_grid(GridSetup *grid_setup, std::atomic<bool> &keep_running) {
  auto successful=true;
  // TODO: sort out this when refactoring file loading
  for (INT_T i = 0; i < this->grid_image_size.wimage(); i++) {
    if (!successful) {
      continue;
    }
    for (INT_T j = 0; j < this->grid_image_size.himage(); j++) {
      if (!successful) {
        continue;
      }
      if (!keep_running) {
        successful=false;
      }
      auto ij=j*this->grid_image_size.wimage()+i;
      MSG("Loading: " << grid_setup->filenames[ij]);
      this->squares[i][j].load_file(grid_setup->filenames[ij]);
    }
  }
  return successful;
}

GridPixelSize ImageGrid::get_image_max_pixel_size() {
  return this->image_max_size;

}

TextureGridSquareZoomLevel::~TextureGridSquareZoomLevel () {
  // not freeing SDL... causes a crash when exiting
  // if (this->display_texture != nullptr) {
  //   SDL_FreeSurface(this->display_texture);
  //   this->display_texture=nullptr;
  // }
}

TextureGridSquare::TextureGridSquare () {
  this->texture_array=new TextureGridSquareZoomLevel*[MAX_ZOOM_LEVELS]();
  for (auto i = 0; i < MAX_ZOOM_LEVELS; i++) {
    this->texture_array[i]=new TextureGridSquareZoomLevel();
  }
}

TextureGridSquare::~TextureGridSquare () {
  for (auto i = 0; i < MAX_ZOOM_LEVELS; i++) {
    DELETE_IF_NOT_NULLPTR(this->texture_array[i]);
  }
  DELETE_ARRAY_IF_NOT_NULLPTR(this->texture_array)
}

TextureGrid::TextureGrid (GridSetup *grid_setup) {
  this->grid_image_size=GridImageSize(grid_setup->grid_image_size);
  this->squares = new TextureGridSquare*[grid_setup->grid_image_size.wimage()];
  for (INT_T i = 0; i < grid_setup->grid_image_size.wimage(); i++) {
    this->squares[i] = new TextureGridSquare[grid_setup->grid_image_size.himage()];
  }
}

TextureGrid::~TextureGrid() {
  for (auto i=0; i < this->grid_image_size.wimage(); i++) {
    DELETE_ARRAY_IF_NOT_NULLPTR(this->squares[i]);
  }
  DELETE_ARRAY_IF_NOT_NULLPTR(this->squares);
  // DELETE_IF_NOT_NULLPTR(this->grid_image_size);
}

void TextureGrid::init_max_zoom_index(const GridPixelSize &image_max_pixel_size) {
  INT_T zoom_length = 0;
  FLOAT_T current_zoom=1.0;
  // auto max_wpixel=grid->image_max_size.wpixel();
  // auto max_hpixel=grid->image_max_size.hpixel();
  // swap out and reallocate
  this->max_pixel_size=GridPixelSize(image_max_pixel_size);
  zoom_length += 1;
  current_zoom /= 2.0;
  // TODO: inefficient, do without a while loop
  while (!(((this->grid_image_size.wimage()*this->max_pixel_size.wpixel()*current_zoom) < MAX_SCREEN_WIDTH) &&
           ((this->grid_image_size.himage()*this->max_pixel_size.hpixel()*current_zoom) < MAX_SCREEN_HEIGHT))) {
    zoom_length += 1;
    current_zoom /= 2.0;
  }
  this->textures_max_zoom_index=zoom_length-1;
}
