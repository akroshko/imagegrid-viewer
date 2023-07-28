/**
 * Implementation of the main classes representing the grid.  Includes
 * both loaded images and (zoomed) textures.
 */
// local headers
#include "debug.hpp"
#include "error.hpp"
#include "types.hpp"
#include "utility.hpp"
#include "gridsetup.hpp"
#include "imagegrid.hpp"
#include "iterators.hpp"
#include "viewport_current_state.hpp"
// C compatible headers
#include "c_compatible/fileload.hpp"
// C++ headers
#include <array>
#include <atomic>
#include <cmath>
#include <iostream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

ImageGridSquareZoomLevel::ImageGridSquareZoomLevel(INT_T zoom_index) {
  this->_zoom_index=zoom_index;
}

ImageGridSquareZoomLevel::~ImageGridSquareZoomLevel() {
  this->unload_file();
}

bool ImageGridSquareZoomLevel::load_file(std::string filename, std::vector<ImageGridSquareZoomLevel*> dest_squares) {
  // block until things load
  // only block things actually being loaded
  auto load_successful=false;
  std::vector<std::pair<ImageGridSquareZoomLevel* const,std::shared_ptr<LoadFileData>>> data_pairs;
  std::vector<std::shared_ptr<LoadFileData>> data_read;
  for (auto & dest_square : dest_squares) {
    data_pairs.emplace_back(std::pair<ImageGridSquareZoomLevel* const,std::shared_ptr<LoadFileData>>(dest_square,std::make_shared<LoadFileData>()));
    data_pairs.back().second->zoom_index=dest_square->zoom_index();
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
      data_pair.first->_rgb_wpixel=data_pair.second->rgb_wpixel;
      data_pair.first->_rgb_hpixel=data_pair.second->rgb_hpixel;
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

INT_T ImageGridSquareZoomLevel::zoom_index() const {
  return this->_zoom_index;
}

size_t ImageGridSquareZoomLevel::rgb_wpixel() const {
  return this->_rgb_wpixel;
}

size_t ImageGridSquareZoomLevel::rgb_hpixel() const {
  return this->_rgb_hpixel;
}


ImageGridSquare::ImageGridSquare(std::string filename) {
  // TODO: this is the place
  MSG("Reading: " << filename);
  this->_read_file(filename);
}

ImageGridSquare::~ImageGridSquare() {
  // TODO: want better way to do this
  for (auto i=0l; i < this->_zoom_step_number; i++) {
    DELETE_IF_NOT_NULLPTR(this->image_array[i]);
  }
}

void ImageGridSquare::_read_file(std::string filename) {
  if (check_tiff(filename)) {
    // TODO: check success
    DEBUG("Constructing TIFF: " << filename);
    read_tiff_data(filename,
                   this->_image_wpixel,
                   this->_image_hpixel);
    DEBUG("Done constructing Tiff: " << filename << " " << this->image_wpixel()
                                                 << " " << this->image_hpixel());
  } else if (check_png(filename)) {
    DEBUG("Constructing PNG: " << filename);
    // TODO: check success
    read_png_data(filename,
                  this->_image_wpixel,
                  this->_image_hpixel);
    DEBUG("Done constructing PNG: " << filename << " " << this->image_wpixel()
                                                << " " << this->image_hpixel());
  } else {
    ERROR("ImageGridSquare::read_file can't read: " << filename);
  }
}

INT_T ImageGridSquare::image_wpixel() const {
  return this->_image_wpixel;
}

INT_T ImageGridSquare::image_hpixel() const {
  return this->_image_hpixel;
}

ImageGrid::~ImageGrid() {
  for (INT_T i=0l; i < this->_grid_image_size.wimage(); i++) {
    for (INT_T j=0l; j < this->_grid_image_size.himage(); j++) {
      DELETE_IF_NOT_NULLPTR(this->squares[i][j]);
    }
    DELETE_ARRAY_IF_NOT_NULLPTR(this->squares[i]);
  }
}

void ImageGrid::read_grid_info(const GridSetup* const grid_setup, std::shared_ptr<ViewPortCurrentState> viewport_current_state_imagegrid_update) {
  this->_grid_image_size=GridImageSize(grid_setup->grid_image_size());
  this->squares=std::make_unique<ImageGridSquare**[]>(grid_setup->grid_image_size().wimage());
  this->_viewport_current_state_imagegrid_update=viewport_current_state_imagegrid_update;
  for (INT_T i=0l; i < grid_setup->grid_image_size().wimage(); i++) {
    this->squares[i]=new ImageGridSquare*[grid_setup->grid_image_size().himage()];
  }
  this->_image_max_size=GridPixelSize(0,0);
  // now actually intialize squares
  for (INT_T i=0l; i < this->_grid_image_size.wimage(); i++) {
    for (INT_T j=0l; j < this->_grid_image_size.himage(); j++) {
      auto ij=j*this->_grid_image_size.wimage()+i;
      this->squares[i][j]=new ImageGridSquare(grid_setup->filenames()[ij]);
      // set the RGB of the surface
      auto rgb_wpixel=this->squares[i][j]->image_wpixel();
      auto rgb_hpixel=this->squares[i][j]->image_hpixel();
      auto max_wpixel=this->_image_max_size.wpixel();
      auto max_hpixel=this->_image_max_size.hpixel();
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
      this->_image_max_size=GridPixelSize(new_wpixel,new_hpixel);;
      // calculate the second and thumbnail images
    }
  }
  // find max zoom for each level
  auto image_max_size_wpixel=this->_image_max_size.wpixel();
  auto image_max_size_hpixel=this->_image_max_size.hpixel();
  auto zoom_step=2;
  // find how many zoom_step to get whole image grid as a 3x3 grid of original size
  // always have a top level that fits in 32x32
  auto max_scale=(INT_T)ceil((FLOAT_T)(fmax(image_max_size_wpixel,image_max_size_hpixel))/(FLOAT_T)MAX_MIN_SCALED_IMAGE_SIZE);
  this->_zoom_step_number=(INT_T)ceil(log2(max_scale));
  if (this->_zoom_step_number < 1) {
    this->_zoom_step_number=1;
  }
  // TODO: a quick hack to avoid aliasing
  this->_zoom_step_number+=1;
  MSG("zoom_step: " << zoom_step);
  MSG("max_scale: " << max_scale);
  MSG("zoom_step_number: " << this->_zoom_step_number);
  // add this info to the various data structure
  for (INT_T i=0l; i < this->_grid_image_size.wimage(); i++) {
    for (INT_T j=0l; j < this->_grid_image_size.himage(); j++) {
      this->squares[i][j]->image_array=std::make_unique<ImageGridSquareZoomLevel*[]>(this->_zoom_step_number);
      INT_T local_zoom_index=1;
      this->squares[i][j]->_zoom_step_number=this->_zoom_step_number;
      for (auto k=0l; k < this->_zoom_step_number; k++) {
        this->squares[i][j]->image_array[k]=new ImageGridSquareZoomLevel(local_zoom_index);
        local_zoom_index*=zoom_step;
      }
    }
  }
  // actually use this
  this->_read_grid_info_successful=true;
}

bool ImageGrid::read_grid_info_successful() const {
  return this->_read_grid_info_successful;
}

INT_T ImageGrid::zoom_step_number() const {
  return this->_zoom_step_number;
}

bool ImageGrid::_check_bounds(INT_T i, INT_T j) {
  if (i >= 0 && i < this->_grid_image_size.wimage() && j >= 0 && j < this->_grid_image_size.himage()) {
    return true;
  } else {
    return false;
  }
}

bool ImageGrid::_check_load(INT_T k, INT_T i, INT_T j, INT_T current_load_zoom, INT_T current_grid_x, INT_T current_grid_y, INT_T load_all) {
  if ((k == this->_zoom_step_number-1 ||
      (i >= current_grid_x-current_load_zoom && i <= current_grid_x+current_load_zoom &&
       j >= current_grid_y-current_load_zoom && j <= current_grid_y+current_load_zoom) ||
       load_all)) {
    return true;
  } else {
    return false;
  }
}

bool ImageGrid::_load_file(INT_T i, INT_T j,
                           INT_T current_grid_x, INT_T current_grid_y,
                           INT_T zoom_index_lower_limit,
                           INT_T load_all, const GridSetup* const grid_setup) {
  // decide whether to load
  // always load if top level
  bool load_successful=false;
  if (this->_check_bounds(i, j)) {
    std::vector<INT_T> zoom_load_list;
    // build up zoom
    for (INT_T k=this->_zoom_step_number-1; k >= 0l; k--) {
      // don't load anything if nothing loaded above lower limit
      if (k < zoom_index_lower_limit && zoom_load_list.size() == 0) {
        continue;
      }
      auto current_load_zoom=this->squares[i][j]->image_array[k]->zoom_index();
      if (this->_check_load(k, i, j, current_load_zoom, current_grid_x, current_grid_y, load_all)) {
        if (!this->squares[i][j]->image_array[k]->is_loaded) {
          zoom_load_list.push_back(k);
        }
      }
    }
    if (zoom_load_list.size() > 0) {
      auto dest_squares=std::vector<ImageGridSquareZoomLevel*>{};
      for (auto & zoom_load : zoom_load_list) {
        dest_squares.push_back(this->squares[i][j]->image_array[zoom_load]);
      }
      auto ij=j*this->_grid_image_size.wimage()+i;
      load_successful=ImageGridSquareZoomLevel::load_file(grid_setup->filenames()[ij],dest_squares);
    }
  }
  return load_successful;
}

void ImageGrid::load_grid(const GridSetup* const grid_setup, std::atomic<bool> &keep_running) {
  auto keep_trying=true;
  // get location of viewport
  FLOAT_T zoom;
  this->_viewport_current_state_imagegrid_update->GetGridValues(zoom,this->_viewport_grid);
  // find the grid extents and choose things that should be loaded/unloaded
  // different things for what should be loaded/unloaded
  // load in reverse order of size
  auto current_grid_x=(INT_T)floor(this->_viewport_grid.xgrid());
  auto current_grid_y=(INT_T)floor(this->_viewport_grid.ygrid());
  auto grid_w=this->_grid_image_size.wimage();
  auto grid_h=this->_grid_image_size.himage();
  auto load_all=false;
  // unload first
  for (auto k=this->_zoom_step_number-1; k >= 0l; k--) {
    for (INT_T i=0l; i < grid_w; i++) {
      for (INT_T j=0l; j < grid_h; j++) {
        if (!keep_running) {
          keep_trying=false;
        }
        auto current_load_zoom=this->squares[i][j]->image_array[k]->zoom_index();
        if (!this->_check_load(k, i, j, current_load_zoom, current_grid_x, current_grid_y, load_all)) {
          this->squares[i][j]->image_array[k]->unload_file();
        }
      }
    }
  }
  INT_T i,j;
  // files actually loaded
  INT_T load_count=0;
  auto current_zoom_index=ViewPortCurrentState::find_zoom_index(zoom);
  auto iterator_visible=ImageGridIteratorVisible(this->_grid_image_size.wimage(),this->_grid_image_size.himage(),
                                                 this->_image_max_size.wpixel(),this->_image_max_size.hpixel(),
                                                 current_grid_x,current_grid_y,zoom);

  // TODO need a good iterator class for this type of work load what
  // we are looking at if things are not loaded
  while (keep_trying) {
    keep_trying=iterator_visible.get_next(i,j);
    if (!keep_running) { keep_trying=false; break; }
    if (keep_trying) {
      auto load_successful=this->_load_file(i, j, current_grid_x, current_grid_y,
                                            current_zoom_index,
                                            load_all, grid_setup);
      if (load_successful) { load_count++; }
      if (load_count >= LOAD_FILES_BATCH) { keep_trying=false; }
    }
  }
  auto iterator_normal=ImageGridIteratorFull(this->_grid_image_size.wimage(),this->_grid_image_size.himage(),
                                         this->_image_max_size.wpixel(),this->_image_max_size.hpixel(),
                                         current_grid_x,current_grid_y,zoom);
  // TODO need a good iterator class for this type of work
  // load the one we are looking at
  if (load_count >= LOAD_FILES_BATCH) {
    keep_trying=false;
  } else {
    keep_trying=true;
  }
  while (keep_trying) {
    keep_trying=iterator_normal.get_next(i,j);
    if (!keep_running) { keep_trying=false; break; }
    if (keep_trying) {
      auto load_successful=this->_load_file(i, j, current_grid_x, current_grid_y,
                                            0l,
                                            load_all, grid_setup);
      if (load_successful) { load_count++; }
      if (load_count >= LOAD_FILES_BATCH) { keep_trying=false; }
    }
  }
}

GridPixelSize ImageGrid::get_image_max_pixel_size() {
  return this->_image_max_size;

}
