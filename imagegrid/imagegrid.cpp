/**
 * Implementation of the main classes representing the grid.  Includes
 * both loaded images and (zoomed) textures.
 */
// local headers
#include "../common.hpp"
#include "../utility.hpp"
#include "gridsetup.hpp"
#include "imagegrid.hpp"
#include "../iterators.hpp"
// C compatible headers
#include "../c_io_net/fileload.hpp"
// C++ headers
#include <array>
#include <atomic>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>
#include <utility>
#include <vector>
// C headers
#include <cmath>

ImageGridSquareZoomLevel::ImageGridSquareZoomLevel(INT_T zoom_out_value,
                                                   INT_T max_subgrid_wpixel,
                                                   INT_T max_subgrid_hpixel,
                                                   INT_T subgrid_width,
                                                   INT_T subgrid_height) {
  this->_zoom_out_value=zoom_out_value;
  this->_max_subgrid_wpixel=max_subgrid_wpixel/zoom_out_value;
  this->_max_subgrid_hpixel=max_subgrid_hpixel/zoom_out_value;
  this->_w_subgrid=subgrid_width;
  this->_h_subgrid=subgrid_height;
  // the size of the subgrid
  this->_rgb_wpixel=std::make_unique<std::unique_ptr<size_t[]>[]>(subgrid_width);
  for (INT_T i=0L; i < subgrid_width; i++) {
    this->_rgb_wpixel[i]=std::make_unique<size_t[]>(subgrid_height);
  }
  this->_rgb_hpixel=std::make_unique<std::unique_ptr<size_t[]>[]>(subgrid_width);
  for (INT_T i=0L; i < subgrid_width; i++) {
    this->_rgb_hpixel[i]=std::make_unique<size_t[]>(subgrid_height);
  }
  // the origin of each subgrid image
  this->_rgb_xpixel_origin=std::make_unique<std::unique_ptr<INT_T[]>[]>(subgrid_width);
  for (INT_T i=0L; i < subgrid_width; i++) {
    this->_rgb_xpixel_origin[i]=std::make_unique<INT_T[]>(subgrid_height);
  }
  this->_rgb_ypixel_origin=std::make_unique<std::unique_ptr<INT_T[]>[]>(subgrid_width);
  for (INT_T i=0L; i < subgrid_width; i++) {
    this->_rgb_ypixel_origin[i]=std::make_unique<INT_T[]>(subgrid_height);
  }
  // the rgb data itself
  this->rgb_data=std::make_unique<std::unique_ptr<unsigned char*[]>[]>(subgrid_width);
  for (INT_T i=0L; i < subgrid_width; i++) {
    this->rgb_data[i]=std::make_unique<unsigned char*[]>(subgrid_height);
  }
}

ImageGridSquareZoomLevel::~ImageGridSquareZoomLevel() {
  this->unload_square();
}

