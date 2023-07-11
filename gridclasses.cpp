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
#include "iterators.hpp"
#include "viewport_current_state.hpp"
// C++ headers
#include <array>
#include <atomic>
#include <cmath>
#include <iostream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

ImageGridSquareZoomLevel::~ImageGridSquareZoomLevel() {
  this->unload_file();
}

bool ImageGridSquareZoomLevel::load_file(std::string filename, std::vector<ImageGridSquareZoomLevel*> dest_squares) {
  // block until things load
  // only block things actually being loaded
  auto load_successful=false;
  std::vector<std::pair<ImageGridSquareZoomLevel*,std::shared_ptr<LoadFileData>>> data_pairs;
  std::vector<std::shared_ptr<LoadFileData>> data_read;
  for (auto & dest_square : dest_squares) {
    data_pairs.emplace_back(std::pair<ImageGridSquareZoomLevel*,std::shared_ptr<LoadFileData>>(dest_square,std::make_shared<LoadFileData>()));
    data_pairs.back().second->zoom_level=dest_square->zoom_level;
    data_read.emplace_back(data_pairs.back().second);
  }
  DEBUG("Trying to load: " << filename);
  if (check_tiff(filename)) {
    MSG("Loading TIFF: " << filename);
    // TODO: check success
    load_tiff_as_rgb(filename,
                     data_read);
    // printing pointer here
    MSG("Done TIFF: " << filename);
    load_successful=true;
    } else if (check_png(filename)) {
      MSG("Loading PNG: " << filename);
      // TODO: check success
      load_png_as_rgb(filename,
                      data_read);
      MSG("Done PNG: " << filename);
      load_successful=true;
  } else {
    ERROR("ImageGridSquare::load_file can't load: " << filename);
  }
  if (load_successful) {
    for (auto & data_pair : data_pairs) {
      std::lock_guard<std::mutex> guard(data_pair.first->load_mutex);
      data_pair.first->rgb_wpixel=data_pair.second->rgb_wpixel;
      data_pair.first->rgb_hpixel=data_pair.second->rgb_hpixel;
      data_pair.first->rgb_data=data_pair.second->rgb_data;
      data_pair.first->is_loaded=true;
    }
  }
  return load_successful;
}

void ImageGridSquareZoomLevel::unload_file() {
  if (this->rgb_data != nullptr) {
    // this is problematic code, hence the debugging statements here
    if (this->is_loaded) {
      std::lock_guard<std::mutex> guard(this->load_mutex);
      delete[] this->rgb_data;
      this->rgb_data=nullptr;
      this->is_loaded=false;
    }
  }
}

ImageGridSquare::ImageGridSquare() {
  this->image_array=std::make_unique<ImageGridSquareZoomLevel*[]>(IMAGE_GRID_LENGTH);
  for (auto i=0ul; i < IMAGE_GRID_LENGTH; i++) {
    this->image_array[i]=new ImageGridSquareZoomLevel();
  }
}

ImageGridSquare::~ImageGridSquare() {
  for (auto i=0ul; i < IMAGE_GRID_LENGTH; i++) {
    DELETE_IF_NOT_NULLPTR(this->image_array[i]);
  }
}

void ImageGridSquare::read_file(std::string filename) {
  if (check_tiff(filename)) {
    // TODO: check success
    DEBUG("Constructing TIFF: " << filename);
    read_tiff_data(filename,
                   this->image_wpixel,
                   this->image_hpixel);
    DEBUG("Done constructing TIff: " << filename << " " << this->image_wpixel
                                                 << " " << this->image_hpixel);
  } else if (check_png(filename)) {
    DEBUG("Constructing PNG: " << filename);
    // TODO: check success
    read_png_data(filename,
                  this->image_wpixel,
                  this->image_hpixel);
    DEBUG("Done constructing PNG: " << filename << " " << this->image_wpixel
                                                << " " << this->image_hpixel);
  } else {
    ERROR("ImageGridSquare::read_file can't read: " << filename);
  }
}

ImageGrid::ImageGrid(GridSetup *grid_setup, std::shared_ptr<ViewPortCurrentState> viewport_current_state_imagegrid_update) {
  this->grid_image_size=GridImageSize(grid_setup->grid_image_size);
  this->squares=std::make_unique<ImageGridSquare*[]>(grid_setup->grid_image_size.wimage());
  this->_viewport_current_state_imagegrid_update=viewport_current_state_imagegrid_update;
  for (INT_T i=0l; i < grid_setup->grid_image_size.wimage(); i++) {
    this->squares[i]=new ImageGridSquare[grid_setup->grid_image_size.himage()];
  }
  this->image_max_size=GridPixelSize(0,0);
}

