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
#include <cmath>
#include <iostream>
#include <string>

ImageGridSquareZoomLevel::~ImageGridSquareZoomLevel() {
  delete[] this->rgb_data;
}

ImageGridSquare::ImageGridSquare() {
  this->image_array=new ImageGridSquareZoomLevel*[IMAGE_GRID_INDEX+1];
  for (auto i = 0; i < IMAGE_GRID_INDEX+1; i++) {
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
  for (auto i = 0; i < IMAGE_GRID_INDEX+1; i++) {
    delete[] this->image_array[i];
    this->image_array[i]=nullptr;
  }
  delete[] this->image_array;
  this->image_array=nullptr;
}

ImageGrid::ImageGrid(GridSetup *grid_setup) {
  this->grid_image_size =new GridImageSize(grid_setup->grid_image_size);
  this->squares = new ImageGridSquare*[grid_setup->grid_image_size->wimage()];
  for (INT_T i = 0; i < grid_setup->grid_image_size->wimage(); i++) {
    this->squares[i] = new ImageGridSquare[grid_setup->grid_image_size->himage()];
  }
  this->image_max_size=new GridPixelSize(0,0);
}

ImageGrid::~ImageGrid() {
  for (INT_T i = 0; i < this->grid_image_size->wimage(); i++) {
    delete[] this->squares[i];
    this->squares[i]=nullptr;
  }
  delete[] this->squares;
  this->squares=nullptr;
}

bool ImageGrid::load_images(GridSetup *grid_setup) {
  std::vector<std::string> filenames;
  if (grid_setup->path_value[0] != 0) {
    DEBUG("load_image_grid() with w: " << grid_setup->grid_image_size->wimage << " h: " << grid_setup->grid_image_size->himage << " " << grid_setup->path_value);
    std::vector<std::string> filenames;
    filenames=load_numbered_images(std::string(grid_setup->path_value));
    return load_grid(filenames);
  } else {
    return load_grid(grid_setup->filenames);
  }
}

bool ImageGrid::load_grid(std::vector<std::string> filenames) {
  auto successful=true;
  // TODO: sort out this when refactoring file loading
  for (INT_T i = 0; i < this->grid_image_size->wimage(); i++) {
    if (!successful) {
      squares[i]=nullptr;
      continue;
    }
    for (INT_T j = 0; j < this->grid_image_size->himage(); j++) {
      if (!successful) {
        continue;
      }
      auto ij=j*this->grid_image_size->wimage()+i;
      MSG("Loading: " << filenames[ij]);
      squares[i][j].load_file(filenames[ij]);
      // set the RGB of the surface
      auto rgb_wpixel=squares[i][j].image_array[IMAGE_GRID_INDEX]->rgb_wpixel;
      auto rgb_hpixel=squares[i][j].image_array[IMAGE_GRID_INDEX]->rgb_hpixel;
      auto max_wpixel=this->image_max_size->wpixel();
      auto max_hpixel=this->image_max_size->hpixel();
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
      this->image_max_size = new GridPixelSize(new_wpixel,new_hpixel);;
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
  for (auto i = 0; i < MAX_ZOOM_LEVELS; i++) {
    this->texture_array[i]=new TextureGridSquareZoomLevel();
  }
}

TextureGridSquare::~TextureGridSquare () {
  for (auto i = 0; i < MAX_ZOOM_LEVELS; i++) {
    delete this->texture_array[i];
    this->texture_array[i]=nullptr;
  }
  delete[] this->texture_array;
  this->texture_array=nullptr;
}

TextureGrid::TextureGrid (GridSetup *grid_setup) {
  this->grid_image_size=new GridImageSize(grid_setup->grid_image_size);
  squares = new TextureGridSquare*[grid_setup->grid_image_size->wimage()];
  for (INT_T i = 0; i < grid_setup->grid_image_size->wimage(); i++) {
    squares[i] = new TextureGridSquare[grid_setup->grid_image_size->himage()];
  }
}

void TextureGrid::init_max_zoom_index(ImageGrid *grid) {
  INT_T zoom_length = 0;
  FLOAT_T current_zoom=1.0;
  auto max_wpixel=grid->image_max_size->wpixel();
  auto max_hpixel=grid->image_max_size->hpixel();
  // swap out and reallocate
  this->max_pixel_size = new GridPixelSize(grid->image_max_size);
  zoom_length += 1;
  current_zoom /= 2.0;
  // TODO: inefficient, do without a while loop
  while (!(((this->grid_image_size->wimage()*max_wpixel*current_zoom) < MAX_SCREEN_WIDTH) &&
           ((this->grid_image_size->himage()*max_hpixel*current_zoom) < MAX_SCREEN_HEIGHT))) {
    zoom_length += 1;
    current_zoom /= 2.0;
  }
  this->textures_max_zoom_index=zoom_length-1;
}

bool TextureGrid::load_texture (TextureGridSquare &dest_square,
                                ImageGridSquare &source_square,
                                INT_T zoom_level) {
  auto successful=true;
  auto source_wpixel=(INT_T)source_square.image_array[IMAGE_GRID_INDEX]->rgb_wpixel;
  auto source_hpixel=(INT_T)source_square.image_array[IMAGE_GRID_INDEX]->rgb_hpixel;
  auto zoom_reduction=((INT_T)pow(2,zoom_level));
  auto dest_wpixel=(dest_square.texture_pixel_size->wpixel())/zoom_reduction;
  auto dest_hpixel=(dest_square.texture_pixel_size->hpixel())/zoom_reduction;
  dest_wpixel=dest_wpixel + (TEXTURE_ALIGNMENT - (dest_wpixel % TEXTURE_ALIGNMENT));
  dest_square.texture_array[zoom_level]->display_texture = SDL_CreateRGBSurfaceWithFormat(0,dest_wpixel,dest_hpixel,24,SDL_PIXELFORMAT_RGB24);
  SDL_LockSurface(dest_square.texture_array[zoom_level]->display_texture);
  auto skip = zoom_reduction;
  if (zoom_level == 0) {
    // use memcpy to hopefully take advantage of standard library when zoom index is zero
    for (INT_T l = 0; l < source_hpixel; l+=skip) {
      auto dest_index = (l*dest_wpixel)*3;
      auto source_index = (l*source_wpixel)*3;
      memcpy(((unsigned char *)dest_square.texture_array[zoom_level]->display_texture->pixels)+dest_index,
             ((unsigned char *)source_square.image_array[IMAGE_GRID_INDEX]->rgb_data)+source_index,
             sizeof(unsigned char)*source_wpixel*3);
    }
  } else {
    INT_T ld=0;
    for (INT_T l = 0; l < source_hpixel; l+=skip) {
      INT_T kd=0;
      for (INT_T k = 0; k < source_wpixel; k+=skip) {
        auto dest_index = (ld*dest_wpixel+kd)*3;
        auto source_index = (l*source_wpixel+k)*3;
        auto source_data=source_square.image_array[IMAGE_GRID_INDEX]->rgb_data;
        auto dest_array=dest_square.texture_array[zoom_level]->display_texture->pixels;
        ((unsigned char *)dest_array)[dest_index]=source_data[source_index];
        ((unsigned char *)dest_array)[dest_index+1]=source_data[source_index+1];
        ((unsigned char *)dest_array)[dest_index+2]=source_data[source_index+2];
        kd++;
      }
      ld++;
    }
  }
  SDL_UnlockSurface(dest_square.texture_array[zoom_level]->display_texture);
  return successful;
}

// TODO: the grid showing what's going on doesn't work in cases right now
void TextureGrid::update_textures (ImageGrid *grid,
                                   GridCoordinate* grid_coordinate,
                                   INT_T zoom_level,
                                   bool load_all) {
  DEBUG("update_textures() " << zoom_level << " | " << grid_coordinate->xgrid << " | " << grid_coordinate->ygrid);
  // j,i is better for plotting a grid
  for (INT_T j = 0; j < this->grid_image_size->himage(); j++) {
    MSGNONEWLINE("| ");
    // TODO: this will eventually be a switch statement to load in different things
    for (INT_T i = 0; i < this->grid_image_size->wimage(); i++) {
      std::unique_lock<std::mutex> lock(squares[i][j].texture_array[zoom_level]->display_mutex, std::defer_lock);
      // skip everything if locked
      if (lock.try_lock()) {
        if (!load_all &&
            ((i < next_smallest(grid_coordinate->xgrid())) ||
             (i > next_largest(grid_coordinate->xgrid())) ||
             (j < next_smallest(grid_coordinate->ygrid())) ||
             (j > next_largest(grid_coordinate->ygrid())))) {
          MSGNONEWLINE("0 ");
          if (squares[i][j].texture_array[zoom_level]->display_texture != nullptr) {
            SDL_FreeSurface(squares[i][j].texture_array[zoom_level]->display_texture);
            squares[i][j].texture_array[zoom_level]->display_texture = nullptr;
          }
        } else {
          MSGNONEWLINE("L ");
          squares[i][j].texture_pixel_size=new GridPixelSize(this->max_pixel_size);
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
