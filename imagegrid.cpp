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
#include "viewport.hpp"
#include "viewport_current_state.hpp"
// C compatible headers
#include "c_compatible/fileload.hpp"
#include <png.h>
// C++ headers
#include <array>
#include <atomic>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

ImageGridSquareZoomLevel::ImageGridSquareZoomLevel(INT_T zoom_out_value) {
  this->_zoom_out_value=zoom_out_value;
}

ImageGridSquareZoomLevel::~ImageGridSquareZoomLevel() {
  this->unload_file();
}

bool ImageGridSquareZoomLevel::load_file(std::string filename,
                                         std::string cached_filename,
                                         std::vector<ImageGridSquareZoomLevel*> dest_squares) {
  // block until things load
  // only block things actually being loaded
  auto load_successful=false;
  std::vector<std::pair<ImageGridSquareZoomLevel* const,std::shared_ptr<LoadFileData>>> data_pairs;
  std::vector<std::shared_ptr<LoadFileData>> data_read;
  for (auto & dest_square : dest_squares) {
    data_pairs.emplace_back(std::pair<ImageGridSquareZoomLevel* const,
                            std::shared_ptr<LoadFileData>>(dest_square,std::make_shared<LoadFileData>()));
    data_pairs.back().second->zoom_out_value=dest_square->zoom_out_value();
    data_read.emplace_back(data_pairs.back().second);
  }
  if (check_tiff(filename)) {
    MSG("Loading TIFF: " << filename);
    // TODO: check success
    load_tiff_as_rgb(filename,
                     cached_filename,
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
  } else if (check_empty(filename)) {
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

INT_T ImageGridSquareZoomLevel::zoom_out_value() const {
  return this->_zoom_out_value;
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
  for (auto i=0L; i < this->_zoom_step_number; i++) {
    DELETE_IF_NOT_NULLPTR(this->image_array[i]);
  }
}

void ImageGridSquare::_read_file(std::string filename) {
  if (check_tiff(filename)) {
    // TODO: check success
    read_tiff_data(filename,
                   this->_image_wpixel,
                   this->_image_hpixel);
  } else if (check_png(filename)) {
    // TODO: check success
    read_png_data(filename,
                  this->_image_wpixel,
                  this->_image_hpixel);
  } else if (check_empty(filename)) {
    // think that 0 is a reasonable non-image for these values
    // TODO: other values may be good
    this->_image_wpixel=0;
    this->_image_hpixel=0;
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
  for (INT_T i=0L; i < this->_grid_image_size.wimage(); i++) {
    for (INT_T j=0L; j < this->_grid_image_size.himage(); j++) {
      DELETE_IF_NOT_NULLPTR(this->squares[i][j]);
    }
    DELETE_ARRAY_IF_NOT_NULLPTR(this->squares[i]);
  }
}

void ImageGrid::_read_grid_info_setup_squares(const GridSetup* const grid_setup) {
  this->squares=std::make_unique<ImageGridSquare**[]>(grid_setup->grid_image_size().wimage());
  for (INT_T i=0L; i < grid_setup->grid_image_size().wimage(); i++) {
    this->squares[i]=new ImageGridSquare*[grid_setup->grid_image_size().himage()];
  }
  this->_image_max_size=GridPixelSize(0,0);
  // now actually intialize squares
  for (INT_T i=0L; i < this->_grid_image_size.wimage(); i++) {
    for (INT_T j=0L; j < this->_grid_image_size.himage(); j++) {
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
    }
  }
  // find max zoom for each level
  auto image_max_size_wpixel=this->_image_max_size.wpixel();
  auto image_max_size_hpixel=this->_image_max_size.hpixel();

  // find how many zoom_step to get whole image grid as a 3x3 grid of original size
  // always have a top level that fits in 32x32
  auto max_scale=(INT_T)ceil((FLOAT_T)(fmax(image_max_size_wpixel,image_max_size_hpixel))/(FLOAT_T)MAX_MIN_SCALED_IMAGE_SIZE);
  this->_zoom_index_length=(INT_T)ceil(log2(max_scale));
  if (this->_zoom_index_length < 1) {
    this->_zoom_index_length=1;
  }
  MSG("max_scale: " << max_scale);
  MSG("max_zoom_index: " << this->_zoom_index_length);
  auto zoom_step=ZOOM_STEP;
  MSG("zoom_step: " << zoom_step);
  // add this info to the various data structure
  for (INT_T i=0L; i < this->_grid_image_size.wimage(); i++) {
    for (INT_T j=0L; j < this->_grid_image_size.himage(); j++) {
      this->squares[i][j]->image_array=std::make_unique<ImageGridSquareZoomLevel*[]>(this->_zoom_index_length);
      INT_T zoom_out_value=1;
      this->squares[i][j]->_zoom_step_number=this->_zoom_index_length;
      for (auto k=0L; k < this->_zoom_index_length; k++) {
        this->squares[i][j]->image_array[k]=new ImageGridSquareZoomLevel(zoom_out_value);
        zoom_out_value*=zoom_step;
      }
    }
  }
}

void ImageGrid::read_grid_info(const GridSetup* const grid_setup, std::shared_ptr<ViewPortTransferState> viewport_current_state_imagegrid_update) {
  this->_grid_image_size=GridImageSize(grid_setup->grid_image_size());
  this->_viewport_current_state_imagegrid_update=viewport_current_state_imagegrid_update;
  this->_read_grid_info_setup_squares(grid_setup);
  // actually use this
  this->_read_grid_info_successful=true;
}

bool ImageGrid::read_grid_info_successful() const {
  return this->_read_grid_info_successful;
}

INT_T ImageGrid::zoom_index_length() const {
  return this->_zoom_index_length;
}

bool ImageGrid::_check_bounds(INT_T i, INT_T j) {
  auto return_value=(i >= 0 && i < this->_grid_image_size.wimage() && j >= 0 && j < this->_grid_image_size.himage());
  return return_value;
}

bool ImageGrid::_check_load(const ViewPortCurrentState& viewport_current_state,
                            INT_T zoom_index, INT_T i, INT_T j, INT_T zoom_index_lower_limit,
                            INT_T load_all) {
  auto current_grid_x=viewport_current_state.current_grid_coordinate().xgrid();
  auto current_grid_y=viewport_current_state.current_grid_coordinate().ygrid();
  auto screen_width=viewport_current_state.screen_size().hpixel();
  auto screen_height=viewport_current_state.screen_size().wpixel();
  // TODO: don't like this so need to keep working on transferring calculations
  auto viewport_current_state_new=ViewPortCurrentState(GridCoordinate(current_grid_x,current_grid_y),
                                                       GridPixelSize(this->_image_max_size.wpixel(), this->_image_max_size.hpixel()),
                                                       ViewPortTransferState::find_zoom_upper(zoom_index),
                                                       ViewportPixelSize(screen_width,screen_height), true);
  auto return_value=((zoom_index == this->_zoom_index_length-1 ||
                      (zoom_index >= zoom_index_lower_limit &&
                       ViewPortTransferState::grid_index_visible(i,j,
                                                                 viewport_current_state_new)) ||
                      // only load adjacent grid below zoom limit
                      (i >= (floor(current_grid_x)-1) && i <= (floor(current_grid_x)+1) &&
                       j >= (floor(current_grid_y)-1) && j <= (floor(current_grid_y)+1)) ||
                      load_all));
  return return_value;
}

bool ImageGrid::_load_file(const ViewPortCurrentState& viewport_current_state,
                           INT_T i, INT_T j,
                           INT_T zoom_index_lower_limit,
                           INT_T load_all, const GridSetup* const grid_setup) {
  // decide whether to load
  // always load if top level
  bool load_successful=false;
  if (this->_check_bounds(i, j)) {
    std::vector<INT_T> zoom_index_list;
    for (INT_T zoom_index=this->_zoom_index_length-1; zoom_index >= 0L; zoom_index--) {
      if (this->_check_load(viewport_current_state,
                            zoom_index, i, j, zoom_index_lower_limit,
                            load_all)) {
        if (!this->squares[i][j]->image_array[zoom_index]->is_loaded) {
          zoom_index_list.push_back(zoom_index);
        }
      }
    }
    if (zoom_index_list.size() > 0) {
      auto dest_squares=std::vector<ImageGridSquareZoomLevel*>{};
      for (auto & zoom_index : zoom_index_list) {
        dest_squares.push_back(this->squares[i][j]->image_array[zoom_index]);
      }
      auto ij=j*this->_grid_image_size.wimage()+i;
      auto filename=grid_setup->filenames()[ij];
      std::string cached_file;
      if (grid_setup->use_cache()) {
        cached_file=this->_create_cache_filename(filename);
      } else {
        cached_file="";
      }
      load_successful=ImageGridSquareZoomLevel::load_file(filename,
                                                          cached_file,
                                                          dest_squares);
    }
  }
  return load_successful;
}

bool ImageGrid::_write_cache(INT_T i, INT_T j, std::string filename) {
  bool loaded_512=false;
  for (INT_T k=0L; k<this->_zoom_index_length; k++) {
    if (loaded_512) {
      break;
    }
    auto dest_square=this->squares[i][j]->image_array[k];
    // find current size
    auto wpixel=dest_square->rgb_wpixel();
    auto hpixel=dest_square->rgb_hpixel();
    if (wpixel < CACHE_MAX_PIXEL_SIZE && hpixel < CACHE_MAX_PIXEL_SIZE) {
      // TODO: check size of filename_new here
      auto filename_new=this->_create_cache_filename(filename);
      char new_filename[PATH_BUFFER_SIZE]="";
      strncpy(new_filename,filename_new.c_str(),PATH_BUFFER_SIZE);
      // write a PNG
      // TODO: put this elsewhere
      png_image image;
      memset(&image, 0, (sizeof image));
      image.version=PNG_IMAGE_VERSION;
      image.opaque=NULL;
      image.width=wpixel;
      image.height=hpixel;
      image.format=PNG_FORMAT_RGB;
      image.flags=0;
      image.colormap_entries=0;
      png_image_write_to_file(&image, new_filename, 0, (void*)dest_square->rgb_data, 0, 0);
      loaded_512=true;
    }
  }
  return 0;
}

std::string ImageGrid::_create_cache_filename(std::string filename) {
  std::filesystem::path filename_path{filename};
  auto filename_parent=filename_path.parent_path();
  auto filename_base=filename_path.filename();
  auto filename_stem=filename_base.stem();
  auto filename_new=filename_parent;
  filename_new/="__imagegrid__cache__";
  std::filesystem::create_directories(filename_new);
  filename_new/=filename_stem;
  filename_new+=".png";
  return filename_new.string();
}

void ImageGrid::load_grid(const GridSetup* const grid_setup, std::atomic<bool> &keep_running) {
  auto keep_trying=true;
  // get information on viewport
  auto viewport_current_state=this->_viewport_current_state_imagegrid_update->GetGridValues();
  // find the grid extents and choose things that should be loaded/unloaded
  // different things for what should be loaded/unloaded
  // load in reverse order of size
  auto current_zoom_index=ViewPortTransferState::find_zoom_index_bounded(viewport_current_state.zoom(),0.0,this->_zoom_index_length-1);
  auto grid_w=this->_grid_image_size.wimage();
  auto grid_h=this->_grid_image_size.himage();
  auto load_all=false;
  auto zoom_index_lower_limit=current_zoom_index-1;
  // unload first
  for (auto zoom_index=this->_zoom_index_length-1; zoom_index >= 0L; zoom_index--) {
    for (INT_T i=0L; i < grid_w; i++) {
      for (INT_T j=0L; j < grid_h; j++) {
        if (!keep_running) {
          keep_trying=false;
        }
        if (!this->_check_load(viewport_current_state,
                               zoom_index, i, j, zoom_index_lower_limit,
                               load_all)) {
          this->squares[i][j]->image_array[zoom_index]->unload_file();
          // always try and unload rest, except top level
        }
      }
    }
  }
  INT_T i,j;
  // files actually loaded
  INT_T load_count=0;
  auto iterator_visible=ImageGridIteratorVisible(this->_grid_image_size.wimage(),this->_grid_image_size.himage(),
                                                 viewport_current_state);
  // TODO need a good iterator class for this type of work load what
  // we are looking at if things are not loaded
  while (keep_trying) {
    keep_trying=iterator_visible.get_next(i,j);
    if (!keep_running) { break; }
    if (keep_trying) {
      auto load_successful=this->_load_file(viewport_current_state,
                                            i, j,
                                            zoom_index_lower_limit,
                                            load_all, grid_setup);
      if (load_successful) { load_count++; }
      if (load_count >= LOAD_FILES_BATCH) { keep_trying=false; }
    }
  }
  auto iterator_normal=ImageGridIteratorFull(this->_grid_image_size.wimage(),this->_grid_image_size.himage(),
                                             viewport_current_state);
  // TODO need a good iterator class for this type of work
  // load the one we are looking at
  if (load_count >= LOAD_FILES_BATCH || !keep_running) {
    keep_trying=false;
  } else {
    keep_trying=true;
  }
  while (keep_trying) {
    keep_trying=iterator_normal.get_next(i,j);
    if (!keep_running) { break; }
    if (keep_trying) {
      auto load_successful=this->_load_file(viewport_current_state,
                                            i, j,
                                            zoom_index_lower_limit,
                                            load_all, grid_setup);
      if (load_successful) { load_count++; }
      if (load_count >= LOAD_FILES_BATCH) { keep_trying=false; }
    }
  }
  // uncomment for something fairly useful for seeing how files load
  // MSG("================================================================================");
  // for (auto zoom_index=this->_zoom_index_length-1; zoom_index >= 0L; zoom_index--) {
  //   MSG("Zoom: " << zoom_index);
  //   for (INT_T j=0L; j < grid_h; j++) {
  //     for (INT_T i=0L; i < grid_w; i++) {
  //       std::cout << this->squares[i][j]->image_array[zoom_index]->is_loaded;
  //     }
  //     std::cout << " === ";
  //     for (INT_T i=0L; i < grid_w; i++) {
  //       std::cout << this->_check_load(viewport_current_state,
  //                                      zoom_index,i,j,zoom_index_lower_limit,load_all);
  //     }
  //     std::cout << "\n";
  //   }
  // }
}

GridPixelSize ImageGrid::get_image_max_pixel_size() const {
  return this->_image_max_size;

}

void ImageGrid::setup_grid_cache(const GridSetup* grid_setup) {
  // no read_grid_info(...) called for now
  this->_grid_image_size=GridImageSize(grid_setup->grid_image_size());
  auto grid_w=this->_grid_image_size.wimage();
  auto grid_h=this->_grid_image_size.himage();
  this->_read_grid_info_setup_squares(grid_setup);
  // loop over the whole grid
  for (INT_T i=0L; i < grid_w; i++) {
    for (INT_T j=0L; j < grid_h; j++) {
      // load the file into the data structure
      // current_grid_x and current_grid_y are dummy argument because of load_all being true
      // don't like this dummy stuff
      this->_load_file(ViewPortCurrentState(GridCoordinate(0.0,0.0),
                                            GridPixelSize(0,0),
                                            0.0,
                                            ViewportPixelSize(0,0),
                                            true),
                       i,j,
                       0L,true,grid_setup);
      // TODO: eventually cache this out as tiles that fit in 128x128 and 512x512
      auto ij=j*this->_grid_image_size.wimage()+i;
      auto filename=grid_setup->filenames()[ij];
      this->_write_cache(i,j,filename);
      // unload
      for (auto k=this->_zoom_index_length-1; k >= 0L; k--) {
        this->squares[i][j]->image_array[k]->unload_file();
      }
    }
  }
}