ImageGrid::~ImageGrid() {
  for (INT_T i=0l; i < this->grid_image_size.wimage(); i++) {
    DELETE_ARRAY_IF_NOT_NULLPTR(this->squares[i]);
  }
}

bool ImageGrid::read_grid_info(GridSetup* grid_setup) {
  auto successful=true;
  if (grid_setup->path_value[0] != 0) {
    grid_setup->filenames=load_numbered_images(std::string(grid_setup->path_value));
  }
  for (INT_T i=0l; i < this->grid_image_size.wimage(); i++) {
    if (!successful) {
      this->squares[i]=nullptr;
      continue;
    }
    for (INT_T j=0l; j < this->grid_image_size.himage(); j++) {
      if (!successful) {
        continue;
      }
      auto ij=j*this->grid_image_size.wimage()+i;
      MSG("Reading: " << grid_setup->filenames[ij]);
      this->squares[i][j].read_file(grid_setup->filenames[ij]);
      // set the RGB of the surface
      auto rgb_wpixel=this->squares[i][j].image_wpixel;
      auto rgb_hpixel=this->squares[i][j].image_hpixel;
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
  // find max zoom for each level
  // TODO: need a "viewport_setup" instead of defines this instead of global const
  auto image_max_size_wpixel=this->image_max_size.wpixel();
  auto image_max_size_hpixel=this->image_max_size.hpixel();
  // find the zoom level where screen fills grid square
  auto zoom_limit_w=(FLOAT_T)image_max_size_wpixel/(FLOAT_T)MAX_SCREEN_WIDTH;
  auto zoom_limit_h=(FLOAT_T)image_max_size_hpixel/(FLOAT_T)MAX_SCREEN_HEIGHT;
  // find zoom limit as rounded down power of 2
  DEBUG("====================");
  DEBUG(zoom_limit_w);
  DEBUG(zoom_limit_h);
  DEBUG(round_down_power_of_2(zoom_limit_w));
  DEBUG(round_down_power_of_2(zoom_limit_h));
  auto zoom_limit=fmin(round_down_power_of_2(zoom_limit_w),
                       round_down_power_of_2(zoom_limit_h));
  // add this info to the various data structure
  this->zoom_step=zoom_limit;
  for (INT_T i=0l; i < this->grid_image_size.wimage(); i++) {
    for (INT_T j=0l; j < this->grid_image_size.himage(); j++) {
      INT_T local_zoom_level=1;
      for (auto k=0ul; k < IMAGE_GRID_LENGTH; k++) {
        this->squares[i][j].image_array[k]->zoom_level=local_zoom_level;
        this->squares[i][j].image_array[k]->max_zoom_level=local_zoom_level*zoom_limit;
        DEBUG("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
        DEBUG("LOAD: " << " " << i << " " << " " << j << " " << k << " " << " " << zoom_limit << " " << local_zoom_level << " " << local_zoom_level*zoom_limit);
        local_zoom_level*=zoom_limit;
      }
    }
  }
  return successful;
}

bool ImageGrid::_check_bounds(INT_T i, INT_T j) {
  if (i >= 0 && i < this->grid_image_size.wimage() && j >= 0 && j < this->grid_image_size.himage()) {
    return true;
  } else {
    return false;
  }
}

bool ImageGrid::_check_load(INT_T k, INT_T i, INT_T j, INT_T current_load_zoom, INT_T current_grid_x, INT_T current_grid_y, INT_T load_all) {
  if ((k == IMAGE_GRID_LENGTH-1 ||
      (i >= current_grid_x-current_load_zoom && i <= current_grid_x+current_load_zoom &&
       j >= current_grid_y-current_load_zoom && j <= current_grid_y+current_load_zoom) ||
       load_all)) {
    return true;
  } else {
    return false;
  }
}

bool ImageGrid::_load_file(INT_T i, INT_T j, INT_T current_grid_x, INT_T current_grid_y, INT_T load_all, GridSetup *grid_setup) {
  // decide whether to load
  // always load if top level
  bool load_successful=false;
  if (this->_check_bounds(i, j)) {
    std::vector<INT_T> zoom_load_list;
    // build up zoom
    for (INT_T k=IMAGE_GRID_LENGTH-1; k >= 0l; k--) {
      auto current_load_zoom=this->squares[i][j].image_array[k]->zoom_level;
      if (this->_check_load(k, i, j, current_load_zoom, current_grid_x, current_grid_y, load_all)) {
        if (!this->squares[i][j].image_array[k]->is_loaded) {
          zoom_load_list.push_back(k);
        }
      }
    }
    if (zoom_load_list.size() > 0) {
      auto dest_squares=std::vector<ImageGridSquareZoomLevel*>{};
      for (auto & zoom_load : zoom_load_list) {
        dest_squares.push_back(this->squares[i][j].image_array[zoom_load]);
      }
      auto ij=j*this->grid_image_size.wimage()+i;
      load_successful=ImageGridSquareZoomLevel::load_file(grid_setup->filenames[ij],dest_squares);
    }
  }
  return load_successful;
}

void ImageGrid::load_grid(GridSetup *grid_setup, std::atomic<bool> &keep_running) {
  auto keep_trying=true;
  // get location of viewport
  FLOAT_T zoom;
  auto view_changed=this->_viewport_current_state_imagegrid_update->GetGridValues(zoom,this->_viewport_grid);
  // find the grid extents and choose things that should be loaded/unloaded
  // different things for what should be loaded/unloaded
  // load in reverse order of size
  auto current_grid_x=(INT_T)floor(this->_viewport_grid.xgrid());
  auto current_grid_y=(INT_T)floor(this->_viewport_grid.ygrid());
  auto grid_w=this->grid_image_size.wimage();
  auto grid_h=this->grid_image_size.himage();
  auto load_all=false;
  // files actually loaded
  INT_T load_count=0;
  // unload first
  for (auto k=IMAGE_GRID_LENGTH-1; k >= 0l; k--) {
    for (INT_T i=0l; i < grid_w; i++) {
      for (INT_T j=0l; j < grid_h; j++) {
        if (!keep_running) {
          keep_trying=false;
        }
        auto current_load_zoom=this->squares[i][j].image_array[k]->zoom_level;
        if (!this->_check_load(k, i, j, current_load_zoom, current_grid_x, current_grid_y, load_all)) {
          this->squares[i][j].image_array[k]->unload_file();
        }
      }
    }
  }
  auto iterator=ImageGridIterator(this->grid_image_size.wimage(), this->grid_image_size.himage(), current_grid_x, current_grid_y);
  // TODO need a good iterator class for this type of work
  // load the one we are looking at
  INT_T i,j;
  while (keep_trying) {
    keep_trying=iterator.get_next(i,j);
    if (!keep_running) { keep_trying=false; }
    if (keep_trying) {
      auto load_successful=this->_load_file(i, j, current_grid_x, current_grid_y, load_all, grid_setup);
      if (load_successful) { load_count++; }
      if (load_count >= LOAD_FILES_BATCH) { keep_trying=false; }
    }
  }
}

GridPixelSize ImageGrid::get_image_max_pixel_size() {
  return this->image_max_size;

}

TextureGridSquareZoomLevel::~TextureGridSquareZoomLevel () {
  // crashes for some reason, need to work this out, probably some other SDL deinitialization happening
  // this->unload_texture();
}

void TextureGridSquareZoomLevel::unload_texture () {
  if (this->display_texture != nullptr) {
    SDL_FreeSurface(this->display_texture);
    this->display_texture=nullptr;
    this->last_load_index=INT_MAX;
  }
}


TextureGridSquare::TextureGridSquare () {
  this->texture_array=std::make_unique<TextureGridSquareZoomLevel*[]>(MAX_TEXTURE_ZOOM_LEVELS);
  for (auto i=0ul; i < MAX_TEXTURE_ZOOM_LEVELS; i++) {
    this->texture_array[i]=new TextureGridSquareZoomLevel();
  }
}

TextureGridSquare::~TextureGridSquare () {
  for (auto i=0ul; i < MAX_TEXTURE_ZOOM_LEVELS; i++) {
    DELETE_IF_NOT_NULLPTR(this->texture_array[i]);
  }
}

TextureGrid::TextureGrid (GridSetup *grid_setup) {
  this->grid_image_size=GridImageSize(grid_setup->grid_image_size);
  this->squares=std::make_unique<TextureGridSquare*[]>(grid_setup->grid_image_size.wimage());
  for (INT_T i=0l; i < grid_setup->grid_image_size.wimage(); i++) {
    this->squares[i]=new TextureGridSquare[grid_setup->grid_image_size.himage()];
  }
}

TextureGrid::~TextureGrid() {
  for (auto i=0l; i < this->grid_image_size.wimage(); i++) {
    DELETE_ARRAY_IF_NOT_NULLPTR(this->squares[i]);
  }
}

void TextureGrid::init_max_zoom_index(const GridPixelSize &image_max_pixel_size) {
  INT_T zoom_length=0;
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
