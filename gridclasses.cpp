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
  if (this->rgb_data != nullptr) {
    // this is problematic code, hence the debugging statements here
    DEBUG("++++++++++++++++++++ CLEARING PTR: " << (void *)this->rgb_data);
    delete[] this->rgb_data;
    this->rgb_data=nullptr;
    DEBUG("+++++++++ DONE PTR:");
  }
}

ImageGridSquare::ImageGridSquare() {
  this->image_array=new ImageGridSquareZoomLevel*[IMAGE_GRID_INDEX+1];
  for (auto i = 0; i < IMAGE_GRID_INDEX+1; i++) {
    this->image_array[i]=new ImageGridSquareZoomLevel();
  }
}

ImageGridSquare::~ImageGridSquare() {
  for (auto i = 0; i < IMAGE_GRID_INDEX+1; i++) {
    if (this->image_array[i] != nullptr) {
      delete this->image_array[i];
      this->image_array[i]=nullptr;
    }
  }
  if (this->image_array != nullptr) {
    delete[] this->image_array;
    this->image_array=nullptr;
  }
}

void ImageGridSquare::read_file (std::string filename) {
  if (check_tiff(filename)) {
    // TODO: check success
    DEBUG("Constructing TIFF: " << filename);
    read_tiff_data(filename,
                   this->image_array[IMAGE_GRID_INDEX]->rgb_wpixel,
                   this->image_array[IMAGE_GRID_INDEX]->rgb_hpixel);
    DEBUG("Done constructing TIff: " << filename << " " << this->image_array[IMAGE_GRID_INDEX]->rgb_wpixel << " " << this->image_array[IMAGE_GRID_INDEX]->rgb_hpixel);
  } else if (check_png(filename)) {
    DEBUG("Constructing PNG: " << filename);
    // TODO: check success
    read_png_data(filename,
                  this->image_array[IMAGE_GRID_INDEX]->rgb_wpixel,
                  this->image_array[IMAGE_GRID_INDEX]->rgb_hpixel);
    DEBUG("Done constructing PNG: " << filename << " " << this->image_array[IMAGE_GRID_INDEX]->rgb_wpixel << " " << this->image_array[IMAGE_GRID_INDEX]->rgb_hpixel);
  } else {
    ERROR("ImageGridSquare::load_file can't load: " << filename);
  }
}

void ImageGridSquare::load_file (std::string filename) {
  // block until things load
  std::lock_guard<std::mutex> guard(this->image_array[IMAGE_GRID_INDEX]->load_mutex);
  if (check_tiff(filename)) {
    DEBUG("Loading TIFF: " << filename);
    // TODO: check success
    load_tiff_as_rgb(filename,
                     this->image_array[IMAGE_GRID_INDEX]->rgb_wpixel,
                     this->image_array[IMAGE_GRID_INDEX]->rgb_hpixel,
                     &this->image_array[IMAGE_GRID_INDEX]->rgb_data);
    // printing pointer here
    DEBUG("++++++++++++++++++++ PTR: " << (void *)this->image_array[IMAGE_GRID_INDEX]->rgb_data);
    DEBUG("Done TIFF: " << filename);
  } else if (check_png(filename)) {
    DEBUG("Loading PNG: " << filename);
    // TODO: check success
    load_png_as_rgb(filename,
                    this->image_array[IMAGE_GRID_INDEX]->rgb_wpixel,
                    this->image_array[IMAGE_GRID_INDEX]->rgb_hpixel,
                    &this->image_array[IMAGE_GRID_INDEX]->rgb_data);
    DEBUG("Done PNG: " << filename);
  } else {
    ERROR("ImageGridSquare::load_file can't load: " << filename);
  }
  this->image_array[IMAGE_GRID_INDEX]->is_loaded=true;

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
    if (this->squares[i] != nullptr) {
      delete[] this->squares[i];
      this->squares[i]=nullptr;
    }
  }
  if (this->squares != nullptr) {
    delete[] this->squares;
    this->squares=nullptr;
  }
  if (this->image_max_size != nullptr) {
    delete this->image_max_size;
  }
}