bool ImageGridSquareZoomLevel::load_square(SQUARE_DATA_T square_data,
                                           bool use_cache,
                                           std::vector<ImageGridSquareZoomLevel*> dest_squares) {
  auto load_successful=true;
  // iterate over square data
  std::vector<std::pair<ImageGridSquareZoomLevel* const,std::shared_ptr<LoadSquareData>>> data_pairs;
  std::vector<std::shared_ptr<LoadSquareData>> data_read;
  for (auto& dest_square : dest_squares) {
    data_pairs.emplace_back(std::pair<ImageGridSquareZoomLevel* const,
                            std::shared_ptr<LoadSquareData>>(dest_square,std::make_shared<LoadSquareData>()));
    data_pairs.back().second->zoom_out_value=dest_square->zoom_out_value();
    data_read.emplace_back(data_pairs.back().second);
  }
  // block until things load
  // only block things actually being loaded

  // need a more automatic solution to initialization like this
  for (auto& data_read_temp : data_read) {
    for (auto& subgrid_square_kv : square_data) {
      auto sub_i=subgrid_square_kv.first.first;
      auto sub_j=subgrid_square_kv.first.second;
      CURRENT_SUBGRID_T current_subgrid(sub_i,sub_j);
      data_read_temp->rgb_wpixel[current_subgrid]=INT_MIN;
      data_read_temp->rgb_hpixel[current_subgrid]=INT_MIN;
      data_read_temp->rgb_data[current_subgrid]=nullptr;
    }
  }
  for (auto& subgrid_square_kv : square_data) {
    auto filename=subgrid_square_kv.second;
    std::string cached_filename;
    if (use_cache) {
      cached_filename=ImageGridSquareZoomLevel::create_cache_filename(filename);
    } else {
      cached_filename="";
    }
    auto sub_i=subgrid_square_kv.first.first;
    auto sub_j=subgrid_square_kv.first.second;
    CURRENT_SUBGRID_T current_subgrid(sub_i,sub_j);
    auto load_successful_temp=load_data_as_rgb(filename,
                                               cached_filename,
                                               current_subgrid,
                                               data_read);
    // TODO: what happens if one part loads succesfully and another does not?
    if (!load_successful_temp) {
      load_successful=false;
    }
  }
  // TODO: change this once error handling is better
  if (load_successful) {
    for (auto& data_pair : data_pairs) {
      std::lock_guard<std::mutex> guard(data_pair.first->load_mutex);
      for (auto& subgrid_square_kv : square_data) {
        auto sub_i=subgrid_square_kv.first.first;
        auto sub_j=subgrid_square_kv.first.second;
        const INT_T origin_x=sub_i*data_pair.first->_max_subgrid_wpixel;
        const INT_T origin_y=sub_j*data_pair.first->_max_subgrid_hpixel;
        CURRENT_SUBGRID_T current_subgrid(sub_i,sub_j);
        data_pair.first->_rgb_wpixel[sub_i][sub_j]=data_pair.second->rgb_wpixel[current_subgrid];
        data_pair.first->_rgb_hpixel[sub_i][sub_j]=data_pair.second->rgb_hpixel[current_subgrid];
        data_pair.first->_rgb_xpixel_origin[sub_i][sub_j]=origin_x;
        data_pair.first->_rgb_ypixel_origin[sub_i][sub_j]=origin_y;
        data_pair.first->rgb_data[sub_i][sub_j]=data_pair.second->rgb_data[current_subgrid];
      }
      data_pair.first->is_loaded=true;
    }
  }
  return load_successful;
}