bool ImageGrid::read_grid_info(GridSetup* grid_setup) {
  auto successful=true;
  if (grid_setup->path_value[0] != 0) {
    grid_setup->filenames=load_numbered_images(std::string(grid_setup->path_value));
  }
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
      MSG("Reading: " << grid_setup->filenames[ij]);
      squares[i][j].read_file(grid_setup->filenames[ij]);
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

bool ImageGrid::load_grid(GridSetup *grid_setup, std::atomic<bool> &keep_running) {
  auto successful=true;
  // TODO: sort out this when refactoring file loading
  for (INT_T i = 0; i < this->grid_image_size->wimage(); i++) {
    if (!successful) {
      continue;
    }
    for (INT_T j = 0; j < this->grid_image_size->himage(); j++) {
      if (!successful) {
        continue;
      }
      if (!keep_running) {
        successful=false;
      }
      auto ij=j*this->grid_image_size->wimage()+i;
      MSG("Loading: " << grid_setup->filenames[ij]);
      this->squares[i][j].load_file(grid_setup->filenames[ij]);
    }
  }
  return successful;
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
    if (this->texture_array[i] != nullptr) {
      delete this->texture_array[i];
      this->texture_array[i]=nullptr;
    }
  }
  if (this->texture_array != nullptr) {
    delete[] this->texture_array;
    this->texture_array=nullptr;
  }
  if (this->texture_pixel_size != nullptr) {
    delete this->texture_pixel_size;
    this->texture_pixel_size=nullptr;
  }
}

TextureGrid::TextureGrid (GridSetup *grid_setup) {
  this->grid_image_size=new GridImageSize(grid_setup->grid_image_size);
  this->squares = new TextureGridSquare*[grid_setup->grid_image_size->wimage()];
  for (INT_T i = 0; i < grid_setup->grid_image_size->wimage(); i++) {
    this->squares[i] = new TextureGridSquare[grid_setup->grid_image_size->himage()];
  }
}

TextureGrid::~TextureGrid() {
  for (auto i=0; i < this->grid_image_size->wimage(); i++) {
    if (this->squares[i] != nullptr) {
      delete[] this->squares[i];
      this->squares[i]=nullptr;
    }
  }
  if (this->squares != nullptr) {
    delete[] this->squares;
    this->squares=nullptr;
  }
  if (this->grid_image_size != nullptr) {
    delete this->grid_image_size;
    this->grid_image_size=nullptr;
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

bool TextureGrid::load_texture (TextureGridSquareZoomLevel *dest_square,
                                ImageGridSquareZoomLevel *source_square,
                                INT_T zoom_level,
                                INT_T wpixel,
                                INT_T hpixel) {
  auto successful=true;
  auto source_wpixel=(INT_T)source_square->rgb_wpixel;
  auto source_hpixel=(INT_T)source_square->rgb_hpixel;
  auto zoom_reduction=((INT_T)pow(2,zoom_level));
  auto dest_wpixel=wpixel/zoom_reduction;
  auto dest_hpixel=hpixel/zoom_reduction;
  dest_wpixel=dest_wpixel + (TEXTURE_ALIGNMENT - (dest_wpixel % TEXTURE_ALIGNMENT));
  dest_square->display_texture = SDL_CreateRGBSurfaceWithFormat(0,dest_wpixel,dest_hpixel,24,SDL_PIXELFORMAT_RGB24);
  // skip if can't load texture
  if (dest_square->display_texture) {
    auto lock_surface_return=SDL_LockSurface(dest_square->display_texture);
    if (lock_surface_return == 0) {
        auto skip = zoom_reduction;
        // does the thing we are copying exist?
        auto source_data=source_square->rgb_data;
        auto dest_array=dest_square->display_texture->pixels;
        if (dest_array != nullptr && source_data != nullptr) {
          if (zoom_level == 0) {
            // use memcpy to hopefully take advantage of standard library when zoom index is zero
            for (INT_T l = 0; l < source_hpixel; l+=skip) {
              auto dest_index = (l*dest_wpixel)*3;
              auto source_index = (l*source_wpixel)*3;
              memcpy(((unsigned char *)dest_array)+dest_index,
                     ((unsigned char *)source_data)+source_index,
                     sizeof(unsigned char)*source_wpixel*3);
            }
          } else {
            INT_T ld=0;
            for (INT_T l = 0; l < source_hpixel; l+=skip) {
              INT_T kd=0;
              for (INT_T k = 0; k < source_wpixel; k+=skip) {
                auto dest_index = (ld*dest_wpixel+kd)*3;
                auto source_index = (l*source_wpixel+k)*3;
                ((unsigned char *)dest_array)[dest_index]=source_data[source_index];
                ((unsigned char *)dest_array)[dest_index+1]=source_data[source_index+1];
                ((unsigned char *)dest_array)[dest_index+2]=source_data[source_index+2];
                kd++;
              }
              ld++;
            }
          }
        } else {
          DEBUG("Source or dest data is null in TextureGrid::load_texture");
        }
        SDL_UnlockSurface(dest_square->display_texture);
      } else {
        DEBUG("SDL_LockSurface failed with: " << lock_surface_return);
        dest_square->display_texture=nullptr;
        return false;
      }
  } else {
    DEBUG("SDL_CreateRGBSurfaceWithFormat failed with: " << SDL_GetError());
    return false;
  }
  return successful;
}

// TODO: the grid showing what's going on doesn't work in cases right now
void TextureGrid::update_textures (ImageGrid *grid,
                                   GridCoordinate* grid_coordinate,
                                   INT_T zoom_level,
                                   bool load_all) {
  DEBUG("update_textures() " << zoom_level << " | " << grid_coordinate->xgrid() << " | " << grid_coordinate->ygrid());
  // j,i is better for plotting a grid
  for (INT_T j = 0; j < this->grid_image_size->himage(); j++) {
    MSGNONEWLINE("| ");
    // TODO: this will eventually be a switch statement to load in different things
    for (INT_T i = 0; i < this->grid_image_size->wimage(); i++) {
      auto dest_square=this->squares[i][j].texture_array[zoom_level];
      std::unique_lock<std::mutex> display_lock(dest_square->display_mutex, std::defer_lock);
      // skip everything if locked
      if (display_lock.try_lock()) {
        // try and get the source square mutex
        if (!load_all &&
            ((i < next_smallest(grid_coordinate->xgrid())) ||
             (i > next_largest(grid_coordinate->xgrid())) ||
             (j < next_smallest(grid_coordinate->ygrid())) ||
             (j > next_largest(grid_coordinate->ygrid())))) {
          MSGNONEWLINE("0 ");
          if (dest_square->display_texture != nullptr) {
            SDL_FreeSurface(dest_square->display_texture);
            dest_square->display_texture = nullptr;
          }
        } else {
          auto image_square=grid->squares[i][j].image_array[IMAGE_GRID_INDEX];
          if(image_square->is_loaded) {
            std::unique_lock<std::mutex> load_lock(image_square->load_mutex, std::defer_lock);
            if(load_lock.try_lock()) {
              MSGNONEWLINE("L ");
              this->squares[i][j].texture_pixel_size=new GridPixelSize(this->max_pixel_size);
              if (dest_square->display_texture == nullptr) {
                this->load_texture(dest_square,
                                   image_square,
                                   zoom_level,
                                   this->squares[i][j].texture_pixel_size->wpixel(),
                                   this->squares[i][j].texture_pixel_size->hpixel());
              } else {
                DEBUG("Display texture is nullptr in TextureGrid::update_textures");
              }
            } else {
              DEBUG("Unable to aquire load_mutex in TextureGrid::update_textures corresponding to: " << i << " " << j);
            }
          } else {
            DEBUG("File not loaded according to is_loaded in TextureGrid::update_textures: " << i << " " << j);
          }
        }
        display_lock.unlock();
      } else {
        DEBUG("Unable to aquire display_mutex in TextureGrid::update_textures");
      }
    }
    MSGNONEWLINE("|" << std::endl);
  }
  DEBUG("update_textures() end");
}