std::string ImageGridSquareZoomLevel::create_cache_filename(std::string filename) {
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

void ImageGridSquareZoomLevel::unload_square() {
  if (this->is_loaded) {
    std::lock_guard<std::mutex> guard(this->load_mutex);
    for (int i=0; i < this->_w_subgrid; i++) {
      for (int j=0; j < this->_h_subgrid; j++) {
        delete[] this->rgb_data[i][j];
        this->rgb_data[i][j]=nullptr;

      }
    }
    this->is_loaded=false;
  }
}

INT_T ImageGridSquareZoomLevel::zoom_out_value() const {
  return this->_zoom_out_value;
}

size_t ImageGridSquareZoomLevel::rgb_wpixel(INT_T w_index, INT_T h_index) const {
  return this->_rgb_wpixel[w_index][h_index];
}

size_t ImageGridSquareZoomLevel::rgb_hpixel(INT_T w_index, INT_T h_index) const {
  return this->_rgb_hpixel[w_index][h_index];
}

size_t ImageGridSquareZoomLevel::w_subgrid() const {
  return this->_w_subgrid;
}

size_t ImageGridSquareZoomLevel::h_subgrid() const {
  return this->_h_subgrid;
}

INT_T ImageGridSquareZoomLevel::rgb_xpixel_origin(INT_T w_index, INT_T h_index) const {
  return this->_rgb_xpixel_origin[w_index][h_index];
}

INT_T ImageGridSquareZoomLevel::rgb_ypixel_origin(INT_T w_index, INT_T h_index) const {
  return this->_rgb_ypixel_origin[w_index][h_index];
}

unsigned char* ImageGridSquareZoomLevel::get_rgb_data(INT_T w_index, INT_T h_index) const {
  return this->rgb_data[w_index][h_index];
}

ImageGridSquare::ImageGridSquare(SQUARE_DATA_T square_data) {
  this->_read_data(square_data);
}

void ImageGridSquare::_read_data(SQUARE_DATA_T square_data) {
  // I don't mind using vectors here because this is only called
  // during a constructor
  std::vector<INT_T> column_max;
  std::vector<INT_T> row_max;

  // TODO: right now the code works best for sparse/empty if these default to one
  INT_T max_subgrid_width=1;
  INT_T max_subgrid_height=1;

  INT_T subgrid_width=1;
  INT_T subgrid_height=1;

  for (auto& subgrid_kv_pair : square_data) {
    auto i=subgrid_kv_pair.first.first;
    auto j=subgrid_kv_pair.first.second;
    INT_T image_wpixel;
    INT_T image_hpixel;
    auto filename=square_data[subgrid_kv_pair.first];
    read_data(filename,
              image_wpixel,
              image_hpixel);
    if (image_wpixel > max_subgrid_width) {
      max_subgrid_width=image_wpixel;
    }
    if (image_hpixel > max_subgrid_height) {
      max_subgrid_height=image_hpixel;
    }
    if ((i+1) > subgrid_width) {
      subgrid_width=(i+1);
    }
    if ((j+1) > subgrid_height) {
      subgrid_height=(j+1);
    }
  }
  this->_image_wpixel=max_subgrid_width*subgrid_width;
  this->_image_hpixel=max_subgrid_height*subgrid_height;
  this->_subgrid_width=subgrid_width;
  this->_subgrid_height=subgrid_height;
  this->_max_subgrid_wpixel=max_subgrid_width;
  this->_max_subgrid_hpixel=max_subgrid_height;
}

void ImageGrid::_read_grid_info_setup_squares(const GridSetup* const grid_setup) {
  // delayed allocation for the squares
  auto grid_wimage=grid_setup->grid_image_size().wimage();
  auto grid_himage=grid_setup->grid_image_size().himage();
  this->squares=std::make_unique<std::unique_ptr<std::unique_ptr<ImageGridSquare>[]>[]>(grid_wimage);
  for (INT_T i=0L; i < grid_setup->grid_image_size().wimage(); i++) {
    this->squares[i]=std::make_unique<std::unique_ptr<ImageGridSquare>[]>(grid_himage);
  }
  this->_image_max_size=GridPixelSize(0,0);
  auto file_data=grid_setup->file_data();
  // not ready for an iterator until full sparsity is implemented and tested
  for (INT_T i=0; i<grid_wimage;i++) {
    for (INT_T j=0; j<grid_himage;j++) {
      std::pair<INT_T,INT_T> grid_location(i,j);
      this->squares[i][j]=std::make_unique<ImageGridSquare>(file_data[grid_location]);
      // set the RGB of the surface
      auto rgb_wpixel=this->squares[i][j]->_image_wpixel;
      auto rgb_hpixel=this->squares[i][j]->_image_hpixel;
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
  // }
  auto image_max_size_wpixel=this->_image_max_size.wpixel();
  auto image_max_size_hpixel=this->_image_max_size.hpixel();

  // find how many zoom_step to get whole image grid as a 3x3 grid of original size
  // always have a top level that fits in 32x32
  auto max_scale=(INT_T)ceil((FLOAT_T)(fmax(image_max_size_wpixel,image_max_size_hpixel))/(FLOAT_T)MAX_MIN_SCALED_IMAGE_SIZE);
  this->_zoom_index_length=(INT_T)ceil(log2(max_scale));
  if (this->_zoom_index_length < 1) {
    this->_zoom_index_length=1;
  }
  // diagnostic information
  MSG("max_scale: " << max_scale);
  MSG("max_zoom_index: " << this->_zoom_index_length);
  auto zoom_step=ZOOM_STEP;
  MSG("zoom_step: " << zoom_step);
  MSG("max_wpixel: " << this->_image_max_size.wpixel());
  MSG("max_hpixel: " << this->_image_max_size.hpixel());
  // add this info to the various data structure
  for (INT_T i=0L; i < this->_grid_image_size.wimage(); i++) {
    for (INT_T j=0L; j < this->_grid_image_size.himage(); j++) {
      this->squares[i][j]->image_array=std::make_unique<std::unique_ptr<ImageGridSquareZoomLevel>[]>(this->_zoom_index_length);
      INT_T zoom_out_value=1;
      this->squares[i][j]->_zoom_step_number=this->_zoom_index_length;
      for (auto k=0L; k < this->_zoom_index_length; k++) {
        this->squares[i][j]->image_array[k]=std::make_unique<ImageGridSquareZoomLevel>(zoom_out_value,
                                                                                       this->squares[i][j]->_max_subgrid_wpixel,
                                                                                       this->squares[i][j]->_max_subgrid_hpixel,
                                                                                       this->squares[i][j]->_subgrid_width,
                                                                                       this->squares[i][j]->_subgrid_height);
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

bool ImageGrid::_load_square(const ViewPortCurrentState& viewport_current_state,
                             INT_T i, INT_T j,
                             INT_T zoom_index_lower_limit,
                             INT_T load_all, const GridSetup* const grid_setup) {
  // decide whether to load
  // always load if top level
  bool tried_load=false;
  bool never_false=true;
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
        dest_squares.push_back(this->squares[i][j]->image_array[zoom_index].get());
      }
      const std::pair<INT_T,INT_T> grid_pair(i,j);
      auto file_data=grid_setup->file_data();
      std::string cached_file;
      tried_load=true;
      // TODO: this might go funny for half-empty squares
      auto load_successful_temp=ImageGridSquareZoomLevel::load_square(file_data[grid_pair],
                                                                      grid_setup->use_cache(),
                                                                      dest_squares);
      if (!load_successful_temp) {
        never_false=false;
      }

    }
  }
  return (tried_load && never_false);
}

// TODO: This probably won't work for certain squres, need to take into account zoom for whole thing
void ImageGrid::_write_cache(INT_T i, INT_T j, SQUARE_DATA_T square_data) {
  // TODO: this kind of individually probably won't work longterm
  for (auto& subgrid_square_kv : square_data) {
    bool loaded_512=false;
    auto i_sub=subgrid_square_kv.first.first;
    auto j_sub=subgrid_square_kv.first.second;
    auto filename=subgrid_square_kv.second;
    for (INT_T k=0L; k<this->_zoom_index_length; k++) {
      if (loaded_512) {
        break;
      }
      auto dest_square=this->squares[i][j]->image_array[k].get();
      // find current size
      auto wpixel=dest_square->rgb_wpixel(i_sub,j_sub);
      auto hpixel=dest_square->rgb_hpixel(i_sub,j_sub);
      // TODO: check size of filename_new here
      auto filename_new=ImageGridSquareZoomLevel::create_cache_filename(filename);
      MSG("Trying to writing cache filename: " << filename_new << " for " << filename << " at zoom index " << k << " i: " << i << " j: " << j << "i_sub: " << i_sub << " j_sub: " << j_sub);
      if (wpixel < CACHE_MAX_PIXEL_SIZE && hpixel < CACHE_MAX_PIXEL_SIZE) {
        loaded_512=write_png(filename_new, wpixel, hpixel, dest_square->rgb_data[i_sub][j_sub]);
        MSG("Cache tried with return: " << loaded_512);
       }
    }
  }
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
          this->squares[i][j]->image_array[zoom_index]->unload_square();
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
      auto load_successful=this->_load_square(viewport_current_state,
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
  keep_trying=(!(load_count >= LOAD_FILES_BATCH || !keep_running));
  while (keep_trying) {
    keep_trying=iterator_normal.get_next(i,j);
    if (!keep_running) { break; }
    if (keep_trying) {
      auto load_successful=this->_load_square(viewport_current_state,
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
      this->_load_square(ViewPortCurrentState(GridCoordinate(0.0,0.0),
                                              GridPixelSize(0,0),
                                              0.0,
                                              ViewportPixelSize(0,0),
                                              true),
                         i,j,
                         0L,true,grid_setup);
      // TODO: eventually cache this out as tiles that fit in 128x128 and 512x512
      const std::pair<INT_T,INT_T> grid_pair(i,j);
      // const CURRENT_SUBGRID_T subgrid_pair(0,0);
      // auto file_data=grid_setup->file_data();
      // auto filename=file_data[grid_pair][subgrid_pair];
      auto file_data=grid_setup->file_data();
      auto square_data=file_data[grid_pair];
      this->_write_cache(i,j,square_data);
      // unload
      for (auto k=this->_zoom_index_length-1; k >= 0L; k--) {
        this->squares[i][j]->image_array[k]->unload_square();
      }
    }
  }

}
